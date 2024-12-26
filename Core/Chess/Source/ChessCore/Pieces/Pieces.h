#pragma once
#include "../ChessCore.h"

namespace Chess
{
	struct Piece
	{
		Color color = Color::None;
		PieceType type = PieceType::None;

		Piece() = default;
		Piece( Chess::Color color, Chess::PieceType type );

		bool IsNullPiece() const;
	};
}