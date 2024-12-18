#pragma once
#include "../ChessCore.h"

namespace Chess::Pieces
{
	struct Piece
	{
		Chess::Color Color;
		Chess::PieceType Type;

		Piece();
		Piece( Chess::Color color, Chess::PieceType type );
	};
}