#include "Generator.h"
#include "../Chessboard.h"

namespace Chess::MoveGen
{
	MoveGenerator::MoveGenerator( ) : 
		m_Moves( std::make_unique< std::array<Move, 218> >() ),
		m_PinRays( std::make_unique< std::unordered_map<int, Bitboard> >() ),
		RookMoves( CreateSlidingMoves( false ) ),
		BishopMoves( CreateSlidingMoves( true ) ),
		InBetweenLookup( CreateInBetweenLookup() )
	{	
		logger.debug( "Movegenerator created" );
	}

	void MoveGenerator::GenerateInfo()
	{
		ChessCoord King( FriendlyKingSq );

		Bitboard Ray;

		bool FriendlyAlongRay;
		bool Diagonal = false;

		for ( auto& dir : DirChanges )
		{
			Ray = 0;
			FriendlyAlongRay = false;

			for ( ChessCoord New = King + dir; New.IsValid(); New += dir )
			{
				ChessPiece piece = m_Board->m_Bitboards.GetPieceAtSquare( New.AsSquare() );

				if ( piece.color == FriendlyColor )
				{
					if ( FriendlyAlongRay )
						break;	// Two friendlies found so no pin
					FriendlyAlongRay = true;
				}
				else
				{
					if ( (Diagonal && (piece.type != PieceType::Queen || piece.type != PieceType::Bishop)) ||
						(!Diagonal && (piece.type != PieceType::Queen || piece.type != PieceType::Rook)) )
						break; // Enemy piece is not a sliding piece so early break

					/*if ( FriendlyAlongRay )
						PinRays |= Ray;
					else*/
						Checkmask |= Ray;

					break;
				}
			}
			Diagonal = !Diagonal;
		}
	}

	void MoveGenerator::AddMove( Move move )
	{
		m_Moves->at( MoveListTop++ ) = move;
	}

	void MoveGenerator::AddMovesFromBitboard( int Start, Bitboard bb )
	{
		while ( bb )
		{
			int sq = bb.BitscanForward();
			bb &= bb - 1;

			MoveFlag flag = MoveFlag::None;
			// TODO: Bæta við logic fyrir önnur flags
			if ( EnemyPieces.IsOccupied( sq ) )
				flag |= MoveFlag::Capture;
			
			Move move = Move( Start, sq, flag );
			m_Moves->at( MoveListTop++ ) = move;
		}
	}
	void MoveGenerator::ClearMoveList()
	{
		m_Moves->fill( Move() );
		MoveListTop = 0;
	}

	std::unique_ptr<std::array<std::unordered_map<int, Bitboard>, 64>> MoveGenerator::CreateSlidingMoves( bool Diagonal )
	{
		auto result = std::make_unique< std::array<std::unordered_map<int, Bitboard>, 64> >();

		std::unordered_map<int, Bitboard> Masks;

		const std::array<Bitboard, 64>& BlockerMasks = Diagonal ? BishopBlockerMasks : RookBlockerMasks;

		Bitboard BlockerMask, MoveMask;
		int MagicIdx;
		std::vector<Bitboard> BlockerPerms;

		if ( Diagonal )
		{
			// Average number of perms for bishop masks
			BlockerPerms.reserve( 8192 );
			Masks.reserve( 82 );
		}
		else
		{
			// Average number of perms for rook masks
			Masks.reserve( 1600 );
			BlockerPerms.reserve( 16384 );
		}

		for ( short Sq = 0; Sq < 64; Sq++ )
		{
			Masks.clear();
			BlockerMask = BlockerMasks[Sq];
			BlockerPerms = CreateBlockerPerms( BlockerMask );

			for ( auto& BlockMask : BlockerPerms )
			{
				MoveMask = MoveMaskFromBlocker( Sq, BlockMask, Diagonal );
				MagicIdx = Magic::GetMagicIdx( BlockMask, Sq, Diagonal );
				Masks[MagicIdx] = MoveMask;
			}
			result->at( Sq ) = Masks;
		}
		return result;
	}

