#include "MoveGen.h"
#include "Chessboard_new.h"
#include "Bitboards.h"
#include <iostream>
#include <algorithm>

namespace Chess_Rework
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

Chess_Rework::MoveGenerator::MoveGenerator(const Chessboard_New& board) : 
	Board(board),
	
	WhiteToMove(board.m_WhiteToMove),
	FriendlyColor((board.m_WhiteToMove ? White : Black)),
	EnemyColor(~FriendlyColor),
	
	CastleRights(board.m_CastlingRights & (board.m_WhiteToMove ? White_Castling : Black_Castling)),
	KingSquare(Bitboards::bitscan_forward(
		board.m_WhiteToMove ? board.m_Bitboards.WKings : board.m_Bitboards.BKings)
	)
{
	AllPieces = board.m_Bitboards.GetAllPieces();
	FriendlyPieces = board.m_Bitboards.GetPieces(FriendlyColor);
	EnemyPieces = board.m_Bitboards.GetPieces(EnemyColor);

	CreateAttackedBitboard();
	CreatePinnedBitboard();
}

void Chess_Rework::MoveGenerator::GenMoves(std::vector<Move>& moves) const
{
	using Bitboards::from_sq, Bitboards::bitscan_forward_auto;
	if (moves.capacity() < 218)
		moves.reserve(218); // Reserve space for at least 218 moves (theoretical maximum in a position)

	moves.clear();

	memset(Bitboards::LegalTargets, 0, sizeof(Bitboards::LegalTargets));
	memset(Bitboards::PromoMask, 0, sizeof(Bitboards::PromoMask));

	Rank PromotionRank		= WhiteToMove ? Rank_8 : Rank_1;
	Direction ForwardDir	= WhiteToMove ? Dir_North : Dir_South;

	Square EPSquare			= Board.m_EP_Square;
	Square EPCaptureSq		= EPSquare - ForwardDir;

	Bitboard Pieces = FriendlyPieces;
	while (Pieces)
	{
		Square sq = bitscan_forward_auto(Pieces);
		Piece piece = Board.GetPiece(sq);
		PieceType type = type_of(piece);

		Bitboard Attacks = Bitboards::attacks(sq, type, AllPieces, FriendlyColor);
		Attacks &= ~FriendlyPieces;

		bool IsPinned = Bitboards::is_occupied(PinnedPieces, sq);

		switch (type)
		{
		case Chess_Rework::King:
			Attacks &= ~AttackedSquares;
			AddCastlingMoves(moves);
			break;
		case Chess_Rework::Pawn:
			if (CanEPCapture())
				Attacks &= EnemyPieces | from_sq(Board.m_EP_Square);
			else
				Attacks &= EnemyPieces;
			AddPawnPushes(sq, moves);
			[[fallthrough]];
		default:
			if (IsPinned)
				Attacks &= Bitboards::get_line_bb(KingSquare, sq);
			if (Check_Status > NoCheck)
				Attacks &= CheckRays;
			break;
		}

		Bitboards::LegalTargets[sq] |= Attacks;

		while (Attacks)
		{
			Square to_sq = bitscan_forward_auto(Attacks);
			bool IsCapture = Bitboards::is_occupied(EnemyPieces, to_sq);
			MoveFlag flag = MoveFlag::None;
			
			if (IsCapture)
				flag |= MoveFlag::Capture;

			if (to_sq == Board.m_EP_Square && type == Pawn)
				flag |= MoveFlag::EnPassant;

			if (rank_of(to_sq) == PromotionRank && type == Pawn)
			{
				Bitboards::PromoMask[sq] |= from_sq(to_sq);
				for (auto promotion_flag : { PromoteQueen, PromoteRook, PromoteBishop, PromoteKnight })
					moves.emplace_back(make_move(
						sq,
						to_sq,
						flag | promotion_flag
					));
			}
			else
			{
				moves.emplace_back(make_move(
					sq,
					to_sq,
					flag
				));
			}
		}
	}
}

