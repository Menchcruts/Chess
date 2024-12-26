#pragma once
#include <stdint.h>

struct Bitboard
{
	uint64_t m_Bitboard;

	bool IsOccupied( int Square ) const;
	void RemoveBit( int Square );
	void AddBit( int Square );
};