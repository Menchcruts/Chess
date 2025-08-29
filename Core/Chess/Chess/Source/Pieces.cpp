#include "Pieces.h"

namespace Chess_Old
{
	ChessPiece::ChessPiece( Chess_Old::Color color, Chess_Old::PieceType type )
		: color( color ), type( type ) { }

	bool ChessPiece::IsNullPiece() const
	{
		return type == PieceType::None || color == Color::None;
	}

    bool ChessPiece::IsSlidingPiece() const
    {
        return type == PieceType::Queen || type == PieceType::Rook || type == PieceType::Bishop;
    }

    bool ChessPiece::IsDiagonalMoving() const
    {
        return type == PieceType::Queen || type == PieceType::Bishop;
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
        case Chess_Old::PieceType::King:
            return "King";
        case Chess_Old::PieceType::Pawn:
            return "Pawn";
        case Chess_Old::PieceType::Knight:
            return "Knight";
        case Chess_Old::PieceType::Bishop:
            return "Bishop";
        case Chess_Old::PieceType::Rook:
            return "Rook";
        case Chess_Old::PieceType::Queen:
            return "Queen";
        case Chess_Old::PieceType::None:
        default:
            return "None";
        }
	}
}