void Chess_Rework::MoveGenerator::CreateAttackedBitboard()
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
		Piece piece = Board.GetPiece(sq);
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

void Chess_Rework::MoveGenerator::CreatePinnedBitboard()
{
	using Bitboards::bitscan_forward_auto, Bitboards::get_between_bb;
	PinnedPieces = 0;

	Bitboard opRQ = WhiteToMove ? Board.m_Bitboards.BRooks   | Board.m_Bitboards.BQueens :
								  Board.m_Bitboards.WRooks   | Board.m_Bitboards.WQueens ;
	Bitboard opBQ = WhiteToMove ? Board.m_Bitboards.BBishops | Board.m_Bitboards.BQueens :
								  Board.m_Bitboards.WBishops | Board.m_Bitboards.WQueens ;
	
	Bitboard Pinners = RookXRay(KingSquare, AllPieces, FriendlyPieces) & opRQ;
	Pinners |= BishopXRay(KingSquare, AllPieces, FriendlyPieces) & opBQ;
	while (Pinners)
	{
		Square sq = bitscan_forward_auto(Pinners);
		PinnedPieces |= get_between_bb(KingSquare, sq) & FriendlyPieces;
	}
}

void Chess_Rework::MoveGenerator::AddPawnPushes(Square sq, std::vector<Move>& moves) const
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
		Bitboards::LegalTargets[sq] |= from_sq(to_sq);

		if (promoting)
		{
			Bitboards::PromoMask[sq] |= Single;
			for (auto promotion_flag : { PromoteQueen, PromoteRook, PromoteBishop, PromoteKnight })
				moves.emplace_back(make_move(
					sq,
					to_sq,
					promotion_flag
				));
		}
		else
		{
			moves.emplace_back(make_move(
				sq,
				to_sq,
				MoveFlag::None
			));
		}
	}

	if (can_double_push && Double && !(Combined & AllPieces))
	{
		Square to_sq = bitscan_forward(Double);
		Bitboards::LegalTargets[sq] |= Double;

		moves.emplace_back(make_move(
			sq,
			to_sq,
			DoublePawnMove
		));
	}
}

void Chess_Rework::MoveGenerator::AddCastlingMoves(std::vector<Move>& moves) const
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

		if (!(KingPath & AttackedSquares) && !(RookPath && AllPieces))
		{
			Bitboards::LegalTargets[KingSquare] |= KingPath;
			moves.emplace_back(make_move(
				KingSquare,
				KingTo,
				CastleKing
			));
		}
	}
	if (CastleRights & Queen_Side)
	{
		Square KingTo = WhiteToMove ? SQ_C1 : SQ_C8;
		Bitboard KingPath = from_sq(KingSquare + Dir_West);
		KingPath |= shift(KingPath, Dir_West);

		Bitboard RookPath = KingPath | shift(KingPath, Dir_West);

		if (!(KingPath & AttackedSquares) && !(RookPath && AllPieces))
		{
			Bitboards::LegalTargets[KingSquare] |= KingPath;
			moves.emplace_back(make_move(
				KingSquare,
				KingTo,
				CastleQueen
			));
		}
	}
}

bool Chess_Rework::MoveGenerator::CanEPCapture() const
{
	using Bitboards::from_sq;
	
	if (!is_ok(Board.m_EP_Square))
		return false;
	
	Bitboard opRQ = WhiteToMove ? Board.m_Bitboards.BRooks | Board.m_Bitboards.BQueens :
								  Board.m_Bitboards.WRooks | Board.m_Bitboards.WQueens ;

	Square EPCaptureSq = Board.m_EP_Square + (WhiteToMove ? Dir_South : Dir_North);
	
	Bitboard LineThrough = Bitboards::attacks(
		KingSquare, Rook, 
		AllPieces ^ (from_sq(Board.m_EP_Square) | from_sq(EPCaptureSq))
	);
	return !(LineThrough & opRQ);
}

