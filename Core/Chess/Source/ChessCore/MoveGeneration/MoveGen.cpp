#include "MoveGen.h"
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include "../Stopwatch/Stopwatch.h"


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

    Bitboard GetRookMoveMask( int Square, Bitboard Occupied )
    {
        Bitboard BlockerMask = RookBlockerMasks[Square];
        BlockerMask &= Occupied;
        int MagicIdx = Magic::GetMagicIdx( BlockerMask, Square, false );
        return RookMoveMasks[Square][MagicIdx];
    }
    Bitboard GetBishopMoveMask( int Square, Bitboard Occupied )
    {
        Bitboard BlockerMask = BishopBlockerMasks[Square];
        BlockerMask &= Occupied;
        int MagicIdx = Magic::GetMagicIdx( BlockerMask, Square, true );
        return BishopMoveMasks[Square][MagicIdx];
    }
}

namespace Chess::MoveGen
{
    std::array<std::unordered_map<int, Bitboard>, 64> CreateAlignMasks()
    {
        std::array<std::unordered_map<int, Bitboard>, 64> result;
        ChessCoord Center;
        Bitboard Mask;

        short NewSquare;
        
        for ( int Square = 0; Square < 64; ++Square )
        {
            result[Square] = std::unordered_map<int, Bitboard>();
            for ( const auto& dir : DirChanges )
            {
                Center = ChessCoord( Square );
                Mask = 0;

                for ( int _ = 0; _ < 8; ++_ )
                {
                    Center += dir;
                    if ( Center.IsValid() )
                    {
                        NewSquare = Center.AsSquare();
                        Mask |= Bit << (unsigned int)NewSquare;
                        result[Square][NewSquare] = Mask;
                        continue;
                    }
                }
            }
        }

        return result;
    }

    const auto AlignMasks = CreateAlignMasks();

