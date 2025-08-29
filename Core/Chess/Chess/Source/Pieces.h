#pragma once
#include "ChessCore.h"
#include <string>

namespace Chess_Old
{
	struct ChessPiece
	{
		Color color = Color::None;
		PieceType type = PieceType::None;

		ChessPiece() = default;
		ChessPiece( Chess_Old::Color color, Chess_Old::PieceType type );

		bool IsNullPiece() const;
		bool IsSlidingPiece() const;
		bool IsDiagonalMoving() const;

		void MakeNullPiece();
		std::string GetPieceRepr() const;
	};
}