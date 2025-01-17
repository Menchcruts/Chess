#pragma once
#include <cstdint>
#include "Pieces.h"

namespace Chess
{
	struct Bitboard
	{
		std::uint64_t m_Bitboard;

		bool IsOccupied( int Square ) const;
		void RemoveBit( int Square );
		void AddBit( int Square );
	};


	struct Bitboards
	{
		Bitboard KingWhite;
		Bitboard KingBlack;

		Bitboard PawnWhite;
		Bitboard PawnBlack;

		Bitboard KnightWhite;
		Bitboard KnightBlack;

		Bitboard BishopWhite;
		Bitboard BishopBlack;

		Bitboard RookWhite;
		Bitboard RookBlack;

		Bitboard QueenWhite;
		Bitboard QueenBlack;

		void AddBit( int Square, ChessPiece piece );
		void RemoveBit( int Square, ChessPiece piece );
	};
}