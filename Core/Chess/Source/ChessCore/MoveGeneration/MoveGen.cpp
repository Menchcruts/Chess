#include "MoveGen.h"
#include <iostream>
#include <algorithm>


namespace Chess::MoveGen::SlidingPieces
{
    std::vector<Bitboard> CreateBlockerPerms( Bitboard BlockerMask )
    {
        std::vector<Bitboard> result;
        std::vector<short> SquareIdxs;
        SquareIdxs.reserve( 12 );

        for ( int i = 0; i < 64; ++i )
            if ( BlockerMask.IsOccupied( i ) )
                SquareIdxs.push_back( i );

        int NumPatterns = 1 << SquareIdxs.size();

        Bitboard Mask;
        Bitboard TempBit;

        for ( int pattern = 0; pattern < NumPatterns; ++pattern )
        {
            Mask = 0;
            TempBit = 0;
            for ( int i = 0; i < SquareIdxs.size(); ++i )
            {
                TempBit = (pattern >> i) & 1;
                Mask |= (TempBit << (unsigned int)SquareIdxs[i]);
            }
            result.push_back( Mask );
        }

        return result;
    }

    Bitboard MoveMaskFromBlocker( short Square, Bitboard BlockerMask, bool IsDiagonal )
    {
        Bitboard Result;

        int StartIdx = IsDiagonal ? 1 : 0;

        ChessCoord Center = ChessCoord( Square );
        ChessCoord NewCoord;
        short NewSquare;

        for ( int i = StartIdx; i < 8; i += 2 )
        {
            auto& dir = DirChanges[i];
            NewCoord = Center;
            
            for ( int _ = 0; _ < 8; ++_ )
            {
                NewCoord += dir;
                if ( !NewCoord.IsValid() )
                    break;
                
                NewSquare = NewCoord.AsSquare();
                Result |= Bit << (unsigned int)NewSquare;

                if ( BlockerMask.IsOccupied( NewSquare ) )
                    break;
            }
        }
        return Result;
    }

    std::array<std::unordered_map<int, Bitboard>, 64> CreateRookMoves()
    {
        int total_size = 0;

        std::array<std::unordered_map<int, Bitboard>, 64> result{ };
        
        std::unordered_map<int, Bitboard> Masks( 1600 );   // Average number of perms for rook masks

        Bitboard BlockerMask, MoveMask;
        int MagicIdx;
        std::vector<Bitboard> BlockerPerms;

        for ( short Square = 0; Square < 64; ++Square )
        {
            Masks.clear();

            BlockerMask = RookBlockerMasks[Square];
            BlockerPerms = CreateBlockerPerms( BlockerMask );

            for ( auto& BlockMask : BlockerPerms )
            {
                MoveMask = MoveMaskFromBlocker( Square, BlockMask, false );
                MagicIdx = Magic::GetMagicIdx( BlockMask, Square, false );
                Masks[MagicIdx] = MoveMask;

                ++total_size;
            }
            result[Square] = Masks;
        }
        std::cout << "Total Rook Masks: " << total_size << "\n";
        return result;
    }
    std::array<std::unordered_map<int, Bitboard>, 64> CreateBishopMoves()
    {
        int total_size = 0;

        std::array<std::unordered_map<int, Bitboard>, 64> result{ };

        std::unordered_map<int, Bitboard> Masks( 82 );   // Average number of perms for bishop masks

        Bitboard BlockerMask, MoveMask;
        int MagicIdx;
        std::vector<Bitboard> BlockerPerms;

        for ( short Square = 0; Square < 64; ++Square )
        {
            Masks.clear();

            BlockerMask = BishopBlockerMasks[Square];
            BlockerPerms = CreateBlockerPerms( BlockerMask );

            for ( auto& BlockMask : BlockerPerms )
            {
                MoveMask = MoveMaskFromBlocker( Square, BlockMask, true );
                MagicIdx = Magic::GetMagicIdx( BlockMask, Square, true );
                Masks[MagicIdx] = MoveMask;

                ++total_size;
            }
            result[Square] = Masks;
        }
        std::cout << "Total Bishop Masks: " << total_size << "\n";
        return result;
    }

