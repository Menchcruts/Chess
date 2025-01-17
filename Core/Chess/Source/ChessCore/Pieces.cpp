#include "Pieces.h"

namespace Chess
{
	ChessPiece::ChessPiece( Chess::Color color, Chess::PieceType type )
		: color( color ), type( type ) { }

	bool ChessPiece::IsNullPiece() const
	{
		return type == PieceType::None || color == Color::None;
	}

	void ChessPiece::MakeNullPiece()
	{
		color = Color::None;
		type = PieceType::None;
	}

	std::string ChessPiece::GetPieceRepr() const
	{
        switch ( type )
        {
        case Chess::PieceType::King:
            return "King";
        case Chess::PieceType::Pawn:
            return "Pawn";
        case Chess::PieceType::Knight:
            return "Knight";
        case Chess::PieceType::Bishop:
            return "Bishop";
        case Chess::PieceType::Rook:
            return "Rook";
        case Chess::PieceType::Queen:
            return "Queen";
        case Chess::PieceType::None:
        default:
            return "None";
        }
	}
}