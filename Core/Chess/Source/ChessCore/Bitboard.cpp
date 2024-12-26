#include "Bitboard.h"

bool Bitboard::IsOccupied( int Square ) const
{
    return 1ULL << Square & m_Bitboard;
}

void Bitboard::RemoveBit( int Square )
{
    m_Bitboard ^= 1ULL << Square;
}

void Bitboard::AddBit( int Square )
{
    m_Bitboard |= 1ULL << Square;
}