    std::array<std::unordered_map<int, Bitboard>, 64> RookMoveMasks = CreateRookMoves();
    std::array<std::unordered_map<int, Bitboard>, 64> BishopMoveMasks = CreateBishopMoves();
}

namespace Chess::MoveGen::King
{
    std::unordered_set<Move> Chess::MoveGen::King::GetKingMoves(int Square, const Bitboards& bitboards, Color FriendlyColor, Bitboard PinRays, Bitboard CheckRays, Bitboard AttackMask, CastlingRights Rights )
    {
        std::unordered_set<Move> result( 10 );

        Bitboard FriendlyMask = bitboards.GetColorMask( FriendlyColor );
        Bitboard EnemyMask = bitboards.GetColorMask( FriendlyColor == Color::White ? Color::Black : Color::White );

        Bitboard MoveMask = KingMoveBitboards[Square];
        MoveMask &= ~(FriendlyMask | AttackMask);

        MoveFlag Flag;

        for ( int i = 0; i < 64; ++i )
        {
            if ( MoveMask.IsOccupied( i ) )
            {
                Flag = MoveFlag::None;
                if ( EnemyMask.IsOccupied( i ) )
                    Flag = MoveFlag::Capture;

                result.emplace( Square, i, Flag );
            }
        }

        if ( Rights != CastlingRights::None )
        {
            bool IsWhite = FriendlyColor == Color::White;
            int CastleTarget;
            Bitboard CastleMask, KingsideCastleMask = 0b01100000, QueensideCastleMask = 0b00001110;
            
            if ( (Rights | CastlingRights::Kingside) != CastlingRights::None )
            {
                if ( !IsWhite )
                    KingsideCastleMask <<= 56;
                CastleMask = KingsideCastleMask;

                CastleMask &= ~(FriendlyMask | EnemyMask | AttackMask);

                if ( CastleMask == KingsideCastleMask )
                {
                    CastleTarget = IsWhite ? 6 : 62;
                    result.emplace( Square, CastleTarget, MoveFlag::CastleKing );
                }
            }
            if ( (Rights | CastlingRights::QueenSide) != CastlingRights::None )
            {
                if ( !IsWhite )
                    QueensideCastleMask <<= 56;
                CastleMask = QueensideCastleMask;

                CastleMask &= ~(FriendlyMask | EnemyMask | AttackMask);

                if ( CastleMask == QueensideCastleMask )
                {
                    CastleTarget = IsWhite ? 2 : 58;
                    result.emplace( Square, CastleTarget, MoveFlag::CastleQueen );
                }
            }
        }

        return result;
    }
}

namespace Chess::MoveGen::Pawn
{
    static Bitboard GetPawnAttackMask( short Square, Color FriendlyColor )
    {
        Bitboard Result;

        bool IsWhite = FriendlyColor == Color::White;
        ChessCoord Start = ChessCoord( Square );
        ChessCoord NewSquare;

        auto& Attacks = IsWhite ? WhitePawnAttacks : BlackPawnAttacks;
        for ( auto& Attack : Attacks )
        {
            NewSquare = Start + Attack;
            if ( NewSquare.IsValid() )
                Result |= Bit << (unsigned int)NewSquare.AsSquare();
        }
        return Result;
    }