    MoveGenInfo CreateMoveGenInfo( short KingPos, const Bitboards& bitboards, short EnPassantSquare, bool IsWhite )
    {
        auto clock = StopWatch( "Creating MoveGenInfo" );
        MoveGenInfo result;

        short EnPassantPiecePos;
        if ( EnPassantSquare == -1 )
            EnPassantPiecePos = -1;
        else
            EnPassantPiecePos = IsWhite ? EnPassantSquare - 8 : EnPassantSquare + 8;

        Color FriendlyColor = IsWhite ? Color::White : Color::Black;
        Color EnemyColor = IsWhite ? Color::Black : Color::White;

        result.AttackMask = GetAttackMask( bitboards, EnemyColor, KingPos, &result );

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
                    if ( EnPassantAlongRay && !IsDiagonalDir )
                    {
                        result.EnPassantBlocked = true;
                        break;
                    }

                    if ( FriendlyAlongRay )
                    {
                        if ( !EnPassantAlongRay )
                            result.PinRays |= Ray;
                    }
                    else
                    {   // No friendly to block the check
                        result.CheckRays |= Ray;
                    }
                    break;
                }
            }
            IsDiagonalDir = !IsDiagonalDir;
        }

        return result;
    }

    Bitboard GetAttackMask( const Bitboards& bitboards, Color EnemyColor, int KingPos, MoveGenInfo* Info )
    {
        auto clock = StopWatch( "Creating Attack Mask" );
        Bitboard result;

        bool IsWhite = EnemyColor == Color::White;
        Color FriendlyColor = IsWhite ? Color::Black : Color::White;
        Bitboard Pieces = bitboards.GetColorMask( EnemyColor );

        const auto& PawnAttackMasks = IsWhite ? Pawn::WhitePawnAttacks : Pawn::BlackPawnAttacks;

        Bitboard AllPieces = bitboards.GetColorMask( FriendlyColor ) | bitboards.GetColorMask( EnemyColor );
        Bitboard KingMask = IsWhite ? bitboards.KingBlack : bitboards.KingWhite;
        AllPieces ^= KingMask;
        Bitboard BlockerMask, Blockers, MoveMask;
        int MagicIdx;

        Bitboard Mask;
        ChessPiece piece;
        int Pos;
        while ( Pieces )
        {
            Pos = Pieces.BitscanForward();
            Pieces &= Pieces - 1;

            piece = bitboards.GetPieceAtSquare( Pos );
            if ( piece.IsNullPiece() )
                continue;

            switch ( piece.type )
            {
            case PieceType::King:
                Mask = King::KingMoveBitboards[Pos];
                result |= Mask;
                break;
            case PieceType::Pawn:
                Mask = PawnAttackMasks[Pos];
                result |= Mask;
                break;
            case PieceType::Knight:
                Mask = Knight::KnightMoveBitboards[Pos];
                result |= Mask;
                break;
            case PieceType::Bishop:
                BlockerMask = SlidingPieces::BishopBlockerMasks[Pos];
                Blockers = BlockerMask & AllPieces;
                MagicIdx = Magic::GetMagicIdx( Blockers, Pos, true );

                Mask = SlidingPieces::BishopMoveMasks[Pos][MagicIdx];
                result |= Mask;
                break;
            case PieceType::Rook:
                BlockerMask = SlidingPieces::RookBlockerMasks[Pos];
                Blockers = BlockerMask & AllPieces;
                MagicIdx = Magic::GetMagicIdx( Blockers, Pos, false );

                Mask = SlidingPieces::RookMoveMasks[Pos][MagicIdx];
                result |= Mask;
                break;
            case PieceType::Queen:
                BlockerMask = SlidingPieces::RookBlockerMasks[Pos];
                Blockers = BlockerMask & AllPieces;
                MagicIdx = Magic::GetMagicIdx( Blockers, Pos, false );

                Mask = SlidingPieces::RookMoveMasks[Pos][MagicIdx];

                BlockerMask = SlidingPieces::BishopBlockerMasks[Pos];
                Blockers = BlockerMask & AllPieces;
                MagicIdx = Magic::GetMagicIdx( Blockers, Pos, true );

                Mask |= SlidingPieces::BishopMoveMasks[Pos][MagicIdx];
                result |= Mask;
                break;
            case PieceType::None:
            default:
                break;
            }

            if ( Info != nullptr && Mask.IsOccupied( KingPos ) )
            {
                if ( Info->InCheck )
                    Info->InDoubleCheck = true;
                Info->InCheck = true;

                Info->CheckingPieces |= Bit << (unsigned int)Pos;
            }
        }
        return result;
    }
}

namespace Chess::MoveGen::King
{
    std::array<Move, 32> GetKingMoves(int Square, const Bitboards& bitboards, Color FriendlyColor, Bitboard AttackMask, CastlingRights Rights, MoveGenInfo Info )
    {
        std::array<Move, 32> result;
        size_t Top = 0;

        Bitboard FriendlyMask = bitboards.GetColorMask( FriendlyColor );
        Bitboard EnemyMask = bitboards.GetColorMask( FriendlyColor == Color::White ? Color::Black : Color::White );

        Bitboard MoveMask = KingMoveBitboards[Square];
        MoveMask &= ~(FriendlyMask | AttackMask);

        MoveFlag Flag;

        int Target;
        while ( MoveMask )
        {
            Target = MoveMask.BitscanForward();
            MoveMask &= MoveMask - 1;

            Flag = MoveFlag::None;
            if ( EnemyMask.IsOccupied( Target ) )
                Flag = MoveFlag::Capture;
            result[Top] = Move( Square, Target, Flag );
            ++Top;
        }

        if ( !Info.InCheck && Rights != CastlingRights::None )
        {
            bool IsWhite = FriendlyColor == Color::White;
            int CastleTarget;

            Bitboard KingSideBlock  = 0b01100000;
            Bitboard KingSideCheck  = 0b01100000;
            Bitboard QueenSideBlock = 0b00001110;
            Bitboard QueenSideCheck = 0b00001100;

            Bitboard BlockerMask;
            Bitboard CheckMask;

            if ( (Rights & CastlingRights::Kingside) == CastlingRights::Kingside )
            {
                if ( !IsWhite )
                {
                    KingSideBlock <<= 56;
                    KingSideCheck <<= 56;
                }

                BlockerMask = KingSideBlock;
                CheckMask = KingSideCheck;
                
                if ( (BlockerMask & ~(FriendlyMask | EnemyMask)) == KingSideBlock && (CheckMask & ~(AttackMask)) == KingSideCheck )
                {
                    CastleTarget = IsWhite ? 6 : 62;
                    result[Top] = Move( Square, CastleTarget, MoveFlag::CastleKing );
                    ++Top;
                }
            }

            if ( (Rights & CastlingRights::QueenSide) == CastlingRights::QueenSide )
            {
                if ( !IsWhite )
                {
                    QueenSideBlock <<= 56;
                    QueenSideCheck <<= 56;
                }

                BlockerMask = QueenSideBlock;
                CheckMask = QueenSideCheck;

                if ( (BlockerMask & ~(FriendlyMask | EnemyMask)) == QueenSideBlock && (CheckMask & ~(AttackMask)) == QueenSideCheck )
                {
                    CastleTarget = IsWhite ? 2 : 58;
                    result[Top] = Move( Square, CastleTarget, MoveFlag::CastleQueen );
                    ++Top;
                }
            }
        }

        return result;
    }
}

