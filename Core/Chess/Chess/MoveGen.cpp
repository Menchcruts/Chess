#include "MoveGen.h"
#include "Chessboard_new.h"
#include "Bitboards.h"
#include <iostream>
#include <algorithm>

namespace Chess
{
	static std::string GetMoveRepr(Move move)
	{
		const char* files = "abcdefgh";
		const char* ranks = "12345678";

		short start = from_square(move);
		short start_rank = start >> 3;
		short start_file = start & 7;

		short target = to_square(move);
		short target_rank = target >> 3;
		short target_file = target & 7;

		std::string result = { files[start_file], ranks[start_rank], files[target_file], ranks[target_rank] };

		MoveFlag flag = move_flag(move);
		if ((flag & MoveFlag::PromoteKnight) != MoveFlag::None)
		{
			if (flag == MoveFlag::PromoteKnight || flag == MoveFlag::PromoteKnightCapture)
				result.push_back('n');
			else if (flag == MoveFlag::PromoteBishop || flag == MoveFlag::PromoteBishopCapture)
				result.push_back('b');
			else if (flag == MoveFlag::PromoteRook || flag == MoveFlag::PromoteRookCapture)
				result.push_back('r');
			else if (flag == MoveFlag::PromoteQueen || flag == MoveFlag::PromoteQueenCapture)
				result.push_back('q');
		}
		return result;
	}
}

//Chess::MoveGenerator::MoveGenerator(const Chessboard* board) : 
//	Board(board),
//	
//	WhiteToMove(board->m_WhiteToMove),
//	FriendlyColor((board->m_WhiteToMove ? White : Black)),
//	EnemyColor(~FriendlyColor),
//	
//	CastleRights(board->m_CastlingRights & (board->m_WhiteToMove ? White_Castling : Black_Castling)),
//	KingSquare(Bitboards::bitscan_forward(
//		board->m_WhiteToMove ? board->m_Bitboards.WKings : board->m_Bitboards.BKings)
//	)
//{
//	AllPieces = board->m_Bitboards.GetAllPieces();
//	FriendlyPieces = board->m_Bitboards.GetPieces(FriendlyColor);
//	EnemyPieces = board->m_Bitboards.GetPieces(EnemyColor);
//
//	CreateAttackedBitboard();
//	CreatePinnedBitboard();
//}

void Chess::MoveGenerator::Load(const Chessboard* board)
{
	Board = board;

	WhiteToMove = board->m_WhiteToMove;
	FriendlyColor = WhiteToMove ? White : Black;
	EnemyColor = ~FriendlyColor;

	CastleRights = board->m_CastlingRights & (WhiteToMove ? White_Castling : Black_Castling);
	KingSquare = Bitboards::bitscan_forward(
		WhiteToMove ? board->m_Bitboards.WKings : board->m_Bitboards.BKings
	);

	AllPieces = board->m_Bitboards.GetAllPieces();
	FriendlyPieces = board->m_Bitboards.GetPieces(FriendlyColor);
	EnemyPieces = board->m_Bitboards.GetPieces(EnemyColor);

	CreateAttackedBitboard();
	CreatePinnedBitboard();
}

std::vector<Chess::Move> Chess::MoveGenerator::GenerateMoves(const Chessboard* board)
{
	using Bitboards::from_sq, Bitboards::bitscan_forward_auto;
	
	Load(board);

	std::vector<Move> moves; 
	moves.reserve(218); // Reserve space for at least 218 moves (theoretical maximum in a position)

	memset(Bitboards::LegalTargets, 0, sizeof(Bitboards::LegalTargets));
	memset(Bitboards::PromoMask, 0, sizeof(Bitboards::PromoMask));

	Bitboard Pieces = FriendlyPieces;
	while (Pieces)
	{
		Square sq = bitscan_forward_auto(Pieces);
		Piece piece = Board->GetPiece(sq);

		PerPiece(sq, piece, moves);
	}
	return moves;
}

void Chess::MoveGenerator::PerPiece(Square sq, Piece piece, std::vector<Move>& moves) const
{
	namespace BB = Bitboards;

	PieceType type = type_of(piece);

	Bitboard Attacks = BB::attacks(sq, type, AllPieces, FriendlyColor);
	Attacks &= ~FriendlyPieces;

	bool IsPinned = BB::is_occupied(PinnedPieces, sq);

	switch (type)
	{
	case King:
		Attacks &= ~AttackedSquares;
		AddCastlingMoves(moves);
		break;
	case Pawn:
		if (CanEPCapture())
			Attacks &= EnemyPieces | BB::from_sq(Board->m_EP_Square);
		else
			Attacks &= EnemyPieces;
		AddPawnPushes(sq, moves);
		[[fallthrough]];
	default:
		if (IsPinned)
			Attacks &= BB::get_line_bb(KingSquare, sq);
		if (Check_Status > NoCheck)
			Attacks &= CheckRays;
		break;
	}

	AddAttacks(sq, type, Attacks, moves);
}

