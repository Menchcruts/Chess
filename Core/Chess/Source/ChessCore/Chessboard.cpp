#include "Chessboard.h"
#include <iostream>


static const Chess::PieceType GetPieceFromRepr( char Piece )
{
    switch ( Piece )
    {
    case 'K':
    case 'k':
        return Chess::PieceType::King;
    case 'P':
    case 'p':
        return Chess::PieceType::Pawn;
    case 'N':
    case 'n':
        return Chess::PieceType::Knight;
    case 'B':
    case 'b':
        return Chess::PieceType::Bishop;
    case 'R':
    case 'r':
        return Chess::PieceType::Rook;
    case 'Q':
    case 'q':
        return Chess::PieceType::Queen;
    default:
        return Chess::PieceType::None;
    }
}


namespace Chess
{
    CastlingRights operator |( CastlingRights a, CastlingRights b )
    {
        return static_cast<CastlingRights>(static_cast<int>(a) | static_cast<int>(b));
    }
    CastlingRights operator &( CastlingRights a, CastlingRights b )
    {
        return static_cast<CastlingRights>(static_cast<int>(a) & static_cast<int>(b));
    }
    CastlingRights operator^( CastlingRights a, CastlingRights b )
    {
        return static_cast<CastlingRights>(static_cast<int>(a) ^ static_cast<int>(b));
    }
    CastlingRights& operator |=( CastlingRights& a, CastlingRights b )
    {
        a = a | b;
        return a;
    }
    CastlingRights& operator &=( CastlingRights& a, CastlingRights b )
    {
        a = a & b;
        return a;
    }
    CastlingRights& operator ^=( CastlingRights& a, CastlingRights b )
    {
        a = a ^ b;
        return a;
    }

    Chessboard::Chessboard()
    {
        m_MoveHistory.reserve( 96 );
        m_LegalMoves.reserve( 218 );    // Current maximum for number of legal moves for one position

        m_PieceHistory.reserve( 32 );
        m_InfoHistory.reserve( 96 );

        LoadFEN( "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" );
    }


    ChessPiece& Chessboard::AddPiece( int Square, Color color, PieceType type )
    {
        ChessPiece Piece( color, type );
        m_Bitboards.AddBit( Square, Piece );
        m_Board[Square] = Piece;

        return m_Board[Square];
    }

    void Chessboard::RemovePiece( int Square )
    {
        ChessPiece* Piece = &m_Board[Square];
        m_Bitboards.RemoveBit( Square, *Piece );
        Piece->MakeNullPiece();
    }

    void Chessboard::MovePiece( int Start, int Target )
    {
        m_Board[Target] = m_Board[Start];
        m_Board[Start].MakeNullPiece();
    }

    void Chessboard::GenerateMoves()
    {
        m_LegalMoves.clear();   // Delete old moves
    }

    Chessboard::BoardInfo Chessboard::GetBoardInfo() const
    {
        return { m_LegalMoves, m_Bitboards, m_Board, m_WhiteToPlay, m_EnPassantSquare, m_FullmoveClock, m_HalfmoveClock, m_WhiteCastling, m_BlackCastling };
    }

