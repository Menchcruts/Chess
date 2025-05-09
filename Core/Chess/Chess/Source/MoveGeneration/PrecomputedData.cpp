#include "PrecomputedData.h"
#include "Utils/Logger.h"
#include <unordered_map>

namespace Chess::MoveGen
{
    std::vector<Bitboard> CreateBlockerPerms( Bitboard BlockerMask )
	{
		std::vector<Bitboard> result;
		std::vector<short> SquareIdxs;
		SquareIdxs.reserve( 12 );

		for ( int i = 0; i < 64; ++i )
			if ( BlockerMask.IsOccupied( i ) )
				SquareIdxs.push_back( i );

		int NumPatterns = 1 << SquareIdxs.size();

		Bitboard Mask;
		Bitboard TempBit;

		for ( int pattern = 0; pattern < NumPatterns; ++pattern )
		{
			Mask = 0;
			TempBit = 0;
			for ( int i = 0; i < SquareIdxs.size(); ++i )
			{
				TempBit = (pattern >> i) & 1;
				Mask |= (TempBit << (unsigned int)SquareIdxs[i]);
			}
			result.push_back( Mask );
		}

		return result;
	}
	Bitboard MoveMaskFromBlocker( short Square, Bitboard BlockerMask, bool IsDiagonal )
	{
		Bitboard Result;

		int StartIdx = IsDiagonal ? 1 : 0;

		ChessCoord Center = ChessCoord( Square );
		ChessCoord NewCoord;
		short NewSquare;

		for ( int i = StartIdx; i < 8; i += 2 )
		{
			auto& dir = DirChanges[i];
			NewCoord = Center;

			for ( int _ = 0; _ < 8; ++_ )
			{
				NewCoord += dir;
				if ( !NewCoord.IsValid() )
					break;

				NewSquare = NewCoord.AsSquare();
				Result |= Bit << (unsigned int)NewSquare;

				if ( BlockerMask.IsOccupied( NewSquare ) )
					break;
			}
		}
		return Result;
	}

	std::array<std::array<Bitboard, 64>, 64>* CreateAlignMasks()
	{
		auto result = new std::array<std::array<Bitboard, 64>, 64>();
		return result;
	}
}