namespace Chess::MoveGen::Pawn
{
    std::array<Move, 32> GetPawnMoves( int Square, const Bitboards& bitboards, Color FriendlyColor, int EnPassantSquare, MoveGenInfo Info, int KingPos )
    {
        std::array<Move, 32> result;
        size_t Top = 0;
        
        if ( Info.InDoubleCheck )
            return result;
        
        bool IsWhite = FriendlyColor == Color::White;

        Color EnemyColor = IsWhite ? Color::Black : Color::White;

        Bitboard FriendMask = bitboards.GetColorMask( FriendlyColor );
        Bitboard EnemyMask = bitboards.GetColorMask( EnemyColor );
        Bitboard AllPieces = FriendMask | EnemyMask;

        if ( EnPassantSquare != -1 )
            EnemyMask |= Bit << (unsigned int)EnPassantSquare;

        const auto& PawnAttackDir = IsWhite ? WhitePawnAttacksDirs : BlackPawnAttacksDirs;
        Bitboard MoveMask;

        ChessCoord Start = ChessCoord( Square );
        ChessCoord AttackTarget;

        int SinglePushSquare;
        int DoublePushSquare;
        int LeftAttackSquare = -1;
        int RightAttackSquare = -1;

        bool AtStartRank;
        bool WillPromote;
        bool SinglePushBlocked = false;

        if ( IsWhite )
        {
            SinglePushSquare = Square + 8;
            WillPromote = Start.Rank == 6;

            DoublePushSquare = Square + 16;
            AtStartRank = Start.Rank == 1;

            AttackTarget = Start + PawnAttackDir[0];
            if ( AttackTarget.IsValid() )
                LeftAttackSquare = AttackTarget.AsSquare();

            AttackTarget = Start + PawnAttackDir[1];
            if ( AttackTarget.IsValid() )
                RightAttackSquare = AttackTarget.AsSquare();
        }
        else
        {
            SinglePushSquare = Square - 8;
            WillPromote = Start.Rank == 1;

            DoublePushSquare = Square - 16;
            AtStartRank = Start.Rank == 6;

            AttackTarget = Start + PawnAttackDir[0];
            if ( AttackTarget.IsValid() )
                LeftAttackSquare = AttackTarget.AsSquare();

            AttackTarget = Start + PawnAttackDir[1];
            if ( AttackTarget.IsValid() )
                RightAttackSquare = AttackTarget.AsSquare();

        }

        if ( !AllPieces.IsOccupied( SinglePushSquare ) )
            MoveMask |= Bit << (unsigned int)SinglePushSquare;
        else
            SinglePushBlocked = true;

        if ( !SinglePushBlocked && AtStartRank && !AllPieces.IsOccupied(DoublePushSquare) )
            MoveMask |= Bit << (unsigned int)DoublePushSquare;

        // Attacks
        if ( LeftAttackSquare != -1 && EnemyMask.IsOccupied(LeftAttackSquare) )
            MoveMask |= Bit << (unsigned int)LeftAttackSquare;

        if ( RightAttackSquare != -1 && EnemyMask.IsOccupied(RightAttackSquare) )
            MoveMask |= Bit << (unsigned int)RightAttackSquare;

        if ( Info.InCheck )
        {
            MoveMask &= (Info.CheckRays | Info.CheckingPieces);
        }
        if ( Info.PinRays.IsOccupied( Square ) )
        {
            Bitboard AlignMask = AlignMasks[KingPos].at( Square );
            MoveMask &= (Info.PinRays & AlignMask);
        }

        MoveFlag Flag;
        int Target;
        while ( MoveMask )
        {
            Target = MoveMask.BitscanForward();
            MoveMask &= MoveMask - 1;

            Flag = MoveFlag::None;
            if ( Target == DoublePushSquare )
                Flag = MoveFlag::DoublePawnMove;
            else if ( Target == EnPassantSquare )
                Flag = MoveFlag::EnPassant;
            else if ( EnemyMask.IsOccupied( Target ) )
                Flag = MoveFlag::Capture;

            if ( WillPromote )
            {
                MoveFlag IsCapture = ((Flag & MoveFlag::Capture) != MoveFlag::None) ? MoveFlag::Capture : MoveFlag::None;

                result[Top] = Move( Square, Target, MoveFlag::PromoteQueen | IsCapture );
                ++Top;

                result[Top] = Move( Square, Target, MoveFlag::PromoteRook | IsCapture );
                ++Top;

                result[Top] = Move( Square, Target, MoveFlag::PromoteBishop | IsCapture );
                ++Top;

                result[Top] = Move( Square, Target, MoveFlag::PromoteKnight | IsCapture );
                ++Top;
            }
            else
            {
                result[Top] = Move( Square, Target, Flag );
                ++Top;
            }
        }

        return result;
    }
}