void Chess::MoveGenerator::AddAttacks(Square sq, PieceType type, Bitboard Attacks, std::vector<Move>& moves) const
{
	namespace BB = Bitboards;
	
	Rank PromotionRank = WhiteToMove ? Rank_8 : Rank_1;

	BB::LegalTargets[sq] |= Attacks;

	while (Attacks)
	{
		Square to_sq = BB::bitscan_forward_auto(Attacks);
		bool IsCapture = BB::is_occupied(EnemyPieces, to_sq);
		MoveFlag flag = MoveFlag::None;

		if (IsCapture)
			flag |= MoveFlag::Capture;

		if (to_sq == Board->m_EP_Square && type == Pawn)
			flag |= MoveFlag::EnPassant;

		if (rank_of(to_sq) == PromotionRank && type == Pawn)
		{
			BB::PromoMask[sq] |= BB::from_sq(to_sq);
			AddMove(sq, to_sq, { flag | PromoteQueen, flag | PromoteRook, flag | PromoteBishop, flag | PromoteKnight }, moves);
		}
		else
		{
			AddMove(sq, to_sq, flag, moves);
		}
	}
}

void Chess::MoveGenerator::AddMove(Square sq, Square to_sq, std::vector<MoveFlag> flags, std::vector<Move>& moves) const
{
	for (auto& flag : flags)
		moves.emplace_back(make_move(
			sq,
			to_sq,
			flag
		));
}

void Chess::MoveGenerator::AddMove(Square sq, Square to_sq, MoveFlag flag, std::vector<Move>& moves) const
{
	moves.emplace_back(make_move(
		sq,
		to_sq,
		flag
	));
}

void Chess::MoveGenerator::CreateAttackedBitboard()
{
	using Bitboards::from_sq, Bitboards::bitscan_forward_auto, Bitboards::get_between_bb;

	AttackedSquares = 0;
	CheckRays = 0;
	Check_Status = NoCheck;

	Bitboard KingSq_BB = from_sq(KingSquare);

	Bitboard Enemies = EnemyPieces;
	while (Enemies)
	{
		Square sq = bitscan_forward_auto(Enemies);
		Piece piece = Board->GetPiece(sq);
		PieceType type = type_of(piece);

		Bitboard piece_attacks = 0;

		piece_attacks |= Bitboards::attacks(sq, type, AllPieces ^ KingSq_BB, EnemyColor);

		if (KingSq_BB & piece_attacks)
		{
			CheckRays |= from_sq(sq);
			if (type != Pawn && type != Knight)
				CheckRays |= get_between_bb(KingSquare, sq);

			if (Check_Status & Check)
				Check_Status |= DoubleCheck;
			Check_Status |= Check;
		}

		AttackedSquares |= piece_attacks;
	}
}

void Chess::MoveGenerator::CreatePinnedBitboard()
{
	using Bitboards::bitscan_forward_auto, Bitboards::get_between_bb;
	PinnedPieces = 0;

	Bitboard opRQ = WhiteToMove ? Board->m_Bitboards.BRooks   | Board->m_Bitboards.BQueens :
								  Board->m_Bitboards.WRooks   | Board->m_Bitboards.WQueens ;
	Bitboard opBQ = WhiteToMove ? Board->m_Bitboards.BBishops | Board->m_Bitboards.BQueens :
								  Board->m_Bitboards.WBishops | Board->m_Bitboards.WQueens ;
	
	Bitboard Pinners = RookXRay(KingSquare, AllPieces, FriendlyPieces) & opRQ;
	Pinners |= BishopXRay(KingSquare, AllPieces, FriendlyPieces) & opBQ;
	while (Pinners)
	{
		Square sq = bitscan_forward_auto(Pinners);
		PinnedPieces |= get_between_bb(KingSquare, sq) & FriendlyPieces;
	}
}