	Bitboard MoveGenerator::InBetween( int sq1, int sq2 ) const
	{
		using U64 = std::uint64_t;
		
		// This function is taken verbatim from the Chessprogramming wiki
		// https://www.chessprogramming.org/Square_Attacked_By#Pure_Calculation
		
		const U64 m1 = U64( -1 );
		const U64 a2a7 = U64( 0x0001010101010100 );
		const U64 b2g7 = U64( 0x0040201008040200 );
		const U64 h1b7 = U64( 0x0002040810204080 ); /* Thanks Dustin, g2b7 did not work for c1-a3 */
		U64 btwn, line, rank, file;

		btwn = (m1 << sq1) ^ (m1 << sq2);
		file = (sq2 & 7) - (sq1 & 7);
		rank = ((sq2 | 7) - sq1) >> 3;
		line = ((file & 7) - 1) & a2a7; /* a2a7 if same file */
		line += 2 * (((rank & 7) - 1) >> 58); /* b1g1 if same rank */
		line += (((rank - file) & 15) - 1) & b2g7; /* b2g7 if same diagonal */
		line += (((rank + file) & 15) - 1) & h1b7; /* h1b7 if same antidiag */
		line *= btwn & -btwn; /* mul acts like shift by smaller square */
		return line & btwn;   /* return the bits on that line in-between */
	}

	std::unique_ptr < std::array<std::array<Bitboard, 64>, 64>> MoveGenerator::CreateInBetweenLookup()
	{
		auto result = std::make_unique< std::array<std::array<Bitboard, 64>, 64> >();

		for ( int sq1 = 0; sq1 < 64; sq1++ )
		{
			for ( int sq2 = 0; sq2 < 64; sq2++ )
			{
				(*result)[sq1][sq2] = InBetween( sq1, sq2 );
			}
		}
		return result;
	}

	Bitboard MoveGenerator::GetRookMoveMask( short Sq, Bitboard Occupied ) const
	{
		Bitboard Blockers = RookBlockerMasks[Sq] & Occupied;
		int MagicIdx = Magic::GetMagicIdx( Blockers, Sq, false );
		return RookMoves->at( Sq )[MagicIdx];
	}

	Bitboard MoveGenerator::GetBishopMoveMask( short Sq, Bitboard Occupied ) const
	{
		Bitboard Blockers = BishopBlockerMasks[Sq] & Occupied;
		int MagicIdx = Magic::GetMagicIdx( Blockers, Sq, true );
		return BishopMoves->at( Sq )[MagicIdx];
	}

	Bitboard MoveGenerator::XRayRookAttacks( Bitboard Occupied, Bitboard Blockers, short Sq )
	{
		Bitboard Attacks = GetRookMoveMask( Occupied, Sq );
		Blockers &= Attacks;
		return Attacks ^ GetRookMoveMask( Occupied ^ Blockers, Sq );
	}
	Bitboard MoveGenerator::XRayBishopAttacks( Bitboard Occupied, Bitboard Blockers, short Sq )
	{
		Bitboard Attacks = GetBishopMoveMask( Occupied, Sq );
		Blockers &= Attacks;
		return Attacks ^ GetBishopMoveMask( Occupied ^ Blockers, Sq );
	}

	Bitboard MoveGenerator::GetAlignMask( int Start, int Target ) const
	{
		return InBetweenLookup->at( Start )[Target];
	}

