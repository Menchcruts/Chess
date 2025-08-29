#pragma once
#include <cstdint>
#include "Pieces.h"

namespace Chess_Old
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

		constexpr operator bool() const
		{
			return m_Bitboard != 0;
		}
		constexpr bool operator ==( Bitboard other )
		{
			return this->m_Bitboard == other.m_Bitboard;
		}
		constexpr bool operator !=( Bitboard other )
		{
			return !(*this == other);
		}
 
		inline constexpr Bitboard& operator |=( Bitboard other )
		{
			m_Bitboard |= other.m_Bitboard;
			return *this;
		}
		constexpr friend Bitboard operator | ( Bitboard left, const Bitboard& right )
		{
			left |= right;
			return left;
		}
 		
		constexpr Bitboard& operator &=( Bitboard other )
		{
			m_Bitboard &= other.m_Bitboard;
			return *this;
		}
		constexpr friend Bitboard operator & ( Bitboard left, const Bitboard& right )
		{
			left &= right;
			return left;
		}
 		
		constexpr Bitboard& operator ^=( Bitboard other )
		{
			m_Bitboard ^= other.m_Bitboard;
			return *this;
		}
		constexpr friend Bitboard operator ^ ( Bitboard left, const Bitboard& right )
		{
			left ^= right;
			return left;
		}
 		
		constexpr Bitboard operator~() const
		{
			Bitboard result;
			result.m_Bitboard = ~m_Bitboard;
			return result;
		}
 		
		constexpr Bitboard& operator<<=( int shift )
		{
			m_Bitboard <<= shift;
			return *this;
		}
		constexpr friend Bitboard operator<<( Bitboard lhs, int shift )
		{
			lhs <<= shift;
			return lhs;
		}
		constexpr Bitboard& operator<<=( unsigned int shift )
		{
			m_Bitboard <<= shift;
			return *this;
		}
		constexpr friend Bitboard operator<<( Bitboard lhs, unsigned int shift )
		{
			lhs <<= shift;
			return lhs;
		}
		constexpr Bitboard& operator<<=( short shift )
		{
			m_Bitboard <<= shift;
			return *this;
		}
		constexpr friend Bitboard operator<<( Bitboard lhs, short shift )
		{
			lhs <<= shift;
			return lhs;
		}

		constexpr Bitboard& operator>>=( int shift )
		{
			m_Bitboard >>= shift;
			return *this;
		}
		constexpr friend Bitboard operator>>( Bitboard lhs, int shift )
		{
			lhs >>= shift;
			return lhs;
		}
		constexpr Bitboard& operator>>=( unsigned int shift )
		{
			m_Bitboard >>= shift;
			return *this;
		}
		constexpr friend Bitboard operator>>( Bitboard lhs, unsigned int shift )
		{
			lhs >>= shift;
			return lhs;
		}
		constexpr Bitboard& operator>>=( short shift )
		{
			m_Bitboard >>= shift;
			return *this;
		}
		constexpr friend Bitboard operator>>( Bitboard lhs, short shift )
		{
			lhs >>= shift;
			return lhs;
		}

		constexpr Bitboard& operator -=( int Num )
		{
			m_Bitboard -= Num;
			return *this;
		}
		constexpr friend Bitboard operator -( Bitboard lhs, Bitboard rhs )
		{
			lhs.m_Bitboard -= rhs.m_Bitboard;
			return lhs;
		}
		constexpr friend Bitboard operator -( Bitboard lhs, int Num )
		{
			lhs.m_Bitboard -= Num;
			return lhs;
		}


		constexpr int BitscanForward() const
		{
			if ( m_Bitboard == 0 )
				return -1;

			Bitboard Bit = 1;
			int position = 0;
			while ( !((*this) & Bit) )
			{
				Bit <<= 1;
				++position;
			}
			return position;
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

		Bitboard AllPieces() const;
	};
}