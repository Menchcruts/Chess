#include "Chessboard_new.h"
#include "MoveGen.h"
#include <ranges>
#include <string>
#include <algorithm>
#include <charconv>
#include <iostream>

void Chess_Rework::Chessboard_New::ResetBoard()
{
    // Reset the board to the initial position
    LoadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void Chess_Rework::Chessboard_New::PlacePiece(Square sq, Piece piece)
{
	if (!is_ok(sq))
		return;

	RemovePiece(sq);

    m_BoardArray[sq] = piece;
	m_Bitboards.SetBit(sq, piece);
}

Chess_Rework::Piece Chess_Rework::Chessboard_New::RemovePiece(Square sq)
{
	if ( !is_ok(sq))
		return Piece::NoPiece;

	Piece piece = m_BoardArray[sq];
	m_BoardArray[sq] = NoPiece;
	m_Bitboards.ClearBit(sq, piece);
	return piece;
}

Chess_Rework::Piece Chess_Rework::Chessboard_New::GetPiece(Square sq) const
{
    if (!is_ok(sq))
		return NoPiece;
    return m_BoardArray[sq];
}

/* Takes a string representing a square in normal chess notation, i.e. a5, b7 and so on, and converts it into a Square enum. */
static Chess_Rework::Square square_from_string(const std::string& sq_as_str)
{
    if (sq_as_str.size() != 2)
        return Chess_Rework::NoSquare;
    char file = sq_as_str[0];
    char rank = sq_as_str[1];
    if (file < 'a' || file > 'h' || rank < '1' || rank > '8')
        return Chess_Rework::NoSquare;
    return Chess_Rework::Square((rank - '1') * 8 + (file - 'a'));
}

void Chess_Rework::Chessboard_New::MakeMove(Move move)
{    
    // TODO: Implement null move logic
    
    Square from_sq = from_square(move);
	Square to_sq = to_square(move);
	MoveFlag flag = move_flag(move);

    //std::cout << "Making move: " << move << " from " << from_sq << " to " << to_sq << " with flag: " << flag << "\n";
    
    Piece moving_piece = GetPiece(from_sq);
	PieceType moving_piece_type = type_of(moving_piece);
    if (moving_piece == NoPiece)
		return;

    Square capture_sq;
	bool is_capture = (flag & MoveFlag::Capture) != MoveFlag::None;
    if (is_capture && flag == MoveFlag::EnPassant)
		capture_sq = to_sq + (moving_piece == W_Pawn ? Dir_South : Dir_North);
    else
		capture_sq = to_sq;

    Piece captured_piece = GetPiece(capture_sq);
	PieceType captured_piece_type = type_of(captured_piece);

    Piece PromotionPiece = NoPiece;

    if (flag & MoveFlag::PromoteKnight)
    {
        PromotionPiece = make_piece(m_WhiteToMove ? White : Black, GetPromotionPiece(flag));
    }

    m_PrevStates.emplace_back(PrevState{
        .HalfMoveClock = m_HalfMoveClock,
        .EP_Square = m_EP_Square,
		.CastlingRights = m_CastlingRights,
        .CapturedPiece = captured_piece
		});

	m_MoveHistory.emplace_back(move);

    Direction forward_dir = m_WhiteToMove ? Dir_North : Dir_South;

    // Update the board and bitboards
	RemovePiece(from_sq);
    if (is_capture)
        RemovePiece(capture_sq);

    if (PromotionPiece != NoPiece)
        PlacePiece(to_sq, PromotionPiece);
    else
	    PlacePiece(to_sq, moving_piece);

    // Update castling rights
	CastlingRights color_mask   = m_WhiteToMove ? CastlingRights::White_Castling : CastlingRights::Black_Castling;
	CastlingRights castleOO     = m_WhiteToMove ? CastlingRights::White_OO : CastlingRights::Black_OO;
	CastlingRights castleOOO    = m_WhiteToMove ? CastlingRights::White_OOO : CastlingRights::Black_OOO;

    if (moving_piece_type == King && has_castling_rights(m_CastlingRights, color_mask))
		m_CastlingRights ^= color_mask; // Remove castling rights for the color

    if (moving_piece_type == PieceType::Rook)
    {
		Square OO_rook_sq     = m_WhiteToMove ? Square::SQ_H1 : Square::SQ_H8;
        Square OOO_rook_sq    = m_WhiteToMove ? Square::SQ_A1 : Square::SQ_A8;
        
        if (from_sq == OO_rook_sq && has_castling_rights(m_CastlingRights, castleOO))
            m_CastlingRights ^= castleOO; // Remove kingside castling rights
        
        else if (from_sq == OOO_rook_sq && has_castling_rights(m_CastlingRights, castleOOO))
            m_CastlingRights ^= castleOOO; // Remove queenside castling rights
    }
    
	// Update en passant square
    if (flag == MoveFlag::DoublePawnMove)
        m_EP_Square = to_sq - forward_dir;
    else
        m_EP_Square = NoSquare; // Clear en passant square

    // Handle castling
    if (flag == MoveFlag::CastleKing)
    {
		Square rook_from_sq = (m_WhiteToMove) ? Square::SQ_H1 : Square::SQ_H8;
		Square rook_to_sq = (m_WhiteToMove) ? Square::SQ_F1 : Square::SQ_F8;
		PlacePiece(rook_to_sq, GetPiece(rook_from_sq));
        RemovePiece(rook_from_sq);
    }
    else if (flag == MoveFlag::CastleQueen)
    {
		Square rook_from_sq = (m_WhiteToMove) ? Square::SQ_A1 : Square::SQ_A8;
		Square rook_to_sq = (m_WhiteToMove) ? Square::SQ_D1 : Square::SQ_D8;
        PlacePiece(rook_to_sq, GetPiece(rook_from_sq));
        RemovePiece(rook_from_sq);
	}

	// Update clocks and swap turn
    if (!m_WhiteToMove)
		++m_FullMoveClock; // Increment full move clock on black move

	if (moving_piece_type == PieceType::Pawn || is_capture)
        m_HalfMoveClock = 0; // Reset half move clock on pawn move or capture
    else
        ++m_HalfMoveClock; // Increment half move clock otherwise

	m_WhiteToMove = !m_WhiteToMove; // Switch turn
    GenerateMoves();
}

void Chess_Rework::Chessboard_New::UnMakeMove(Move move)
{
    if (m_MoveHistory.empty() || move != m_MoveHistory.back())
        return; // Future logging

	m_MoveHistory.pop_back();
	PrevState prev_state = m_PrevStates.back(); m_PrevStates.pop_back();

	Square from_sq = from_square(move);
	Square to_sq = to_square(move);
    MoveFlag flag = move_flag(move);

	bool is_capture = (flag & MoveFlag::Capture) != MoveFlag::None;

	Piece moving_piece = GetPiece(to_sq);
	PieceType moving_piece_type = type_of(moving_piece);

	Piece captured_piece = prev_state.CapturedPiece;
	PieceType captured_piece_type = type_of(captured_piece);

    bool WasPromotion = bool(flag & PromoteKnight);

	m_CastlingRights = prev_state.CastlingRights;
	m_EP_Square = prev_state.EP_Square;
	m_HalfMoveClock = prev_state.HalfMoveClock;
	
    m_WhiteToMove = !m_WhiteToMove; // Switch turn back
    
    if (!m_WhiteToMove)
		--m_FullMoveClock; // Decrement full move clock on black move

    Square capture_sq;
    if (flag == MoveFlag::EnPassant)
		capture_sq = to_sq + (moving_piece == W_Pawn ? Dir_South : Dir_North);
	else
		capture_sq = to_sq;

	RemovePiece(to_sq);
    if (is_capture)
		PlacePiece(capture_sq, captured_piece);
    if (WasPromotion)
        PlacePiece(from_sq, make_piece(m_WhiteToMove ? White : Black, Pawn));
    else
        PlacePiece(from_sq, moving_piece);

    if (flag == MoveFlag::CastleKing)
    {
        Square rook_from_sq = (m_WhiteToMove) ? Square::SQ_F1 : Square::SQ_F8;
        Square rook_to_sq = (m_WhiteToMove) ? Square::SQ_H1 : Square::SQ_H8;
        PlacePiece(rook_to_sq, GetPiece(rook_from_sq));
        RemovePiece(rook_from_sq);
    }
    else if (flag == MoveFlag::CastleQueen)
    {
        Square rook_from_sq = (m_WhiteToMove) ? Square::SQ_D1 : Square::SQ_D8;
        Square rook_to_sq = (m_WhiteToMove) ? Square::SQ_A1 : Square::SQ_A8;
        PlacePiece(rook_to_sq, GetPiece(rook_from_sq));
		RemovePiece(rook_from_sq);
    }

    GenerateMoves();
}

void Chess_Rework::Chessboard_New::LoadFEN(const std::string_view& FEN_String)
{
	std::vector<std::string> FEN_Tokens;
    for (auto&& token : FEN_String | std::views::split(' ')) {
        FEN_Tokens.emplace_back(token.begin(), token.end());
	}

    if (FEN_Tokens.size() != 6)
    {
		LoadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
		return;
    }

	std::string BoardString = FEN_Tokens[0];
	std::string CastleString = FEN_Tokens[2];
	
	// Basic metadata
    m_WhiteToMove = (FEN_Tokens[1] == "w");
	m_EP_Square = FEN_Tokens[3] == "-" ? NoSquare : square_from_string(FEN_Tokens[3]);
	
	m_HalfMoveClock = 0;
	m_FullMoveClock = 1;
	std::from_chars(FEN_Tokens[4].data(), FEN_Tokens[4].data() + FEN_Tokens[4].size(), m_HalfMoveClock);
	std::from_chars(FEN_Tokens[5].data(), FEN_Tokens[5].data() + FEN_Tokens[5].size(), m_FullMoveClock);


	// Set castling rights
	m_CastlingRights = CastlingRights::No_Castling;

    for (const auto& castle_char : CastleString)
    {
        switch (castle_char)
        {
            case 'K': m_CastlingRights |= CastlingRights::White_OO; break; // White kingside
            case 'Q': m_CastlingRights |= CastlingRights::White_OOO; break; // White queenside
            case 'k': m_CastlingRights |= CastlingRights::Black_OO; break; // Black kingside
            case 'q': m_CastlingRights |= CastlingRights::Black_OOO; break; // Black queenside
            default: break; // Ignore any other characters
        }
	}

	// Set the board
    m_PrevStates.clear();
	m_BoardArray.fill(NoPiece);
	m_Bitboards = PieceBitboards{}; // Reset all bitboards

    auto split = BoardString | std::views::split('/')
                             | std::views::transform([](const auto& str) {
                                return std::string(str.data(), str.size());
								});

    auto RankStrings = std::vector(split.begin(), split.end());
    std::reverse(RankStrings.begin(), RankStrings.end());

    Square CurrentSquare = Square::SQ_A1;
    for (const auto& RankString : RankStrings)
    {
		bool IsWhite;
        for (const auto& Char : RankString)
        {
            if ('0' <= Char && Char <= '9')
            {
				int skip = Char - '0';
                CurrentSquare += Square(skip); // Skip squares
                continue;
            }
            else if ('a' <= Char && Char <= 'z') // Char is a black piece
            {
				IsWhite = false;
            }
            else if ('A' <= Char && Char <= 'Z') // Char is a white piece
            {
                IsWhite = true;
			}
            else
            {
				// Maybe logging
                continue; // Invalid character, skip
            }

			Piece NewPiece = NoPiece;

			if (Char == 'k' || Char == 'K')
				NewPiece = IsWhite ? W_King : B_King;
			else if (Char == 'q' || Char == 'Q')
				NewPiece = IsWhite ? W_Queen : B_Queen;
			else if (Char == 'r' || Char == 'R')
				NewPiece = IsWhite ? W_Rook : B_Rook;
			else if (Char == 'b' || Char == 'B')
				NewPiece = IsWhite ? W_Bishop : B_Bishop;
			else if (Char == 'n' || Char == 'N')
				NewPiece = IsWhite ? W_Knight : B_Knight;
			else if (Char == 'p' || Char == 'P')
				NewPiece = IsWhite ? W_Pawn : B_Pawn;
            else { /* Maybe logging */ }

			if (NewPiece != NoPiece)
                PlacePiece(CurrentSquare, NewPiece);

			CurrentSquare += Square(1); // Move to the next square
        }
	}

    GenerateMoves();
}

/* This method creates a move with necessary flags for the user. */
Chess_Rework::Move Chess_Rework::Chessboard_New::CreateMove(Square From, Square To, PieceType PromotionType) const
{
    PieceType PieceType = type_of(GetPiece(From));
    MoveFlag flag = MoveFlag::None;

    if (GetPiece(To) != Piece::NoPiece)
        flag |= Capture;
    else if (To == m_EP_Square)
        flag |= EnPassant;

    if (PieceType == Pawn)
    {
        switch (PromotionType)
        {
        case Queen:
            flag |= PromoteQueen; break;
        case Rook:
            flag |= PromoteRook; break;
        case Bishop:
            flag |= PromoteBishop; break;
        case Knight:
            flag |= PromoteKnight; break;
        default: break;
        }
        if (Bitboards::distance(From, To) == 2)
            flag |= DoublePawnMove;
    }
    else if (PieceType == King && Bitboards::distance(From, To) == 2)
    {
        Square KingSide     = m_WhiteToMove ? SQ_G1 : SQ_G8;
        Square QueenSide    = m_WhiteToMove ? SQ_C1 : SQ_C8;
        if (To == KingSide)
            flag |= CastleKing;
        else if (To == QueenSide)
            flag |= CastleQueen;
    }

    return make_move(From, To, flag);
}

void Chess_Rework::Chessboard_New::GenerateMoves()
{
    MoveGenerator gen(*this);
    gen.GenMoves(m_Moves);
    //MoveGenerator::GenerateMoves(*this, m_Moves);
}

Chess_Rework::PieceType Chess_Rework::Chessboard_New::GetPromotionPiece(MoveFlag flag) const
{
    flag = MoveFlag(flag & 0b1011); // Filter out the capture flag if its there
    
    if ((flag & PromoteKnight) == flag)
        return Knight;
    else if ((flag & PromoteBishop) == flag)
        return Bishop;
    else if ((flag & PromoteRook) == flag)
        return Rook;
    else if ((flag & PromoteQueen) == flag)
        return Queen;

    return NoPieceType;
}