    std::unordered_set<Move> GetPawnMoves( short Square, const Bitboards& bitboards, Color FriendlyColor, short EnPassantSquare, Bitboard PinRays, Bitboard CheckRays )
    {
        std::unordered_set<Move> result( 16 );  // Worst case is 12 moves

        ChessCoord CurrentSquare = ChessCoord( Square );
        short NewSquare;

        bool IsWhite = FriendlyColor == Color::White;
        bool AtStartRank = IsWhite ? CurrentSquare.Rank == 1 : CurrentSquare.Rank == 6;
        bool NotPinned = !PinRays.IsOccupied( Square );
        bool FirstPushBlocked = false;

        short PromotionRank = IsWhite ? 7 : 0;

        Bitboard FriendlyMask = bitboards.GetColorMask( FriendlyColor );
        Bitboard EnemyMask = bitboards.GetColorMask( IsWhite ? Color::Black : Color::White );
        Bitboard AllPieces = FriendlyMask | EnemyMask;

        ChessCoord Forward = IsWhite ? ChessCoord( 1, 0 ) : ChessCoord( -1, 0 );

        // Single push
        CurrentSquare += Forward;
        NewSquare = CurrentSquare.AsSquare();
        if ( AllPieces.IsOccupied( NewSquare ) )
            FirstPushBlocked = true;

        if ( !FirstPushBlocked && (NotPinned || PinRays.IsOccupied( NewSquare )) )
        {
            if ( CurrentSquare.Rank == PromotionRank )
            {
                result.emplace( Square, NewSquare, MoveFlag::PromoteQueen );
                result.emplace( Square, NewSquare, MoveFlag::PromoteRook );
                result.emplace( Square, NewSquare, MoveFlag::PromoteBishop );
                result.emplace( Square, NewSquare, MoveFlag::PromoteQueen );
            }
            else
            {
                result.emplace( Square, NewSquare, MoveFlag::None );
            }
        }

        // Double push
        CurrentSquare += Forward;
        NewSquare = CurrentSquare.AsSquare();
        if ( AtStartRank && !FirstPushBlocked && (NotPinned || PinRays.IsOccupied( NewSquare )) )
        {
            result.emplace( Square, NewSquare, MoveFlag::DoublePawnMove );
        }

        // Attacks
        CurrentSquare = ChessCoord( Square );

        Bitboard AttackMask = GetPawnAttackMask( Square, FriendlyColor );
        AttackMask &= EnemyMask;
        if ( !NotPinned )
            AttackMask &= PinRays;

        auto& Attacks = IsWhite ? WhitePawnAttacks : BlackPawnAttacks;
        for ( auto& Attack : Attacks )
        {
            CurrentSquare += Attack;
            NewSquare = CurrentSquare.AsSquare();
            if ( AttackMask.IsOccupied( NewSquare ) || NewSquare == EnPassantSquare )
            {
                if ( CurrentSquare.Rank == PromotionRank )
                {
                    result.emplace( Square, NewSquare, MoveFlag::PromoteQueenCapture );
                    result.emplace( Square, NewSquare, MoveFlag::PromoteRookCapture );
                    result.emplace( Square, NewSquare, MoveFlag::PromoteBishopCapture );
                    result.emplace( Square, NewSquare, MoveFlag::PromoteKnightCapture );
                }
                else if ( NewSquare == EnPassantSquare )
                {
                    result.emplace( Square, NewSquare, MoveFlag::EnPassant );
                }
                else
                {
                    result.emplace( Square, NewSquare, MoveFlag::Capture );
                }
            }

            CurrentSquare -= Attack;
        }
        return result;
    }
}

namespace Chess::MoveGen::Knight
{
    std::unordered_set<Move> Chess::MoveGen::Knight::GetKnightMoves( int Square, const Bitboards& bitboards, Color FriendlyColor, Bitboard PinRays, Bitboard CheckRays )
    {
        std::unordered_set<Move> result( 8 );

        Bitboard FriendlyMask = bitboards.GetColorMask( FriendlyColor );
        Bitboard EnemyMask = bitboards.GetColorMask( FriendlyColor == Color::White ? Color::Black : Color::White );

        Bitboard MoveMask = KnightMoveBitboards[Square];
        MoveMask &= ~FriendlyMask;

        if ( CheckRays )
        {
            MoveMask &= CheckRays;
        }
        if ( PinRays.IsOccupied( Square ) )
        {
            MoveMask &= PinRays;
        }

        MoveFlag Flag;

        for ( int i = 0; i < 64; ++i )
        {
            if ( MoveMask.IsOccupied( i ) )
            {
                Flag = MoveFlag::None;
                if ( EnemyMask.IsOccupied( i ) )
                    Flag = MoveFlag::Capture;

                result.emplace( Square, i, Flag );
            }
        }

        return result;
    }
}