void Chess_Rework::MoveGenerator::GenerateMoves(const Chessboard_New& board, std::vector<Move>& moves)
{
	using Bitboards::from_sq;
	if (moves.capacity() < 218)
		moves.reserve(218); // Reserve space for at least 218 moves (theoretical maximum in a position)

	moves.clear();

	Color friendly_color	= board.m_WhiteToMove ? Color::White : Color::Black;
	Color enemy_color		= ~friendly_color;
	Square friendly_king_sq = board.m_WhiteToMove ? Bitboards::bitscan_forward(board.m_Bitboards.WKings) : 
													Bitboards::bitscan_forward(board.m_Bitboards.BKings);

	/* Generate moves checklist:
	* 1. Evaluate attacked squares and checks
	* 2. Evaluate pins and pinned pieces
	* 3. Generate moves for each piece type according to checks and pins
	*/

	CheckStatus check_status = CheckStatus::NoCheck;
	Bitboard check_rays = 0;
	Bitboard attacked_squares = GetAttackedSquares(board, enemy_color, friendly_king_sq, check_status, check_rays);

	Bitboard pinned_pieces = GetPinnedPieces(board, friendly_color, friendly_king_sq);

	Bitboard friendly_pieces = board.m_Bitboards.GetPieces(friendly_color);
	Bitboard enemy_pieces = board.m_Bitboards.GetPieces(enemy_color);
	Bitboard all_pieces = board.m_Bitboards.GetAllPieces();

	Rank start_rank			= board.m_WhiteToMove ? Rank::Rank_2 : Rank::Rank_7;
	Rank promotion_rank		= board.m_WhiteToMove ? Rank::Rank_8 : Rank::Rank_1;
	Direction forward_dir	= board.m_WhiteToMove ? Dir_North : Dir_South;

	Square ep_captured_sq = board.m_EP_Square - forward_dir;

	// Basic moves
	Bitboard pieces = friendly_pieces;
	while (pieces)
	{
		Square sq = Bitboards::bitscan_forward_auto(pieces);
		Piece piece = board.GetPiece(sq);
		PieceType piece_type = type_of(piece);
		Bitboard piece_moves = Bitboards::attacks(sq, piece_type, all_pieces, friendly_color);

		piece_moves &= ~friendly_pieces; // Remove friendly pieces from the move set

		if (check_status == DoubleCheck && piece_type != King)
			continue;

		Bitboard pin_line = 0;

		bool is_pinned = false;

		if (Bitboards::is_occupied(pinned_pieces, sq))
		{
			is_pinned = true;
			pin_line = Bitboards::get_line_bb(sq, friendly_king_sq);
		}

		if (piece_type == King)
		{
			piece_moves &= ~attacked_squares;
		}
		else if (piece_type == Pawn)
		{
			Bitboard opRQ = board.m_WhiteToMove ? board.m_Bitboards.BRooks | board.m_Bitboards.BQueens : 
												  board.m_Bitboards.WRooks | board.m_Bitboards.WQueens;
			Bitboard line_through = 0;
			if (is_ok(ep_captured_sq))
			{
				line_through = Bitboards::attacks(
					friendly_king_sq, Rook, all_pieces ^ (from_sq(ep_captured_sq) | from_sq(sq))
				);	// Check if king will be in check if pawn takes with en passant
			}

			if (is_ok(board.m_EP_Square) && !(line_through & opRQ))	// King would not be in check after en passant capture
				piece_moves &= enemy_pieces | from_sq(board.m_EP_Square);
			else
				piece_moves &= enemy_pieces; // King will be in check, pawn cannot capture en passant

			Bitboard single_push = Bitboards::shift(from_sq(sq), forward_dir);
			Bitboard double_push = Bitboards::shift(single_push, forward_dir);
			Bitboard combined = single_push | double_push;

			if (is_pinned)
			{
				single_push &= pin_line;
				double_push &= pin_line;
			}
			
			if (check_status & Check)
			{
				single_push &= check_rays;
				double_push &= check_rays;
			}

			if (single_push && !(single_push & all_pieces)) // Single push
			{
				if (rank_of(sq + forward_dir) == promotion_rank)
					for (auto flag : { PromoteQueen, PromoteRook, PromoteBishop, PromoteKnight })
						moves.emplace_back(make_move(
							sq,
							sq + forward_dir,
							flag
						));
				else
					moves.emplace_back(make_move(
						sq,
						sq + forward_dir,
						MoveFlag::None
					));

			}
			if (start_rank == rank_of(sq) && double_push && !(combined & all_pieces) ) // Double push
				moves.emplace_back(make_move(
					sq,
					sq + (2 * forward_dir),
					MoveFlag::DoublePawnMove
				));
			
		}
		if (is_pinned)
			piece_moves &= pin_line;

		if (check_status & Check && piece_type != King)
			piece_moves &= check_rays;

		while (piece_moves)
		{
			Square target_sq = Bitboards::bitscan_forward_auto(piece_moves);
			MoveFlag move_flag = None;

			if (Bitboards::is_occupied(enemy_pieces, target_sq))
			{
				move_flag |= Capture; // Capture move
			}
			else if (target_sq == board.m_EP_Square && piece_type == Pawn)
			{
				move_flag |= EnPassant; // En passant capture
			}

			if (piece_type == Pawn && rank_of(target_sq) == promotion_rank)
			{
				for (auto flag : { PromoteQueen, PromoteRook, PromoteBishop, PromoteKnight })
				{
					moves.emplace_back(make_move(
						sq,
						target_sq,
						move_flag | flag
					));
				}
			}
			else
			{
				moves.emplace_back(make_move(
					sq,
					target_sq,
					move_flag
				));
			}
		}
	}

	// Castling moves
	/*A king can castle if:
	* 1. It has the right castling right
	* 2. It is not in check
	* 3. The squares it moves through are not attacked
	* 4. The rook has an unobstructed path to its castling square
	*/
	CastlingRights rights = board.m_CastlingRights & (board.m_WhiteToMove ? White_Castling : Black_Castling);
	if (rights & CastlingRights::King_Side && check_status == NoCheck)
	{
		Bitboard rook_path = from_sq(friendly_king_sq + Dir_West) | from_sq(friendly_king_sq + (2 * Dir_West));
		Bitboard king_path = rook_path;
		if (!(rook_path & all_pieces || king_path & attacked_squares))	// Can castle kingside
			moves.emplace_back(make_move(
				friendly_king_sq,
				friendly_king_sq + (2 * Dir_West),
				MoveFlag::CastleKing
			));
	}
	
	if (rights & CastlingRights::Queen_Side && check_status == NoCheck)
	{
		Bitboard rook_path = from_sq(friendly_king_sq + Dir_East) | from_sq(friendly_king_sq + (2 * Dir_East)) | from_sq(friendly_king_sq + (3 * Dir_East));
		Bitboard king_path = rook_path ^= from_sq(friendly_king_sq + (3 * Dir_East));
		if (!(rook_path & all_pieces || king_path & attacked_squares))	// Can castle queenside
			moves.emplace_back(make_move(
				friendly_king_sq,
				friendly_king_sq + (2 * Dir_East),
				MoveFlag::CastleQueen
			));
	}
}

