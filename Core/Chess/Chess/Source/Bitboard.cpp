#include "Bitboard.h"

namespace Chess_Old
{
    void Bitboards::Reset()
    {
        KingWhite = 0;
        KingBlack = 0;

        PawnWhite = 0;
        PawnBlack = 0;

        KnightWhite = 0;
        KnightBlack = 0;

        BishopWhite = 0;
        BishopBlack = 0;

        RookWhite = 0;
        RookBlack = 0;

        QueenWhite = 0;
        QueenBlack = 0;
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
    ChessPiece Bitboards::GetPieceAtSquare( int Square ) const
    {
        if ( KingWhite.IsOccupied(Square) )
            return ChessPiece( Color::White, PieceType::King );
        else if ( KingBlack.IsOccupied( Square ) )
            return ChessPiece( Color::Black, PieceType::King );

        else if ( PawnWhite.IsOccupied( Square ) )
            return ChessPiece( Color::White, PieceType::Pawn );
        else if ( PawnBlack.IsOccupied( Square ) )
            return ChessPiece( Color::Black, PieceType::Pawn );

        else if ( KnightWhite.IsOccupied( Square ) )
            return ChessPiece( Color::White, PieceType::Knight );
        else if ( KnightBlack.IsOccupied( Square ) )
            return ChessPiece( Color::Black, PieceType::Knight );

        else if ( BishopWhite.IsOccupied( Square ) )
            return ChessPiece( Color::White, PieceType::Bishop );
        else if ( BishopBlack.IsOccupied( Square ) )
            return ChessPiece( Color::Black, PieceType::Bishop );

        else if ( RookWhite.IsOccupied( Square ) )
            return ChessPiece( Color::White, PieceType::Rook );
        else if ( RookBlack.IsOccupied( Square ) )
            return ChessPiece( Color::Black, PieceType::Rook );

        else if ( QueenWhite.IsOccupied( Square ) )
            return ChessPiece( Color::White, PieceType::Queen );
        else if ( QueenBlack.IsOccupied( Square ) )
            return ChessPiece( Color::Black, PieceType::Queen );

        return ChessPiece();    // No piece on the square
    }
    Bitboard Bitboards::GetColorMask( Color color ) const
    {
        if ( color == Color::White )
            return KingWhite | PawnWhite | KnightWhite | BishopWhite | RookWhite | QueenWhite;
        else if ( color == Color::Black )
            return KingBlack | PawnBlack | KnightBlack | BishopBlack | RookBlack | QueenBlack;
        else
            return Bitboard();
    }
    Bitboard Bitboards::AllPieces() const
    {
        return KingWhite | PawnWhite | KnightWhite | BishopWhite | RookWhite | QueenWhite | KingBlack | PawnBlack | KnightBlack | BishopBlack | RookBlack | QueenBlack;
    }
}