namespace Chess::MoveGen::Bishop
{
    std::unordered_set<Move> GetBishopMoves( short Square, const Bitboards& bitboards, Color FriendlyColor, Bitboard PinRays, Bitboard CheckRays, short KingSquare )
    {
        std::unordered_set<Move> result( 13 );

        Bitboard FriendlyMask = bitboards.GetColorMask( FriendlyColor );
        Bitboard EnemyMask = bitboards.GetColorMask( FriendlyColor == Color::White ? Color::Black : Color::White );

        Bitboard BlockerMask = SlidingPieces::BishopBlockerMasks[Square];
        Bitboard Blockers = BlockerMask & (FriendlyMask | EnemyMask);
        int MagicIdx = Magic::GetMagicIdx( Blockers, Square, true );

        Bitboard MoveMask = SlidingPieces::BishopMoveMasks[Square][MagicIdx];
        MoveMask &= ~FriendlyMask;

        if ( CheckRays )
        {
            MoveMask &= CheckRays;
        }
        if ( PinRays.IsOccupied( Square ) )
        {
            Bitboard KingBlockerMask = SlidingPieces::BishopBlockerMasks[KingSquare];
            Bitboard KingBlockers = KingBlockerMask & EnemyMask;
            MagicIdx = Magic::GetMagicIdx( KingBlockers, KingSquare, true );

            Bitboard KingBishopMoveMask = SlidingPieces::BishopMoveMasks[KingSquare][MagicIdx];

            MoveMask &= (PinRays & KingBishopMoveMask);
        }

        MoveFlag Flag;

        for ( int i = 0; i < 64; ++i )
        {
            if ( MoveMask.IsOccupied( i ) )
            {
                Flag = MoveFlag::None;
                if ( EnemyMask.IsOccupied( i ) )
                    Flag = MoveFlag::Capture;

                result.emplace( Square, i, Flag );
            }
        }

        return result;
    }
}

namespace Chess::MoveGen::Rook
{
    std::unordered_set<Move> GetRookMoves( short Square, const Bitboards& bitboards, Color FriendlyColor, Bitboard PinRays, Bitboard CheckRays, short KingSquare )
    {
        std::unordered_set<Move> result( 14 );

        Bitboard FriendlyMask = bitboards.GetColorMask( FriendlyColor );
        Bitboard EnemyMask = bitboards.GetColorMask( FriendlyColor == Color::White ? Color::Black : Color::White );

        Bitboard BlockerMask = SlidingPieces::RookBlockerMasks[Square];
        Bitboard Blockers = BlockerMask & (FriendlyMask | EnemyMask);
        int MagicIdx = Magic::GetMagicIdx( Blockers, Square, false );

        //std::cout << "Rook blocker mask (" << Square << ") " << BlockerMask.m_Bitboard << "\n";
        //std::cout << "Rook blockers (" << Square << ") " << Blockers.m_Bitboard << "\n";

        Bitboard MoveMask = SlidingPieces::RookMoveMasks[Square][MagicIdx];
        //std::cout << "Rook move mask (" << Square << ") " << MoveMask.m_Bitboard << "\n";
        MoveMask &= ~FriendlyMask;

        if ( CheckRays )
        {
            MoveMask &= CheckRays;
        }
        if ( PinRays.IsOccupied( Square ) )
        {
            Bitboard KingBlockerMask = SlidingPieces::RookBlockerMasks[KingSquare];
            Bitboard KingBlockers = KingBlockerMask & EnemyMask;
            MagicIdx = Magic::GetMagicIdx( KingBlockers, KingSquare, false );

            Bitboard KingRookMoveMask = SlidingPieces::RookMoveMasks[KingSquare][MagicIdx];

            MoveMask &= (PinRays & KingRookMoveMask);
        }

        MoveFlag Flag;

        for ( int i = 0; i < 64; ++i )
        {
            if ( MoveMask.IsOccupied( i ) )
            {
                Flag = MoveFlag::None;
                if ( EnemyMask.IsOccupied( i ) )
                    Flag = MoveFlag::Capture;

                result.emplace( Square, i, Flag );
            }
        }

        return result;
    }
}

