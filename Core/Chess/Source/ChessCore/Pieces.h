#pragma once
#include "ChessCore.h"
#include <string>

namespace Chess
{
	struct ChessPiece
	{
		Color color = Color::None;
		PieceType type = PieceType::None;

		ChessPiece() = default;
		ChessPiece( Chess::Color color, Chess::PieceType type );

		bool IsNullPiece() const;
		void MakeNullPiece();
		std::string GetPieceRepr() const;
	};
}