void Chess::MoveGenerator::AddPawnPushes(Square sq, std::vector<Move>& moves) const
{
	using Bitboards::from_sq, Bitboards::bitscan_forward;

	Rank StartRank			= WhiteToMove ? Rank_2 : Rank_7;
	Rank PromotionRank		= WhiteToMove ? Rank_8 : Rank_1;
	Direction ForwardDir	= WhiteToMove ? Dir_North : Dir_South;

	Bitboard Single = from_sq(sq + ForwardDir);
	Bitboard Double = Bitboards::shift(Single, ForwardDir);
	
	if (Bitboards::is_occupied(PinnedPieces, sq))
	{
		Bitboard PinLine = Bitboards::get_line_bb(KingSquare, sq);
		Single &= PinLine;
		Double &= PinLine;
	}
	if (Check_Status > NoCheck)
	{
		Single &= CheckRays;
		Double &= CheckRays;
	}

	Bitboard Combined = Single | Double;

	bool promoting = rank_of(sq + ForwardDir) == PromotionRank;
	bool can_double_push = rank_of(sq) == StartRank;

	if (!(Single & AllPieces))	// Single push
	{
		Square to_sq = bitscan_forward(Single);
		Bitboards::LegalTargets[sq] |= Single;

		if (promoting)
		{
			Bitboards::PromoMask[sq] |= Single;
			AddMove(sq, to_sq, { PromoteQueen, PromoteRook, PromoteBishop, PromoteKnight }, moves);
		}
		else
		{
			AddMove(sq, to_sq, MoveFlag::None, moves);
		}
	}

	if (can_double_push && Double && !(Combined & AllPieces))
	{
		Square to_sq = bitscan_forward(Double);
		Bitboards::LegalTargets[sq] |= Double;

		AddMove(sq, to_sq, DoublePawnMove, moves);
	}
}

void Chess::MoveGenerator::AddCastlingMoves(std::vector<Move>& moves) const
{
	using Bitboards::from_sq, Bitboards::shift;

	if (Check_Status > NoCheck)
		return;	// Can't castle if in check

	if (CastleRights & King_Side)
	{
		Square KingTo = WhiteToMove ? SQ_G1 : SQ_G8;
		Bitboard KingPath = from_sq(KingSquare + Dir_East);
		KingPath |= shift(KingPath, Dir_East);

		Bitboard RookPath = KingPath;
		if (!(KingPath & AttackedSquares) && !(RookPath & AllPieces))
		{
			Bitboards::LegalTargets[KingSquare] |= KingPath;
			AddMove(KingSquare, KingTo, CastleKing, moves);
		}
	}
	if (CastleRights & Queen_Side)
	{
		Square KingTo = WhiteToMove ? SQ_C1 : SQ_C8;
		Bitboard KingPath = from_sq(KingSquare + Dir_West);
		KingPath |= shift(KingPath, Dir_West);

		Bitboard RookPath = KingPath | shift(KingPath, Dir_West);

		if (!(KingPath & AttackedSquares) && !(RookPath & AllPieces))
		{
			Bitboards::LegalTargets[KingSquare] |= KingPath;
			AddMove(KingSquare, KingTo, CastleQueen, moves);
		}
	}
}

bool Chess::MoveGenerator::CanEPCapture() const
{
	using Bitboards::from_sq;
	
	if (!is_ok(Board->m_EP_Square))
		return false;
	
	Bitboard opRQ = WhiteToMove ? Board->m_Bitboards.BRooks | Board->m_Bitboards.BQueens :
								  Board->m_Bitboards.WRooks | Board->m_Bitboards.WQueens ;

	Square EPCaptureSq = Board->m_EP_Square + (WhiteToMove ? Dir_South : Dir_North);
	
	Bitboard LineThrough = Bitboards::attacks(
		KingSquare, Rook, 
		AllPieces ^ (from_sq(Board->m_EP_Square) | from_sq(EPCaptureSq))
	);
	return !(LineThrough & opRQ);
}

Chess::Bitboard Chess::MoveGenerator::RookXRay(Square sq, Bitboard occupied, Bitboard blockers)
{
	Bitboard attacks = Bitboards::attacks(sq, PieceType::Rook, occupied);
	blockers &= attacks;
	return attacks ^ Bitboards::attacks(sq, PieceType::Rook, occupied ^ blockers);
}

Chess::Bitboard Chess::MoveGenerator::BishopXRay(Square sq, Bitboard occupied, Bitboard blockers)
{
	Bitboard attacks = Bitboards::attacks(sq, PieceType::Bishop, occupied);
	blockers &= attacks;
	return attacks ^ Bitboards::attacks(sq, PieceType::Bishop, occupied ^ blockers);
}