namespace Chess::MoveGen::Queen
{
    std::unordered_set<Move> GetQueenMoves( short Square, const Bitboards& bitboards, Color FriendlyColor, Bitboard PinRays, Bitboard CheckRays, short KingSquare )
    {
        std::unordered_set<Move> result( 27 );

        Bitboard FriendlyMask = bitboards.GetColorMask( FriendlyColor );
        Bitboard EnemyMask = bitboards.GetColorMask( FriendlyColor == Color::White ? Color::Black : Color::White );

        Bitboard OrthoBlockerMask = SlidingPieces::RookBlockerMasks[Square];
        Bitboard OrthoBlockers = OrthoBlockerMask & (FriendlyMask | EnemyMask);
        int OrthoMagicIdx = Magic::GetMagicIdx( OrthoBlockers, Square, false );

        Bitboard DiagBlockerMask = SlidingPieces::BishopBlockerMasks[Square];
        Bitboard DiagBlockers = DiagBlockerMask & (FriendlyMask | EnemyMask);
        int DiagMagicIdx = Magic::GetMagicIdx( DiagBlockers, Square, true );

        Bitboard MoveMask = SlidingPieces::RookMoveMasks[Square][OrthoMagicIdx] | SlidingPieces::BishopMoveMasks[Square][DiagMagicIdx];

        MoveMask &= ~FriendlyMask;

        if ( CheckRays )
        {
            MoveMask &= CheckRays;
        }
        if ( PinRays.IsOccupied( Square ) )
        {
            Bitboard KingBlockerMask = SlidingPieces::RookBlockerMasks[KingSquare];
            int MagicIdx;
            if ( KingBlockerMask.IsOccupied( Square ) )
            {
                Bitboard KingBlockers = KingBlockerMask & EnemyMask;
                MagicIdx = Magic::GetMagicIdx( KingBlockers, KingSquare, false );

                Bitboard KingRookMoveMask = SlidingPieces::RookMoveMasks[KingSquare][MagicIdx];
                MoveMask &= (PinRays & KingRookMoveMask);
            }
            else
            {
                KingBlockerMask = SlidingPieces::BishopBlockerMasks[KingSquare];
                Bitboard KingBlockers = KingBlockerMask & EnemyMask;
                MagicIdx = Magic::GetMagicIdx( KingBlockers, KingSquare, true );

                Bitboard KingBishopMoveMask = SlidingPieces::BishopMoveMasks[KingSquare][MagicIdx];

                MoveMask &= (PinRays & KingBishopMoveMask);
            }
        }

        MoveFlag Flag;

        for ( int i = 0; i < 64; ++i )
        {
            if ( MoveMask.IsOccupied( i ) )
            {
                Flag = MoveFlag::None;
                if ( EnemyMask.IsOccupied( i ) )
                    Flag = MoveFlag::Capture;

                result.emplace( Square, i, Flag );
            }
        }

        return result;
    }
}