	void MoveGenerator::GenerateAttackmask()
	{
		Attackmask = 0;
		Bitboard pieceAttacks;
		Bitboard Enemies = EnemyPieces;
		while ( Enemies )
		{
			int sq = Enemies.BitscanForward();
			Enemies &= Enemies - 1;

			ChessPiece piece = m_Board->m_Bitboards.GetPieceAtSquare( sq );
			switch ( piece.type )
			{
			case Chess::PieceType::King:
				pieceAttacks = KingMoves[sq];
				break;
			case Chess::PieceType::Pawn:
				pieceAttacks = WhiteToPlay ? BlackPawnAttacks[sq] : WhitePawnAttacks[sq];
				break;
			case Chess::PieceType::Knight:
				pieceAttacks = KnightMoves[sq];
				break;
			case Chess::PieceType::Bishop:
				pieceAttacks = GetBishopMoveMask( sq, AllPieces );
				break;
			case Chess::PieceType::Rook:
				pieceAttacks = GetRookMoveMask( sq, AllPieces );
				break;
			case Chess::PieceType::Queen:
				pieceAttacks = GetRookMoveMask( sq, AllPieces ) | GetBishopMoveMask( sq, AllPieces );
				break;
			case Chess::PieceType::None:
			default:
				break;
			}
			if ( pieceAttacks.IsOccupied( FriendlyKingSq ) )
			{
				InDoubleCheck = InCheck;
				InCheck = true;

				Checkmask |= Bit << sq;
				if ( piece.IsSlidingPiece() )
					Checkmask |= GetAlignMask( FriendlyKingSq, sq );

			}
			Attackmask |= pieceAttacks;
		}
	}

	void MoveGenerator::FindPinned()
	{
		m_PinRays->clear();

		Bitboard EnemyRooks = EnemyRQ;
		while ( EnemyRooks )
		{
			int sq = EnemyRooks.BitscanForward();
			EnemyRooks &= EnemyRooks - 1;

			Bitboard Attacks = GetRookMoveMask( sq, AllPieces );
			if ( Attacks.IsOccupied( FriendlyKingSq ) )
			{
				InCheck = true;
			}

			Bitboard Blockers = FriendlyPieces;
			Blockers &= Attacks;
			Attacks = GetRookMoveMask( sq, AllPieces ^ Blockers );
			if ( Attacks.IsOccupied( FriendlyKingSq ) )
			{
				Bitboard AlignMask = GetAlignMask( sq, FriendlyKingSq );
				int Pinned = (AlignMask & Blockers).BitscanForward();
				m_PinRays->at( Pinned ) = (AlignMask | Bit << FriendlyKingSq | Bit << sq);
			}
		}

		Bitboard EnemyBishops = EnemyBQ;
		while ( EnemyBishops )
		{
			int sq = EnemyBishops.BitscanForward();
			EnemyBishops &= EnemyBishops - 1;

			Bitboard Attacks = GetBishopMoveMask( sq, AllPieces );
			if ( Attacks.IsOccupied( FriendlyKingSq ) )
			{
				InCheck = true;
				Bitboard Ray = GetAlignMask( sq, FriendlyKingSq );
				Checkmask |= (Ray | Bit << sq);
			}

			Bitboard Blockers = FriendlyPieces;
			Blockers &= Attacks;
			Attacks = GetBishopMoveMask( sq, AllPieces ^ Blockers );
			if ( Attacks.IsOccupied( FriendlyKingSq ) )
			{
				Bitboard AlignMask = GetAlignMask( sq, FriendlyKingSq );
				int Pinned = (AlignMask & Blockers).BitscanForward();
				m_PinRays->at( Pinned ) = (AlignMask | Bit << FriendlyKingSq | Bit << sq);
			}
		}
	}

	void MoveGenerator::GetKingMoves( short Sq )
	{
		Bitboard MoveMask = KingMoves[Sq];
		MoveMask &= ~(FriendlyPieces | Attackmask);

		AddMovesFromBitboard( Sq, MoveMask );

		// Castling
		if ( !InCheck && Rights != CastlingRights::None )
		{
			bool IsWhite = FriendlyColor == Color::White;
			int CastleTarget;

			Bitboard KingSideBlock = 0b01100000;
			Bitboard KingSideCheck = 0b01100000;
			Bitboard QueenSideBlock = 0b00001110;
			Bitboard QueenSideCheck = 0b00001100;

			Bitboard BlockerMask;
			Bitboard CheckMask;

			if ( (Rights & CastlingRights::Kingside) == CastlingRights::Kingside )
			{
				if ( !IsWhite )
				{
					KingSideBlock <<= 56;
					KingSideCheck <<= 56;
				}

				BlockerMask = KingSideBlock;
				CheckMask = KingSideCheck;

				if ( (BlockerMask & ~(FriendlyPieces | EnemyPieces)) == KingSideBlock && (CheckMask & ~(Attackmask)) == KingSideCheck )
				{
					CastleTarget = IsWhite ? 6 : 62;
					AddMove( Move( Sq, CastleTarget, MoveFlag::CastleKing ) );
				}
			}

			if ( (Rights & CastlingRights::QueenSide) == CastlingRights::QueenSide )
			{
				if ( !IsWhite )
				{
					QueenSideBlock <<= 56;
					QueenSideCheck <<= 56;
				}

				BlockerMask = QueenSideBlock;
				CheckMask = QueenSideCheck;

				if ( (BlockerMask & ~(FriendlyPieces | EnemyPieces)) == QueenSideBlock && (CheckMask & ~(Attackmask)) == QueenSideCheck )
				{
					CastleTarget = IsWhite ? 2 : 58;
					AddMove( Move( Sq, CastleTarget, MoveFlag::CastleQueen ) );
				}
			}
		}
	}

