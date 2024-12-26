#include "Engine.h"
#include <iostream>
#include <vector>
#include <sstream>

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
    CastlingRights operator|( CastlingRights a, CastlingRights b )
    {
        return static_cast<CastlingRights>(static_cast<int>(a) | static_cast<int>(b));
    }
    CastlingRights operator&( CastlingRights a, CastlingRights b )
    {
        return static_cast<CastlingRights>(static_cast<int>(a) & static_cast<int>(b));
    }
    CastlingRights& operator|=( CastlingRights& a, CastlingRights b )
    {
        return a = a | b;
    }
    CastlingRights& operator&=( CastlingRights& a, CastlingRights b )
    {
        return a = a & b;
    }


    /*Engine::Engine()
    {
        LoadFEN( "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" );
    }*/

    const Board& Engine::GetBoard() const
    {
        return m_Board;
    }

    Engine::EngineInfo Engine::GetEngineInfo() const
    {
        return {
            m_WhiteToPlay,
            m_EnPassantSq,
            m_FullmoveClock,
            m_HalfmoveClock,
            m_WhiteCastling,
            m_BlackCastling
        };
    }

    void Engine::MakeMove( int Start, int Target )
    {
        Piece PieceMoved = m_Board[Start];
        Piece PieceCaptured = m_Board[Target];

        m_Board[Start].type = PieceType::None;
        m_Board[Target] = PieceMoved;
    }

    void Engine::LoadFEN( const std::string& FEN_Pos )
    {
        m_Board.Reset();
        m_WhiteCastling = CastlingRights::None;
        m_BlackCastling = CastlingRights::None;

        m_WhiteToPlay = true;
        m_EnPassantSq = -1;
        m_FullmoveClock = 0;
        m_HalfmoveClock = 0;

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
        }
        TempString.clear();

        // En passant
        int Rank = -1;
        int File = -1;
        while( --i >= 0 )
        {
            char Letter = FEN_Pos[i];
            if ( Letter == ' ' )
                break;
            else if ( Letter == '-' )
            {
                m_EnPassantSq = -1;
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
            m_EnPassantSq = Rank * 8 + File;


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
                m_Board[CurrentSq] = Piece( Color::White, GetPieceFromRepr( Letter ) );
            }
            else if ( 'a' <= Letter && Letter <= 'z' )  // Letter is lower case (black)
            {
                m_Board[CurrentSq] = Piece( Color::Black, GetPieceFromRepr( Letter ) );
            }
            --CurrentSq;
        }

        std::cout << "Engine has finished loading FEN " << FEN_Pos << "\n";
    }
}