namespace Chess::MoveGen
{
    RayPayload CalculateRays( short KingPos, const Bitboards& bitboards, short EnPassantSquare, bool IsWhite )
    {
        RayPayload result;

        short EnPassantPiecePos;
        if ( EnPassantSquare == -1 )
            EnPassantPiecePos = -1;
        else
            EnPassantPiecePos = IsWhite ? EnPassantSquare - 8 : EnPassantSquare + 8;

        Color FriendlyColor = IsWhite ? Color::White : Color::Black;

        ChessCoord KingCoord = ChessCoord( KingPos );
        ChessCoord NewCoord;

        short NewSquare;
        ChessPiece piece;

        bool IsDiagonalDir = false;

        bool IsFriendly;
        bool FriendlyAlongRay = false;
        bool EnPassantAlongRay = false;

        Bitboard Ray;

        for ( auto& dir : DirChanges )
        {
            Ray = 0;
            NewCoord = KingCoord;
            FriendlyAlongRay = false;
            EnPassantAlongRay = false;

            for ( int _ = 0; _ < 8; ++_ )
            {
                IsFriendly = false;
                NewCoord += dir;
                if ( !NewCoord.IsValid() )
                    break;

                NewSquare = NewCoord.AsSquare();
                piece = bitboards.GetPieceAtSquare( NewSquare );

                Ray |= Bit << (unsigned int)NewSquare;

                if ( piece.IsNullPiece() )
                    continue;

                IsFriendly = piece.color == FriendlyColor;

                if ( IsFriendly )
                {
                    if ( FriendlyAlongRay ) // Two friendly pieces so no pin
                        break;
                    FriendlyAlongRay = true;
                }
                else
                {
                    if ( piece.type == PieceType::King || piece.type == PieceType::Knight ) // Unfriendly kings and knight aren't pinned to us and can't pin us
                        break;
                    else if ( piece.type == PieceType::Pawn )
                    {
                        if ( NewSquare == EnPassantPiecePos )
                        {
                            EnPassantAlongRay = true;
                            continue;
                        }
                        break;
                    }
                    if ( IsDiagonalDir && piece.type == PieceType::Rook )   // Rook can't pin or check diagonally and bishop can't horizontally
                        break;
                    else if ( !IsDiagonalDir && piece.type == PieceType::Bishop )
                        break;

                    // If we get to here the piece is a sliding piece
                    if ( EnPassantAlongRay )
                    {
                        result.EnPassantBlocked = true;
                        break;
                    }

                    if ( FriendlyAlongRay )
                    {
                        result.PinRays |= Ray;
                    }
                    else
                    {   // No friendly to block the check
                        result.CheckRays |= Ray;
                        if ( result.InCheck )
                            result.InDoubleCheck = true;
                        result.InCheck = true;
                    }
                }
            }
            IsDiagonalDir = !IsDiagonalDir;
        }

        return result;
    }

    Bitboard GetAttackMask( const std::unordered_set<short>& Positions, const Bitboards& bitboards, Color EnemyColor )
    {
        Bitboard result;

        Bitboard AllPieces = bitboards.GetColorMask( Color::White ) | bitboards.GetColorMask( Color::Black );
        Bitboard BlockerMask, Blockers, MoveMask;
        int MagicIdx;

        for ( auto& pos : Positions )
        {
            ChessPiece piece = bitboards.GetPieceAtSquare( pos );
            if ( piece.IsNullPiece() )
                continue;

            switch ( piece.type )
            {
            case PieceType::King:
                result |= King::KingMoveBitboards[pos];
                break;
            case PieceType::Pawn:
                result |= Pawn::GetPawnAttackMask( pos, EnemyColor );
                break;
            case PieceType::Knight:
                result |= Knight::KnightMoveBitboards[pos];
                break;
            case PieceType::Bishop:
                BlockerMask = SlidingPieces::BishopBlockerMasks[pos];
                Blockers = BlockerMask & AllPieces;
                MagicIdx = Magic::GetMagicIdx( Blockers, pos, true );
                result |= SlidingPieces::BishopMoveMasks[pos][MagicIdx];
                break;
            case PieceType::Rook:
                BlockerMask = SlidingPieces::RookBlockerMasks[pos];
                Blockers = BlockerMask & AllPieces;
                MagicIdx = Magic::GetMagicIdx( Blockers, pos, false );
                result |= SlidingPieces::RookMoveMasks[pos][MagicIdx];
                break;
            case PieceType::Queen:
                BlockerMask = SlidingPieces::RookBlockerMasks[pos];
                Blockers = BlockerMask & AllPieces;
                MagicIdx = Magic::GetMagicIdx( Blockers, pos, false );
                result |= SlidingPieces::RookMoveMasks[pos][MagicIdx];

                BlockerMask = SlidingPieces::BishopBlockerMasks[pos];
                Blockers = BlockerMask & AllPieces;
                MagicIdx = Magic::GetMagicIdx( Blockers, pos, true );
                result |= SlidingPieces::BishopMoveMasks[pos][MagicIdx];
                break;
            case PieceType::None:
            default:
                break;
            }
        }
        return result;
    }

}