    void Chessboard::LoadFEN( const std::string& FEN_Pos )
    {
        // Set variables to default values
        m_Board.fill( { Color::None, PieceType::None } );   // Empty the board

        m_WhiteCastling = CastlingRights::None;             // Set castling rights
        m_BlackCastling = CastlingRights::None;             // for both sides to none

        m_EnPassantSquare = -1;
        m_FullmoveClock = 1;
        

        // Setup
        int i = (int)FEN_Pos.size();

        std::string TempString;

        // Fullmove
        while ( --i >= 0 )
        {
            char Letter = FEN_Pos[i];
            if ( Letter == ' ' )
                break;

            TempString.push_back( Letter );
        }

        try
        {
            int num = std::stoi( TempString );
            m_FullmoveClock = num;
        }
        catch ( const std::exception& )
        {
            std::cout << "Error loading m_FullmoveClock. Value '" << TempString << "' was not a number.\n";
        }
        TempString.clear();

        // Halfmove
        while ( --i >= 0 )
        {
            char Letter = FEN_Pos[i];
            if ( Letter == ' ' )
                break;
            TempString.push_back( Letter );
        }
        try
        {
            int num = std::stoi( TempString );
            m_HalfmoveClock = num;
        }
        catch ( const std::exception& )
        {
            std::cout << "Error loading m_HalfmoveClock. Value '" << TempString << "' was not a number.\n";
            m_HalfmoveClock = 0;
        }
        TempString.clear();

        // En passant
        short Rank = -1;
        short File = -1;
        while ( --i >= 0 )
        {
            char Letter = FEN_Pos[i];
            if ( Letter == ' ' )
                break;
            else if ( Letter == '-' )
            {
                m_EnPassantSquare = -1;
                continue;
            }
            else
            {
                if ( Rank == -1 )
                    Rank = (Letter - '0') - 1;
                else
                    File = (Letter - 'a');
            }
        }
        if ( Rank != -1 && File != -1 )
            m_EnPassantSquare = Rank * 8 + File;


        // Castling
        while ( --i >= 0 )
        {
            char Letter = FEN_Pos[i];
            if ( Letter == ' ' )
                break;

            switch ( Letter )
            {
            case 'K':
                m_WhiteCastling |= CastlingRights::Kingside;
                break;
            case 'k':
                m_BlackCastling |= CastlingRights::Kingside;
                break;
            case 'Q':
                m_WhiteCastling |= CastlingRights::QueenSide;
                break;
            case 'q':
                m_BlackCastling |= CastlingRights::QueenSide;
                break;
            default:
                break;
            }
        }

        // Side to play
        while ( --i >= 0 )
        {
            char Letter = FEN_Pos[i];
            if ( Letter == ' ' )
                break;
            else if ( Letter == 'w' )
                m_WhiteToPlay = true;
            else
                m_WhiteToPlay = false;
        }

        // Position
        int CurrentRank = 0;    // Last rank
        int CurrentSq = 7;     // Last square
        while ( --i >= 0 )
        {
            char Letter = FEN_Pos[i];
            if ( Letter == ' ' )
                break;
            else if ( Letter == '/' )
            {
                ++CurrentRank;
                CurrentSq = CurrentRank * 8 + 7;
                continue;
            }
            else if ( '0' <= Letter && Letter <= '9' )  // Letter is number
            {
                CurrentSq -= static_cast<int>(Letter - '0');
                continue;
            }
            else if ( 'A' <= Letter && Letter <= 'Z' )  // Letter is upper case (white)
            {
                AddPiece( CurrentSq, Color::White, GetPieceFromRepr( Letter ) );
            }
            else if ( 'a' <= Letter && Letter <= 'z' )  // Letter is lower case (black)
            {
                AddPiece( CurrentSq, Color::Black, GetPieceFromRepr( Letter ) );
            }
            --CurrentSq;
        }

        std::cout << "Engine has finished loading FEN " << FEN_Pos << "\n";
    }

