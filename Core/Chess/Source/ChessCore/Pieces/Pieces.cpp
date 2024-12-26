#include "Pieces.h"

namespace Chess
{
	Piece::Piece( Chess::Color color, Chess::PieceType type )
		: color( color ), type( type ) { }

	bool Piece::IsNullPiece() const
	{
		return type == PieceType::None || color == Color::None;
	}
}
