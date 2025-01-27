#include "Chessboard.h"
#include "Stopwatch/Stopwatch.h"
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
    Chessboard::Chessboard()
    {
        m_MoveHistory.reserve( 96 );
        m_LegalMoves.reserve( 218 );    // Theoretical maximum for number of legal moves for one position

        m_PieceHistory.reserve( 32 );
        m_InfoHistory.reserve( 96 );

        m_WhitePositions.reserve( 64 );
        m_BlackPositions.reserve( 64 );

        LoadFEN( "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" );
    }

    Chessboard::Chessboard( const std::string& FEN_Pos )
    {
        m_MoveHistory.reserve( 96 );
        m_LegalMoves.reserve( 218 );    // Theoretical maximum for number of legal moves for one position

        m_PieceHistory.reserve( 32 );
        m_InfoHistory.reserve( 96 );

        m_WhitePositions.reserve( 64 );
        m_BlackPositions.reserve( 64 );

        LoadFEN( FEN_Pos );
    }

    void Chessboard::AddPiece( int Square, Color color, PieceType type )
    {
        ChessPiece Piece( color, type );
        m_Bitboards.AddBit( Square, Piece );

        bool IsWhite = color == Color::White;
        auto* Positions = IsWhite ? &m_WhitePositions : &m_BlackPositions;
        Positions->insert( Square );
    }

    void Chessboard::RemovePiece( int Square )
    {
        ChessPiece Piece = m_Bitboards.GetPieceAtSquare( Square );
        m_Bitboards.RemoveBit( Square, Piece );

        bool IsWhite = Piece.color == Color::White;
        auto* Positions = IsWhite ? &m_WhitePositions : &m_BlackPositions;
        Positions->erase( Square );
    }

    void Chessboard::MovePiece( int Start, int Target )
    {
        ChessPiece piece = m_Bitboards.GetPieceAtSquare( Start );
        AddPiece( Target, piece.color, piece.type );
        RemovePiece( Start );
    }

    void Chessboard::PromotePiece( int Square, PieceType NewType )
    {
        ChessPiece OldPiece = m_Bitboards.GetPieceAtSquare( Square );
        RemovePiece( Square );
        AddPiece( Square, OldPiece.color, NewType );
    }
 
    void Chessboard::GenerateMoves()
    {
        /*auto temp = StopWatch( "MoveGenerator" );
        std::cout << "Generating moves...\n";*/
        m_LegalMoves.clear();   // Delete old moves

        Color FriendlyColor = m_WhiteToPlay ? Color::White : Color::Black;
        Color EnemyColor = m_WhiteToPlay ? Color::Black : Color::White;

        short KingPos = m_WhiteToPlay ? m_WhiteKingPos : m_BlackKingPos;
        CastlingRights Rights = m_WhiteToPlay ? m_WhiteCastling : m_BlackCastling;

        MoveGen::RayPayload RayResult = MoveGen::CalculateRays( KingPos, m_Bitboards, m_EnPassantSquare, m_WhiteToPlay );
        
        m_InCheck = RayResult.InCheck;
        m_InDoubleCheck = RayResult.InDoubleCheck;
        m_EnPassantBlocked = RayResult.EnPassantBlocked;
        m_PinMask = RayResult.PinRays;
        m_CheckMask = RayResult.CheckRays;

        const auto& Positions = m_WhiteToPlay ? m_WhitePositions : m_BlackPositions;
        const auto& EnemyPositions = m_WhiteToPlay ? m_BlackPositions : m_WhitePositions;
        m_AttackMask = MoveGen::GetAttackMask( EnemyPositions, m_Bitboards, EnemyColor );

        std::unordered_set<Move> Moves( 27 );   // A queen has a maximum of 27 moves

        for ( auto& Pos : Positions )
        {
            Moves.clear();

            ChessPiece piece = m_Bitboards.GetPieceAtSquare( Pos );

            if ( m_InDoubleCheck && piece.type != PieceType::King )     // If we are in double check then only the king can move
                continue;

            switch ( piece.type )
            {
            case PieceType::King:
                Moves = MoveGen::King::GetKingMoves( Pos, m_Bitboards, FriendlyColor, m_PinMask, m_CheckMask, m_AttackMask, Rights );
                break;
            case PieceType::Pawn:
                Moves = MoveGen::Pawn::GetPawnMoves( Pos, m_Bitboards, FriendlyColor, m_EnPassantBlocked ? -1 : m_EnPassantSquare, m_PinMask, m_CheckMask );
                break;
            case PieceType::Knight:
                Moves = MoveGen::Knight::GetKnightMoves( Pos, m_Bitboards, FriendlyColor, m_PinMask, m_CheckMask );
                break;
            case PieceType::Bishop:
                Moves = MoveGen::Bishop::GetBishopMoves( Pos, m_Bitboards, FriendlyColor, m_PinMask, m_CheckMask, KingPos );
                break;
            case PieceType::Rook:
                Moves = MoveGen::Rook::GetRookMoves( Pos, m_Bitboards, FriendlyColor, m_PinMask, m_CheckMask, KingPos );
                break;
            case PieceType::Queen:
                Moves = MoveGen::Queen::GetQueenMoves( Pos, m_Bitboards, FriendlyColor, m_PinMask, m_CheckMask, KingPos );
                break;
            case PieceType::None:
            default:
                break;
            }

            m_LegalMoves.insert( m_LegalMoves.cend(), Moves.begin(), Moves.end());
        }
        //std::cout << "Moves finished generating!\n";
    }

    int Chessboard::Perft( int Depth, bool FirstPass )
    {
        if ( Depth < 0 )
            throw std::exception( "wtf bro | Depth parameter for method Chessboard::Perft( int Depth ) cannot be negative." );

        if ( Depth == 0 )
            return 1;

        int n_nodes = 0;
        int move_nodes = 0;
        std::vector<Move> Moves = m_LegalMoves;

        for ( const auto& move : Moves )
        {
            MakeMove( move );
            if ( !FirstPass )
            {
                n_nodes += Perft( Depth - 1, false );
                UnMakeMove();
            }
            else
            {
                move_nodes = Perft( Depth - 1, false );

                std::string move_repr = move.GetRepr();
                std::cout << move_repr << ": " << move_nodes << "\n";

                n_nodes += move_nodes;
                UnMakeMove();
            }
        }

        return n_nodes;
    }

    Chessboard::BoardInfo Chessboard::GetBoardInfo() const
    {
        return { m_LegalMoves, m_Bitboards, m_WhiteToPlay, m_EnPassantSquare, m_FullmoveClock, m_HalfmoveClock, m_WhiteCastling, m_BlackCastling, m_WhiteKingPos, m_BlackKingPos };
    }

    void Chessboard::LoadFEN( const std::string& FEN_Pos )
    {
        // Set variables to default values

        m_Bitboards.Reset(); // Reset board

        m_LegalMoves.clear();
        m_PieceHistory.clear();
        
        m_WhitePositions.clear();
        m_BlackPositions.clear();

        m_WhiteCastling = CastlingRights::None;             // Set castling rights
        m_BlackCastling = CastlingRights::None;             // for both sides to none

        m_EnPassantSquare = -1;
        m_FullmoveClock = 1;
        m_HalfmoveClock = 0;

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
                PieceType type = GetPieceFromRepr( Letter );
                AddPiece( CurrentSq, Color::White, type );
                if ( type == PieceType::King )
                    m_WhiteKingPos = CurrentSq;
            }
            else if ( 'a' <= Letter && Letter <= 'z' )  // Letter is lower case (black)
            {
                PieceType type = GetPieceFromRepr( Letter );
                AddPiece( CurrentSq, Color::Black, type );
                if ( type == PieceType::King )
                    m_BlackKingPos = CurrentSq;
            }
            --CurrentSq;
        }

        std::cout << "Engine has finished loading FEN " << FEN_Pos << "\n";
        GenerateMoves();
    }

    void Chessboard::MakeMove( Move move )
    {
        int Start = move.Start(), Target = move.Target();
        MoveFlag Flag = move.Flag();

        int CaptureTarget = Target;
        if ( Flag == MoveFlag::EnPassant )
            CaptureTarget = m_WhiteToPlay ? Target - 8 : Target + 8;

        ChessPiece PieceMoved = m_Bitboards.GetPieceAtSquare( Start );
        ChessPiece PieceCaptured = m_Bitboards.GetPieceAtSquare( CaptureTarget );
        
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

            if ( PieceMoved.color == Color::White )
                m_WhiteKingPos = Target;
            else if ( PieceMoved.color == Color::Black )
                m_BlackKingPos = Target;
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

        AddPiece( Target, PieceMoved.color, PieceMoved.type );       // Move piece to new square


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
            ChessPiece Rook = m_Bitboards.GetPieceAtSquare( RookStart );
            RemovePiece( RookStart );
            AddPiece( RookTarget, Rook.color, Rook.type );
        }
        else if ( Flag == MoveFlag::DoublePawnMove )
        {
            m_EnPassantSquare = m_WhiteToPlay ? Start + 8 : Start - 8;
        }
        if ( (Flag & MoveFlag::PromoteKnight) != MoveFlag::None )
        {
            switch ( Flag )
            {
            case Chess::MoveFlag::PromoteKnight:
            case Chess::MoveFlag::PromoteKnightCapture:
                PromotePiece( Target, PieceType::Knight );
                break;
            case Chess::MoveFlag::PromoteBishop:
            case Chess::MoveFlag::PromoteBishopCapture:
                PromotePiece( Target, PieceType::Bishop );
                break;
            case Chess::MoveFlag::PromoteRook:
            case Chess::MoveFlag::PromoteRookCapture:
                PromotePiece( Target, PieceType::Rook );
                break;
            case Chess::MoveFlag::PromoteQueen:
            case Chess::MoveFlag::PromoteQueenCapture:
                PromotePiece( Target, PieceType::Queen );
                break;
            default:
                break;
            }
        }

        if ( ResetHalfclock )
            m_HalfmoveClock = 0;
        else
            ++m_HalfmoveClock;

        if ( !m_WhiteToPlay )
            ++m_FullmoveClock;

        m_WhiteToPlay = !m_WhiteToPlay;

        m_MoveHistory.push_back( move );    // Add move to the move history

        GenerateMoves();    // Generate the new move list
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

        ChessPiece PieceMoved = m_Bitboards.GetPieceAtSquare( Target );
        ChessPiece PieceCaptured;

        if ( PieceMoved.type == PieceType::King )
        {
            if ( PieceMoved.color == Color::White )
                m_WhiteKingPos = Start;
            else if ( PieceMoved.color == Color::Black )
                m_BlackKingPos = Start;
        }

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
            ChessPiece Rook = m_Bitboards.GetPieceAtSquare( RookTarget );
            RemovePiece( RookTarget );
            AddPiece( RookStart, Rook.color, Rook.type );
        }

        RemovePiece( Target );
        AddPiece( Start, PieceMoved.color, PieceMoved.type );

        if ( !PieceCaptured.IsNullPiece() )
        {
            AddPiece( CaptureTarget, PieceCaptured.color, PieceCaptured.type );
        }

        if ( (Flag & MoveFlag::PromoteKnight) != MoveFlag::None )
        {
            PromotePiece( Start, PieceType::Pawn );
        }

        if ( !m_WhiteToPlay )
            --m_FullmoveClock;

        GenerateMoves();    // Regenerate the move list
    }

    Move Chessboard::GetLastMove() const
    {
        if ( m_MoveHistory.size() > 0 )
            return m_MoveHistory.back();
        return Move();
    }

    int Chessboard::RunPerft( int Depth )
    {
        return Perft( Depth, true );
    }
}