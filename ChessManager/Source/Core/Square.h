#pragma once

#include "Chess.h"

struct Square
{
	Chess::Pieces::Piece Piece;
	int Sq = -1;

	Square();
	Square( Chess::Color color, Chess::PieceType type, int square = -1 );
};