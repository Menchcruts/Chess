#pragma once
#include <cstdint>
#include "Pieces.h"

namespace Chess
{
	struct Bitboard
	{
		std::uint64_t m_Bitboard = 0;

		constexpr bool IsOccupied( int Square ) const
		{
			return 1ULL << Square & m_Bitboard;
		}
		constexpr void RemoveBit( int Square )
		{
			m_Bitboard ^= 1ULL << Square;
		}
		constexpr void AddBit( int Square )
		{
			m_Bitboard |= 1ULL << Square;
		}

		constexpr Bitboard() = default;
		constexpr Bitboard( std::uint64_t value )
			: m_Bitboard(value) { }

		constexpr inline operator bool() const
		{
			return m_Bitboard != 0;
		}
		constexpr inline bool operator ==( Bitboard other )
		{
			return this->m_Bitboard == other.m_Bitboard;
		}
		constexpr inline bool operator !=( Bitboard other )
		{
			return !(*this == other);
		}

		constexpr inline Bitboard& operator |=( Bitboard other )
		{
			m_Bitboard |= other.m_Bitboard;
			return *this;
		}
		constexpr inline friend Bitboard operator | ( Bitboard left, const Bitboard& right )
		{
			left |= right;
			return left;
		}
		
		constexpr inline Bitboard& operator &=( Bitboard other )
		{
			m_Bitboard &= other.m_Bitboard;
			return *this;
		}
		constexpr inline friend Bitboard operator & ( Bitboard left, const Bitboard& right )
		{
			left &= right;
			return left;
		}
		
		constexpr inline Bitboard& operator ^=( Bitboard other )
		{
			m_Bitboard ^= other.m_Bitboard;
			return *this;
		}
		constexpr inline friend Bitboard operator ^ ( Bitboard left, const Bitboard& right )
		{
			left ^= right;
			return left;
		}
		
		constexpr inline Bitboard operator~() const
		{
			Bitboard result;
			result.m_Bitboard = ~m_Bitboard;
			return result;
		}
		
		constexpr inline Bitboard& operator<<=( unsigned int shift )
		{
			m_Bitboard <<= shift;
			return *this;
		}
		constexpr inline friend Bitboard operator<<( Bitboard lhs, unsigned int shift )
		{
			lhs <<= shift;
			return lhs;
		}
		
		constexpr inline Bitboard& operator>>=( unsigned int shift )
		{
			m_Bitboard >>= shift;
			return *this;
		}
		constexpr inline friend Bitboard operator>>( Bitboard lhs, unsigned int shift )
		{
			lhs >>= shift;
			return lhs;
		}
	};


	struct Bitboards
	{
		Bitboard KingWhite = 0;
		Bitboard KingBlack = 0;

		Bitboard PawnWhite = 0;
		Bitboard PawnBlack = 0;

		Bitboard KnightWhite = 0;
		Bitboard KnightBlack = 0;

		Bitboard BishopWhite = 0;
		Bitboard BishopBlack = 0;

		Bitboard RookWhite = 0;
		Bitboard RookBlack = 0;

		Bitboard QueenWhite = 0;
		Bitboard QueenBlack = 0;

		Bitboards() = default;

		void Reset();
		void AddBit( int Square, ChessPiece piece );
		void RemoveBit( int Square, ChessPiece piece );
		ChessPiece GetPieceAtSquare( int Square ) const;

		Bitboard GetColorMask( Color color ) const;
	};
}