namespace Chess::MoveGen::Knight
{
    std::array<Move, 32> GetKnightMoves( short Square, const Bitboards& bitboards, Color FriendlyColor, MoveGenInfo Info, short KingSquare )
    {
        std::array<Move, 32> result;
        size_t Top = 0;

        if ( Info.InDoubleCheck )
            return result;

        Bitboard FriendlyMask = bitboards.GetColorMask( FriendlyColor );
        Bitboard EnemyMask = bitboards.GetColorMask( FriendlyColor == Color::White ? Color::Black : Color::White );

        Bitboard MoveMask = KnightMoveBitboards[Square];
        MoveMask &= ~FriendlyMask;

        if ( Info.InCheck )
        {
            MoveMask &= (Info.CheckRays | Info.CheckingPieces);
        }
        if ( Info.PinRays.IsOccupied( Square ) )
        {
            Bitboard AlignMask = AlignMasks[KingSquare].at( Square );
            MoveMask &= (Info.PinRays & AlignMask);
        }

        MoveFlag Flag;
        int Target;
        while ( MoveMask )
        {
            Target = MoveMask.BitscanForward();
            MoveMask &= MoveMask - 1;

            Flag = MoveFlag::None;
            if ( EnemyMask.IsOccupied( Target ) )
                Flag = MoveFlag::Capture;
            result[Top] = Move( Square, Target, Flag );
            ++Top;
        }
        return result;
    }
}

