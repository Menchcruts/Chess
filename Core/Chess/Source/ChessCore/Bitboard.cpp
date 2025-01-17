#include "Bitboard.h"

namespace Chess
{    
    bool Bitboard::IsOccupied( int Square ) const
    {
        return 1ULL << Square & m_Bitboard;
    }

    void Bitboard::RemoveBit( int Square )
    {
        m_Bitboard ^= 1ULL << Square;
    }

    void Bitboard::AddBit( int Square )
    {
        m_Bitboard |= 1ULL << Square;
    }


    void Bitboards::AddBit( int Square, ChessPiece piece )
    {
        const bool isWhite = piece.color == Color::White;
        switch ( piece.type )
        {
        case PieceType::King:
            isWhite ? KingWhite.AddBit( Square ) : KingBlack.AddBit( Square );
            break;
        case PieceType::Pawn:
            isWhite ? PawnWhite.AddBit( Square ) : PawnBlack.AddBit( Square );
            break;
        case PieceType::Knight:
            isWhite ? KnightWhite.AddBit( Square ) : KnightBlack.AddBit( Square );
            break;
        case PieceType::Bishop:
            isWhite ? BishopWhite.AddBit( Square ) : BishopBlack.AddBit( Square );
            break;
        case PieceType::Rook:
            isWhite ? RookWhite.AddBit( Square ) : RookBlack.AddBit( Square );
            break;
        case PieceType::Queen:
            isWhite ? QueenWhite.AddBit( Square ) : QueenBlack.AddBit( Square );
            break;
        case PieceType::None:
        default:
            break;
        }
    }

    void Bitboards::RemoveBit( int Square, ChessPiece piece )
    {
        const bool isWhite = piece.color == Color::White;
        switch ( piece.type )
        {
        case PieceType::King:
            isWhite ? KingWhite.RemoveBit( Square ) : KingBlack.RemoveBit( Square );
            break;
        case PieceType::Pawn:
            isWhite ? PawnWhite.RemoveBit( Square ) : PawnBlack.RemoveBit( Square );
            break;
        case PieceType::Knight:
            isWhite ? KnightWhite.RemoveBit( Square ) : KnightBlack.RemoveBit( Square );
            break;
        case PieceType::Bishop:
            isWhite ? BishopWhite.RemoveBit( Square ) : BishopBlack.RemoveBit( Square );
            break;
        case PieceType::Rook:
            isWhite ? RookWhite.RemoveBit( Square ) : RookBlack.RemoveBit( Square );
            break;
        case PieceType::Queen:
            isWhite ? QueenWhite.RemoveBit( Square ) : QueenBlack.RemoveBit( Square );
            break;
        case PieceType::None:
        default:
            break;
        }
    }
}
