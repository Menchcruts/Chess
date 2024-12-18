#include "Square.h"

Square::Square() 
	: Piece( Chess::Color::White, Chess::PieceType::None ) {}

Square::Square( Chess::Color color, Chess::PieceType type, int square )
	: Piece( color, type ), Sq( square ) {}