namespace Chess::MoveGen::Bishop
{
    std::array<Move, 32> GetBishopMoves( short Square, const Bitboards& bitboards, Color FriendlyColor, MoveGenInfo Info, short KingSquare )
    {
        std::array<Move, 32> result;
        size_t Top = 0;

        if ( Info.InDoubleCheck )
            return result;

        Bitboard FriendlyMask = bitboards.GetColorMask( FriendlyColor );
        Bitboard EnemyMask = bitboards.GetColorMask( FriendlyColor == Color::White ? Color::Black : Color::White );

        Bitboard MoveMask = SlidingPieces::GetBishopMoveMask( Square, FriendlyMask | EnemyMask );
        MoveMask &= ~FriendlyMask;

        if ( Info.InCheck )
        {
            MoveMask &= (Info.CheckRays | Info.CheckingPieces);
        }
        if ( Info.PinRays.IsOccupied( Square ) )
        {
            Bitboard AlignMask = AlignMasks[KingSquare].at( Square );
            MoveMask &= (Info.PinRays & AlignMask);
        }

        MoveFlag Flag;
        int Target;
        while ( MoveMask )
        {
            Target = MoveMask.BitscanForward();
            MoveMask &= MoveMask - 1;

            Flag = MoveFlag::None;
            if ( EnemyMask.IsOccupied( Target ) )
                Flag = MoveFlag::Capture;
            result[Top] = Move( Square, Target, Flag );
            ++Top;
        }
        return result;
    }
}

namespace Chess::MoveGen::Rook
{
    std::array<Move, 32> GetRookMoves( short Square, const Bitboards& bitboards, Color FriendlyColor, MoveGenInfo Info, short KingSquare )
    {
        std::array<Move, 32> result;
        size_t Top = 0;

        if ( Info.InDoubleCheck )
            return result;

        Bitboard FriendlyMask = bitboards.GetColorMask( FriendlyColor );
        Bitboard EnemyMask = bitboards.GetColorMask( FriendlyColor == Color::White ? Color::Black : Color::White );

        Bitboard MoveMask = SlidingPieces::GetRookMoveMask( Square, FriendlyMask | EnemyMask );
        MoveMask &= ~FriendlyMask;

        if ( Info.InCheck )
        {
            MoveMask &= (Info.CheckRays | Info.CheckingPieces);
        }
        if ( Info.PinRays.IsOccupied( Square ) )
        {
            Bitboard AlignMask = AlignMasks[KingSquare].at( Square );
            MoveMask &= (Info.PinRays & AlignMask);
        }

        MoveFlag Flag;
        int Target;
        while ( MoveMask )
        {
            Target = MoveMask.BitscanForward();
            MoveMask &= MoveMask - 1;

            Flag = MoveFlag::None;
            if ( EnemyMask.IsOccupied( Target ) )
                Flag = MoveFlag::Capture;
            result[Top] = Move( Square, Target, Flag );
            ++Top;
        }

        return result;
    }
}

namespace Chess::MoveGen::Queen
{
    std::array<Move, 32> GetQueenMoves( short Square, const Bitboards& bitboards, Color FriendlyColor, MoveGenInfo Info, short KingSquare )
    {
        std::array<Move, 32> result;
        size_t Top = 0;

        if ( Info.InDoubleCheck )
            return result;

        Bitboard FriendlyMask = bitboards.GetColorMask( FriendlyColor );
        Bitboard EnemyMask = bitboards.GetColorMask( FriendlyColor == Color::White ? Color::Black : Color::White );

        Bitboard MoveMask = SlidingPieces::GetRookMoveMask( Square, FriendlyMask | EnemyMask ) | SlidingPieces::GetBishopMoveMask( Square, FriendlyMask | EnemyMask );

        MoveMask &= ~FriendlyMask;

        if ( Info.InCheck )
        {
            MoveMask &= (Info.CheckRays | Info.CheckingPieces);
        }
        if ( Info.PinRays.IsOccupied( Square ) )
        {
            Bitboard AlignMask = AlignMasks[KingSquare].at( Square );
            MoveMask &= (Info.PinRays & AlignMask);
        }

        MoveFlag Flag;
        int Target;
        while ( MoveMask )
        {
            Target = MoveMask.BitscanForward();
            MoveMask &= MoveMask - 1;

            Flag = MoveFlag::None;
            if ( EnemyMask.IsOccupied( Target ) )
                Flag = MoveFlag::Capture;
            result[Top] = Move( Square, Target, Flag );
            ++Top;
        }

        return result;
    }
}