	void MoveGenerator::GetKnightMoves( short Sq )
	{
		Bitboard MoveMask = KnightMoves[Sq];
		MoveMask &= ~(FriendlyPieces);

		if ( InCheck )
			MoveMask &= Checkmask;

		if ( m_PinRays->contains( Sq ) )
		{
			Bitboard AlignMask = m_PinRays->at( Sq );
			MoveMask &= AlignMask;
		}

		AddMovesFromBitboard( Sq, MoveMask );
	}

	void MoveGenerator::GetPawnMoves( short Sq )
	{
		Bitboard MoveMask = m_Board->m_WhiteToPlay ? WhitePawnMoves[Sq] : BlackPawnMoves[Sq];

		bool double_push = false;
		int DoublePushRank = m_Board->m_WhiteToPlay ? 3 : 4;

		if ( InCheck )
			MoveMask &= Checkmask;

		while ( MoveMask )
		{
			int target = MoveMask.BitscanForward();
			MoveMask &= MoveMask - 1;

			if ( AllPieces.IsOccupied( target ) )
				break;

			double_push = (target >> 3) == DoublePushRank;

			MoveFlag flag = double_push ? MoveFlag::DoublePawnMove : MoveFlag::None;
			AddMove( Move( Sq, target, flag ) );
		}

		Bitboard AttackMask = m_Board->m_WhiteToPlay ? WhitePawnAttacks[Sq] : BlackPawnAttacks[Sq];
		
		int EPSquare = m_Board->m_EnPassantSquare;

		while ( AttackMask )
		{
			int target = AttackMask.BitscanForward();
			AttackMask &= AttackMask - 1;

			if ( target == EPSquare && CanEnPassant(Sq, target, EPSquare + (WhiteToPlay ? -8 : 8) ) )
			{
				AddMove( Move( Sq, target, MoveFlag::EnPassant ) );
			}
			else if ( EnemyPieces.IsOccupied( target ) )
			{
				AddMove( Move( Sq, target, MoveFlag::Capture ) );
			}
		}
	}
	
	void MoveGenerator::GetBishopMoves( short Sq )
	{
		Bitboard Occupied = AllPieces & BishopBlockerMasks[Sq];
		Bitboard MoveMask = GetBishopMoveMask( Sq, Occupied );
		MoveMask &= ~(FriendlyPieces);

		if ( InCheck )
			MoveMask &= Checkmask;

		if ( m_PinRays->contains( Sq ) )
		{
			Bitboard AlignMask = m_PinRays->at( Sq );
			Occupied &= AlignMask;
		}

		AddMovesFromBitboard( Sq, MoveMask );
	}

	void MoveGenerator::GetRookMoves( short Sq )
	{
		Bitboard Occupied = AllPieces & RookBlockerMasks[Sq];
		Bitboard MoveMask = GetRookMoveMask( Sq, Occupied );
		MoveMask &= ~(FriendlyPieces);

		if ( InCheck )
			MoveMask &= Checkmask;

		if ( m_PinRays->contains( Sq ) )
		{
			Bitboard AlignMask = m_PinRays->at( Sq );
			MoveMask &= AlignMask;
		}

		AddMovesFromBitboard( Sq, MoveMask );
	}

