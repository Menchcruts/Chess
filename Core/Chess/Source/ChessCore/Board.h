#pragma once
#include "Square.h"
#include "Pieces/Pieces.h"
#include <array>
#include <memory>

namespace Chess
{
	struct Board
	{
		std::array<Piece, 64> _Board{ };

		Square GetSquare(int Square);

		void Reset();

		Piece& operator []( std::size_t index );
		const Piece& operator []( std::size_t index ) const;
	};
}