int Chess_Rework::MoveGenerator::RunPerft(const std::string& FEN, int depth)
{
	Chessboard_New board;
	board.LoadFEN(FEN);

	std::vector<Move> moves;
	GenerateMoves(board, moves);

	int result = 0, move_result = 0;

	for (const auto& move : moves)
	{
		board.MakeMove(move);
		move_result = Perft(board, depth - 1);
		std::cout << GetMoveRepr(move) << ": " << move_result << "\n";
		result += move_result;
		board.UnMakeMove(move);
	}

	return result;
}

Chess_Rework::Bitboard Chess_Rework::MoveGenerator::GetAttackedSquares(const Chessboard_New& board, Color enemy_color, Square king_sq, CheckStatus& check_status, Bitboard& check_rays)
{
	using Bitboards::from_sq, Bitboards::get_between_bb;
	Bitboard enemies = board.m_Bitboards.GetPieces(enemy_color);
	Bitboard all_pieces = board.m_Bitboards.GetAllPieces();
	Bitboard attacked_squares = 0;

	Bitboard king_sq_bb = from_sq(king_sq);

	while (enemies)
	{
		Square sq = Bitboards::bitscan_forward_auto(enemies);
		Piece piece = board.GetPiece(sq);
		PieceType piece_type = type_of(piece);

		Bitboard sq_attacks = 0;

		if (piece_type == PieceType::Pawn)
			sq_attacks |= Bitboards::attacks(sq, color_of(piece));
		else
			sq_attacks |= Bitboards::attacks(sq, piece_type, all_pieces ^ king_sq_bb);

		if (king_sq_bb & sq_attacks)
		{
			check_rays |= from_sq(sq);
			if (piece_type != Pawn && piece_type != Knight)
				check_rays |= get_between_bb(king_sq, sq);

			if (check_status & Check)
				check_status |= DoubleCheck;
			check_status |= Check;
		}

		attacked_squares |= sq_attacks;
	}
	return attacked_squares;
}

