#include "Pieces.h"

Chess::Pieces::Piece::Piece() 
	: Color( Chess::Color::White ), Type( Chess::PieceType::None ) { }

Chess::Pieces::Piece::Piece( Chess::Color color, Chess::PieceType type )
	: Color( color ), Type( type ) { }
