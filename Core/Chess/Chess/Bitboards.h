#pragma once
#include <bit>

#include "types.h"

#include <cmath>
#include <cassert>
#include <algorithm>
#include <unordered_map>
#include <vector>

namespace Chess::Bitboards
{
	void init();
	
	extern Bitboard BetweenBB[64][64];
	extern Bitboard LineBB[64][64];
	extern std::uint8_t SquareDistances[64][64];

	extern Bitboard KingAttacks[64];
	extern Bitboard KnightAttacks[64];

	extern Bitboard BishopAttacks[64];
	extern Bitboard RookAttacks[64];
	extern Bitboard QueenAttacks[64];

	extern Bitboard RookBlockers[64];
	extern Bitboard BishopBlockers[64];

	struct Magic
	{
		std::unordered_map<int, Bitboard> attacks;
		Bitboard magicIndex = 0;
		Bitboard blockerMask = 0;
		int shift = 0;

		Magic()
		{
			attacks.reserve(16384); // Reserve space for 16384 entries
		}

		Magic(Bitboard Magic, int Shift, Bitboard BlockerMask)
			: magicIndex(Magic), shift(Shift), blockerMask(BlockerMask)
		{
			attacks.reserve(16384);
		}

		constexpr Bitboard filter_blockers(Bitboard blockers) const
		{
			return blockers & blockerMask;
		}

		constexpr int index(Square sq, Bitboard blockers) const
		{
			return (blockers * magicIndex) >> shift;
		}

		Bitboard get_attacks(Square sq, Bitboard blockers) const
		{
			return attacks.at(index(sq, filter_blockers(blockers)));
		}
	};

	extern Magic Magics[64][2];

	constexpr Bitboard FileA_BB = 0x0101010101010101ULL;
	constexpr Bitboard FileB_BB = FileA_BB << 1;
	constexpr Bitboard FileC_BB = FileA_BB << 2;
	constexpr Bitboard FileD_BB = FileA_BB << 3;
	constexpr Bitboard FileE_BB = FileA_BB << 4;
	constexpr Bitboard FileF_BB = FileA_BB << 5;
	constexpr Bitboard FileG_BB = FileA_BB << 6;
	constexpr Bitboard FileH_BB = FileA_BB << 7;

	constexpr Bitboard Rank1_BB = 0xFF;
	constexpr Bitboard Rank2_BB = Rank1_BB << (8 * 1);
	constexpr Bitboard Rank3_BB = Rank1_BB << (8 * 2);
	constexpr Bitboard Rank4_BB = Rank1_BB << (8 * 3);
	constexpr Bitboard Rank5_BB = Rank1_BB << (8 * 4);
	constexpr Bitboard Rank6_BB = Rank1_BB << (8 * 5);
	constexpr Bitboard Rank7_BB = Rank1_BB << (8 * 6);
	constexpr Bitboard Rank8_BB = Rank1_BB << (8 * 7);

	constexpr Bitboard rank_bb( Rank r ) { return Rank1_BB << (8 * int(r)); }
	constexpr Bitboard rank_bb( Square sq ) { return rank_bb( rank_of( sq ) ); }

	constexpr Bitboard file_bb( File f ) { return FileA_BB << int( f ); }
	constexpr Bitboard file_bb( Square sq ) { return file_bb( file_of( sq ) ); }

	template<Direction Dir>
	constexpr Bitboard shift( Bitboard bb )
	{
		switch ( Dir )
		{
		case Dir_North:
			return bb << 8;
		case Dir_North + Dir_North:
			return bb << 16;
		
		case Dir_South:
			return bb >> 8;
		case Dir_South + Dir_South:
			return bb >> 16;

		case Dir_East:
			return (bb & ~FileH_BB) << 1;
		case Dir_West:
			return (bb & ~FileA_BB) >> 1;

		case Dir_NorthEast:
			return (bb & ~FileH_BB) << 9;
		case Dir_NorthWest:
			return (bb & ~FileA_BB) << 7;

		case Dir_SouthEast:
			return (bb & ~FileH_BB) >> 7;
		case Dir_SouthWest:
			return (bb & ~FileA_BB) >> 9;

		default:
			return 0;
		}
	}
	constexpr Bitboard shift( Bitboard bb, Direction Dir )
	{
		switch ( Dir )
		{
		case Dir_North:
			return bb << 8;
		case Dir_North + Dir_North:
			return bb << 16;

		case Dir_South:
			return bb >> 8;
		case Dir_South + Dir_South:
			return bb >> 16;

		case Dir_East:
			return (bb & ~FileH_BB) << 1;
		case Dir_West:
			return (bb & ~FileA_BB) >> 1;

		case Dir_NorthEast:
			return (bb & ~FileH_BB) << 9;
		case Dir_NorthWest:
			return (bb & ~FileA_BB) << 7;

		case Dir_SouthEast:
			return (bb & ~FileH_BB) >> 7;
		case Dir_SouthWest:
			return (bb & ~FileA_BB) >> 9;

		default:
			return 0;
		}
	}