    void Chessboard::MakeMove( Move move )
    {
        int Start = move.Start(), Target = move.Target();
        MoveFlag Flag = move.Flag();

        int CaptureTarget = Target;
        if ( Flag == MoveFlag::EnPassant )
            CaptureTarget = m_WhiteToPlay ? Target - 8 : Target + 8;

        ChessPiece PieceMoved = m_Board[Start];
        ChessPiece PieceCaptured = m_Board[CaptureTarget];
        
        PieceType Type = PieceMoved.type;
        CastlingRights& Rights = m_WhiteToPlay ? m_WhiteCastling : m_BlackCastling;

        m_InfoHistory.emplace_back( m_EnPassantSquare, m_HalfmoveClock, Rights );

        m_EnPassantSquare = -1;

        bool Capture = ((Flag & MoveFlag::Capture) == MoveFlag::Capture) || !PieceCaptured.IsNullPiece();
        bool ResetHalfclock = Type == PieceType::Pawn || Capture;

        if ( Type == PieceType::King )
        {
            if ( Start == (m_WhiteToPlay ? 4 : 60) && ((Rights & CastlingRights::Both) != CastlingRights::None) )
                Rights = CastlingRights::None;
        }
        else if ( Type == PieceType::Rook )
        {
            if ( (Rights & CastlingRights::Kingside) != CastlingRights::None && (Start == 7 || Start == 63) )
                Rights ^= CastlingRights::Kingside;

            else if ( (Rights & CastlingRights::QueenSide) != CastlingRights::None && (Start == 0 || Start == 56) )
                Rights ^= CastlingRights::QueenSide;
        }

        RemovePiece( Start );               // Remove piece from start square

        if ( Capture )
        {
            RemovePiece( CaptureTarget );          // Remove captured piece
            m_PieceHistory.push_back( PieceCaptured );
        }

        m_Board[Target] = PieceMoved;       // Move piece to new square


        if ( Flag == MoveFlag::CastleKing || Flag == MoveFlag::CastleQueen )
        {
            int RookStart, RookTarget;
            if ( Flag == MoveFlag::CastleKing )
            {
                RookStart = m_WhiteToPlay ? 7 : 63;
                RookTarget = m_WhiteToPlay ? 5 : 61;
            }
            else
            {
                RookStart = m_WhiteToPlay ? 0 : 56;
                RookTarget = m_WhiteToPlay ? 3 : 59;
            }
            ChessPiece Rook = m_Board[RookStart];
            m_Board[RookStart].MakeNullPiece();
            m_Board[RookTarget] = Rook;
        }
        else if ( Flag == MoveFlag::DoublePawnMove )
        {
            m_EnPassantSquare = m_WhiteToPlay ? Start + 8 : Start - 8;
        }

        if ( ResetHalfclock )
            m_HalfmoveClock = 0;
        else
            ++m_HalfmoveClock;

        if ( !m_WhiteToPlay )
            ++m_FullmoveClock;

        m_WhiteToPlay = !m_WhiteToPlay;

        m_MoveHistory.push_back( move );    // Add move to the move history
    }

    void Chessboard::UnMakeMove()
    {
        if ( m_MoveHistory.size() <= 0 )
            return;
        
        m_WhiteToPlay = !m_WhiteToPlay;

        Move move = m_MoveHistory.back();
        m_MoveHistory.pop_back();
        
        int Start = move.Start(), Target = move.Target();
        MoveFlag Flag = move.Flag();

        int CaptureTarget = Target;
        if ( Flag == MoveFlag::EnPassant )
            CaptureTarget = m_WhiteToPlay ? Target - 8: Target + 8;

        CastlingRights& CurrentRights = m_WhiteToPlay ? m_WhiteCastling : m_BlackCastling;

        if ( m_InfoHistory.size() > 0 )
        {
            SpecialInfo info = m_InfoHistory.back();
            m_InfoHistory.pop_back();

            m_EnPassantSquare = info.EnPassant;
            m_HalfmoveClock = info.Halfmove;
            CurrentRights = info.Rights;
        }
        else
        {
            m_EnPassantSquare = -1;
            m_HalfmoveClock = 0;
        }

        ChessPiece PieceMoved = m_Board[Target], PieceCaptured;

        if ( (Flag & MoveFlag::Capture) == MoveFlag::Capture )
        {
            if ( m_PieceHistory.size() > 0 )
            {
                PieceCaptured = m_PieceHistory.back();
                m_PieceHistory.pop_back();
            }
        }
        else if ( Flag == MoveFlag::CastleKing || Flag == MoveFlag::CastleQueen )
        {
            int RookStart, RookTarget;
            if ( Flag == MoveFlag::CastleKing )
            {
                RookStart = m_WhiteToPlay ? 7 : 63;
                RookTarget = m_WhiteToPlay ? 5 : 61;
            }
            else
            {
                RookStart = m_WhiteToPlay ? 0 : 56;
                RookTarget = m_WhiteToPlay ? 3 : 59;
            }
            ChessPiece Rook = m_Board[RookTarget];
            m_Board[RookTarget].MakeNullPiece();
            m_Board[RookStart] = Rook;
        }

        RemovePiece( Target );
        AddPiece( Start, PieceMoved.color, PieceMoved.type );

        if ( !PieceCaptured.IsNullPiece() )
        {
            AddPiece( CaptureTarget, PieceCaptured.color, PieceCaptured.type );
        }

        if ( !m_WhiteToPlay )
            --m_FullmoveClock;
    }

}