Chess_Rework::Bitboard Chess_Rework::MoveGenerator::GetPinnedPieces(const Chessboard_New& board, Color friendly_color, Square king_sq)
{
	Bitboard pinned = 0;

	Bitboard opRQ = friendly_color == Color::White ? board.m_Bitboards.BRooks | board.m_Bitboards.BQueens : 
													 board.m_Bitboards.WRooks | board.m_Bitboards.WQueens;
	Bitboard opBQ = friendly_color == Color::White ? board.m_Bitboards.BBishops | board.m_Bitboards.BQueens : 
													 board.m_Bitboards.WBishops | board.m_Bitboards.WQueens;

	Bitboard friendly_pieces = board.m_Bitboards.GetPieces(friendly_color);
	Bitboard all_pieces = board.m_Bitboards.GetAllPieces();

	Bitboard pinners;
	pinners = RookXRay(king_sq, all_pieces, friendly_pieces) & opRQ;
	while (pinners)
	{
		Square sq = Bitboards::bitscan_forward_auto(pinners);
		pinned |= Bitboards::BetweenBB[king_sq][sq] & friendly_pieces;
	}
	pinners = BishopXRay(king_sq, all_pieces, friendly_pieces) & opBQ;
	while (pinners)
	{
		Square sq = Bitboards::bitscan_forward_auto(pinners);
		pinned |= Bitboards::BetweenBB[king_sq][sq] & friendly_pieces;
	}

	return pinned;
}

Chess_Rework::Bitboard Chess_Rework::MoveGenerator::RookXRay(Square sq, Bitboard occupied, Bitboard blockers)
{
	Bitboard attacks = Bitboards::attacks(sq, PieceType::Rook, occupied);
	blockers &= attacks;
	return attacks ^ Bitboards::attacks(sq, PieceType::Rook, occupied ^ blockers);
}

Chess_Rework::Bitboard Chess_Rework::MoveGenerator::BishopXRay(Square sq, Bitboard occupied, Bitboard blockers)
{
	Bitboard attacks = Bitboards::attacks(sq, PieceType::Bishop, occupied);
	blockers &= attacks;
	return attacks ^ Bitboards::attacks(sq, PieceType::Bishop, occupied ^ blockers);
}

int Chess_Rework::MoveGenerator::Perft(Chessboard_New& board, int depth)
{
	if (depth <= 0)
		return 1;

	std::vector<Move> moves;
	int nodes = 0;

	GenerateMoves(board, moves);
	for (const auto& move : moves)
	{
		board.MakeMove(move);
		nodes += Perft(board, depth - 1);
		board.UnMakeMove(move);
	}
	return nodes;
}