	constexpr Bitboard from_sq( Square sq ) { return is_ok(sq) ? Bitboard( 1ULL << sq ) : 0; }

	constexpr bool is_occupied(Bitboard bb, Square sq)
	{
		return is_ok(sq) && bb & from_sq(sq);
	}

	template<Color C>
	constexpr Bitboard pawn_attacks_bb( Bitboard bb )
	{
		return C == White ? shift<Dir_NorthWest>( bb ) | shift<Dir_NorthEast>( bb )
						  : shift<Dir_SouthWest>( bb ) | shift<Dir_SouthEast>( bb );
	}

	constexpr int Squares_to_Edge( Square sq, Direction dir )
	{
		if (!is_ok(sq))
			return -1;
		
		Rank Current_Rank = rank_of( sq );
		File Current_File = file_of( sq );

		switch ( dir )
		{
		case Chess::Dir_North:
			return 7 - Current_Rank;

		case Chess::Dir_East:
			return 7 - Current_File;

		case Chess::Dir_South:
			return Current_Rank;

		case Chess::Dir_West:
			return Current_File;

		case Chess::Dir_NorthEast:
			return std::min( Squares_to_Edge( sq, Dir_North ), Squares_to_Edge( sq, Dir_East ) );

		case Chess::Dir_NorthWest:
			return std::min( Squares_to_Edge( sq, Dir_North ), Squares_to_Edge( sq, Dir_West ) );

		case Chess::Dir_SouthEast:
			return std::min( Squares_to_Edge( sq, Dir_South ), Squares_to_Edge( sq, Dir_East ) );

		case Chess::Dir_SouthWest:
			return std::min( Squares_to_Edge( sq, Dir_South ), Squares_to_Edge( sq, Dir_West ) );

		default:
			return -1;
		}
	}

	constexpr Bitboard get_line_bb( Square sq1, Square sq2 )
	{
		return LineBB[sq1][sq2];
	}
	constexpr Bitboard get_between_bb( Square sq1, Square sq2 )
	{
		return BetweenBB[sq1][sq2];
	}

	constexpr int popcount( Bitboard bb ) { return std::popcount( bb ); }
	constexpr int lsb( Bitboard bb ) { return std::countr_zero( bb ); }
	constexpr int msb( Bitboard bb ) { return std::countl_zero( bb ); }

	constexpr Square bitscan_forward(Bitboard bb) { return Square(lsb(bb)); }
	constexpr Square bitscan_forward_auto(Bitboard& bb) { int bit = lsb(bb); bb &= bb - 1; return Square(bit); }

	template<typename _Ty = Square>
	inline int distance( Square x, Square y );

	template<>
	inline int distance<File>( Square x, Square y )
	{
		return std::abs( file_of( x ) - file_of( y ) );
	}
	
	template<>
	inline int distance<Rank>( Square x, Square y )
	{
		return std::abs( rank_of( x ) - rank_of( y ) );
	}

	template<>
	inline int distance<Square>( Square x, Square y )
	{
		return SquareDistances[x][y];
	}

	inline Bitboard attacks( Square sq, PieceType pt )
	{
		assert(pt == King || pt == Knight);
		if (!is_ok(sq))
			return 0;

		return (pt == King)   ? KingAttacks[sq] : 
			   (pt == Knight) ? KnightAttacks[sq] : 
								0;
	}
	inline Bitboard attacks( Square sq, Color c )
	{
		assert( c == White || c == Black );
		return c == White ? pawn_attacks_bb<White>( from_sq( sq ) )
						  : pawn_attacks_bb<Black>( from_sq( sq ) );
	}
	inline Bitboard attacks( Square sq, PieceType pt, Bitboard blockers )
	{
		if (!(pt == Bishop || pt == Rook || pt == Queen))
			return attacks(sq, pt);
		//assert(pt == Bishop || pt == Rook || pt == Queen);
		if (!is_ok(sq))
			return 0;

		switch ( pt )
		{
		case Chess::Bishop:
		case Chess::Rook:
			return Magics[sq][pt - Bishop].get_attacks( sq, blockers);
		case Chess::Queen:
			return attacks( sq, Rook, blockers ) | attacks( sq, Bishop, blockers );
		default:
			return 0;
		}
	}
	inline Bitboard attacks(Square sq, PieceType pt, Bitboard blockers, Color c) 
	{
		return pt == Pawn ? attacks(sq, c) : attacks(sq, pt, blockers);
	}
}