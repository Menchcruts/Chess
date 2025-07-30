#pragma once
#include <cstdint>

namespace Chess_Rework
{
	class Chessboard;

	enum Square
	{
		SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
		SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
		SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
		SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
		SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
		SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
		SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
		SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,

		NoSquare
	};

	enum File
	{
		File_A,
		File_B,
		File_C,
		File_D,
		File_E,
		File_F,
		File_G,
		File_H
	};

	enum Rank
	{
		Rank_1,
		Rank_2,
		Rank_3,
		Rank_4,
		Rank_5,
		Rank_6,
		Rank_7,
		Rank_8
	};

	enum Direction
	{
		Dir_North = 8,
		Dir_South = -Dir_North,
		Dir_East = 1,
		Dir_West = -Dir_East,

		Dir_NorthEast = Dir_North + Dir_East,
		Dir_NorthWest = Dir_North + Dir_West,
		Dir_SouthEast = Dir_South + Dir_East,
		Dir_SouthWest = Dir_South + Dir_West,
	};

	inline constexpr bool is_ok( Square sq ) { return SQ_A1 <= sq && sq <= SQ_H8; }

	inline constexpr Square make_square( File f, Rank r ) { return Square( f + (r << 3) ); }
	inline constexpr File file_of( Square sq ) { return File( sq & 7 ); }
	inline constexpr Rank rank_of( Square sq ) { return Rank( sq >> 3 ); }

	inline constexpr Square operator +( Square sq, Direction dir ) { return Square( int( sq ) + int( dir ) ); }
	inline constexpr Square operator -( Square sq, Direction dir ) { return Square( int( sq ) - int( dir ) ); }

	inline constexpr Square& operator +=( Square& sq, Direction dir ) { return sq = sq + dir; }
	inline constexpr Square& operator -=( Square& sq, Direction dir ) { return sq = sq - dir; }

	inline constexpr Direction operator *( int i, Direction dir ) { return Direction( i * int( dir ) ); }
	inline constexpr Direction operator +( Direction dir1, Direction dir2 ) { return Direction( int( dir1 ) + int( dir2 ) ); }

#define ENABLE_INCS_OPERATORS_ON(T) \
	inline constexpr T& operator++(T& v) {return v = T(int(v) + 1);} \
	inline constexpr T& operator--(T& v) {return v = T(int(v) - 1);} \
	inline constexpr T& operator++(T& v, int) {T temp = v; ++v; return temp;} \
	inline constexpr T& operator--(T& v, int) {T temp = v; --v; return temp;}
	// End of ENABLE_INCS_OPERATORS_ON

	ENABLE_INCS_OPERATORS_ON( Square );

#undef ENABLE_INCS_OPERATORS_ON


	enum Color
	{
		White,
		Black,
		NoColor
	};

	inline constexpr Color operator ~( Color c ) { return Color(c ^ Black); }

	inline constexpr Square relative_square( Color c, Square sq ) { return Square(sq ^ (c * 56)); }
	inline constexpr Rank relative_rank( Color c, Rank r ) { return Rank( r ^ (c * 7) ); }


	enum PieceType
	{
		NoPieceType,
		King,
		Pawn,
		Knight,
		Bishop,
		Rook,
		Queen
	};

	enum Piece
	{
		NoPiece,

		W_King = King,
		W_Pawn,
		W_Knight,
		W_Bishop,
		W_Rook,
		W_Queen,

		B_King = W_King + 8,
		B_Pawn,
		B_Knight,
		B_Bishop,
		B_Rook,
		B_Queen
	};

	inline constexpr Color color_of( Piece p ) { return Color(p >> 3); }
	inline constexpr PieceType type_of( Piece p ) { return PieceType( p & 7 ); }
	inline constexpr bool is_type_of( Piece p, PieceType type ) { return type_of( p ) == type; }
	inline constexpr Piece make_piece( Color c, PieceType type ) { return Piece( type + (c << 3) ); }

	enum CastlingRights
	{
		No_Castling,
		White_OO,
		White_OOO = White_OO << 1,
		Black_OO = White_OO << 2,
		Black_OOO = White_OO << 3,

		King_Side = White_OO | Black_OO,
		Queen_Side = White_OOO | Black_OOO,
		White_Castling = White_OO | White_OOO,
		Black_Castling = Black_OO | Black_OOO,
		Any_Castling = White_Castling | Black_Castling,
	};

	inline constexpr CastlingRights operator ^( CastlingRights cr1, CastlingRights cr2 ) { return CastlingRights( int( cr1 ) ^ int( cr2 ) ); }
	inline constexpr CastlingRights& operator ^=( CastlingRights& cr1, CastlingRights cr2 ) { return cr1 = cr1 ^ cr2; }

	inline constexpr CastlingRights operator |( CastlingRights cr1, CastlingRights cr2 ) { return CastlingRights( int( cr1 ) | int( cr2 ) ); }
	inline constexpr CastlingRights& operator |=( CastlingRights& cr1, CastlingRights cr2 ) { return cr1 = cr1 | cr2; }

	inline constexpr CastlingRights operator &( Color c, CastlingRights cr ) { return CastlingRights((c == White ? White_Castling : Black_Castling) & cr); }

	using Bitboard = std::uint64_t;

	using Move = std::uint16_t;

	enum class MoveFlag : std::uint16_t
	{
		None = 0b0000,
		DoublePawnMove = 0b0001,

		CastleKing = 0b0010,
		CastleQueen = 0b0011,

		Capture = 0b0100,
		EnPassant = 0b0101,

		PromoteKnight = 0b1000,
		PromoteBishop = 0b1001,
		PromoteRook = 0b1010,
		PromoteQueen = 0b1011,

		PromoteKnightCapture = PromoteKnight | Capture,
		PromoteBishopCapture = PromoteBishop | Capture,
		PromoteRookCapture = PromoteRook | Capture,
		PromoteQueenCapture = PromoteQueen | Capture
	};

	inline constexpr Square from_square(Move m) { return Square(m & 0b111111); }
	inline constexpr Square to_square(Move m) { return Square((m >> 6) & 0b111111); }
	inline constexpr MoveFlag move_flag(Move m) { return MoveFlag((m >> 12) & 0b1111); }
}