	void MoveGenerator::GetQueenMoves( short Sq )
	{
		Bitboard Occupied = AllPieces & BishopBlockerMasks[Sq];
		Bitboard MoveMask = GetBishopMoveMask( Sq, Occupied );

		Occupied = AllPieces & RookBlockerMasks[Sq];
		MoveMask |= GetRookMoveMask( Sq, Occupied );

		MoveMask &= ~(FriendlyPieces);

		if ( InCheck )
			MoveMask &= Checkmask;

		if ( m_PinRays->contains( Sq ) )
		{
			Bitboard AlignMask = m_PinRays->at( Sq );
			MoveMask &= AlignMask;
		}

		AddMovesFromBitboard( Sq, MoveMask );
	}

	bool MoveGenerator::CanEnPassant( short startSquare, short targetSquare, short epCaptureSquare ) const
	{
		Bitboard enemyOrtho = EnemyRQ;
		if ( enemyOrtho )
		{
			Bitboard Occ = (AllPieces ^ (Bit << epCaptureSquare | Bit << startSquare | Bit << targetSquare));
			Bitboard Attacks = GetRookMoveMask( FriendlyKingSq, Occ );
			return !(Attacks & enemyOrtho);
		}

		return false;
	}

	void MoveGenerator::GenerateMoves( Chessboard* board )
	{
		logger.info( "Move generation started" );
		
		m_Board = board;

		if ( m_Board->m_WhiteToPlay )
		{
			FriendlyColor = Color::White;
			EnemyColor = Color::Black;

			FriendlyKingSq = m_Board->m_Bitboards.KingWhite.BitscanForward();
			Rights = m_Board->m_WhiteCastling;

			EnemyRQ = m_Board->m_Bitboards.RookBlack | m_Board->m_Bitboards.QueenBlack;
			EnemyRQ = m_Board->m_Bitboards.BishopBlack | m_Board->m_Bitboards.QueenBlack;
		}
		else
		{
			FriendlyColor = Color::Black;
			EnemyColor = Color::White;

			FriendlyKingSq = m_Board->m_Bitboards.KingBlack.BitscanForward();
			Rights = m_Board->m_BlackCastling;

			EnemyRQ = m_Board->m_Bitboards.RookWhite | m_Board->m_Bitboards.QueenWhite;
			EnemyRQ = m_Board->m_Bitboards.BishopWhite | m_Board->m_Bitboards.QueenWhite;
		}

		FriendlyPieces = m_Board->m_Bitboards.GetColorMask( FriendlyColor );
		EnemyPieces = m_Board->m_Bitboards.GetColorMask( EnemyColor );
		AllPieces = FriendlyPieces | EnemyPieces;

		WhiteToPlay = FriendlyColor == Color::White;

		ClearMoveList(); // Clear the list
		
		GenerateAttackmask();
		FindPinned();

		Bitboard Pieces = FriendlyPieces;

		while ( Pieces )
		{
			int sq = Pieces.BitscanForward();
			Pieces &= Pieces - 1;

			ChessPiece piece = m_Board->m_Bitboards.GetPieceAtSquare( sq );

			if ( InDoubleCheck && piece.type != PieceType::King )
				continue;

			switch ( piece.type )
			{
			case PieceType::King:
				GetKingMoves( sq );
				break;
			case PieceType::Pawn:
				GetPawnMoves( sq );
				break;
			case PieceType::Knight:
				GetKnightMoves( sq );
				break;
			case PieceType::Bishop:
				GetBishopMoves( sq );
				break;
			case PieceType::Rook:
				GetRookMoves( sq );
				break;
			case PieceType::Queen:
				GetQueenMoves( sq );
				break;
			case PieceType::None:
			default:
				break;
			}
		}
		logger.info( "Move generation finished. Number of legal moves: {}", MoveListTop );
	}

	std::array<Move, 218> MoveGenerator::GetMoveList() const
	{
		return (*m_Moves);
	}
}