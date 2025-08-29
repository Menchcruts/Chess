#include "Move.h"
#include <iostream>
#include <string>

namespace Chess_Old
{
    int Move::Start() const
    {
        return 63 & MoveInfo;   // Return first 6 bits as int
    }

    int Move::Target() const
    {
        return 63 & (MoveInfo >> 6); // Return second set of 6 bits as int
    }

    MoveFlag Move::Flag() const
    {
        return static_cast<MoveFlag>(15 & (MoveInfo >> 12)); // Return last 4 bits as MoveFlag enum
    }

    bool Move::IsCapture() const
    {
        return (MoveInfo >> 12) & 0b0100;
    }

    bool Move::IsEnPassant() const
    {
        return (MoveInfo >> 12) == 0b0101;
    }

    bool Move::IsCastle() const
    {
        return (MoveInfo >> 12) == 0b0010 || (MoveInfo >> 12) == 0b0011;
    }

    bool Move::IsPromotion() const
    {
        return (MoveInfo >> 12) & 0b1000;
    }

    void Move::Print() const
    {
        std::string flag_string;
        switch ( Flag() )
        {
        case Chess_Old::MoveFlag::DoublePawnMove:
            flag_string = "DoublePawnMove";
            break;
        case Chess_Old::MoveFlag::CastleKing:
            flag_string = "CastleKing";
            break;
        case Chess_Old::MoveFlag::CastleQueen:
            flag_string = "CastleQueen";
            break;
        case Chess_Old::MoveFlag::Capture:
            flag_string = "Capture";
            break;
        case Chess_Old::MoveFlag::EnPassant:
            flag_string = "EnPassant";
            break;
        case Chess_Old::MoveFlag::PromoteKnight:
            flag_string = "PromoteKnight";
            break;
        case Chess_Old::MoveFlag::PromoteBishop:
            flag_string = "PromoteBishop";
            break;
        case Chess_Old::MoveFlag::PromoteRook:
            flag_string = "PromoteRook";
            break;
        case Chess_Old::MoveFlag::PromoteQueen:
            flag_string = "PromoteQueen";
            break;
        case Chess_Old::MoveFlag::PromoteKnightCapture:
            flag_string = "PromoteKnightCapture";
            break;
        case Chess_Old::MoveFlag::PromoteBishopCapture:
            flag_string = "PromoteBishopCapture";
            break;
        case Chess_Old::MoveFlag::PromoteRookCapture:
            flag_string = "PromoteRookCapture";
            break;
        case Chess_Old::MoveFlag::PromoteQueenCapture:
            flag_string = "PromoteQueenCapture";
            break;
        case Chess_Old::MoveFlag::None:
        default:
            flag_string = "None";
            break;
        }

        std::cout << "Move( " << Start() << ", " << Target() << ", " << flag_string << " )\n";
    }

    std::string Move::GetRepr() const
    {
        const char* files = "abcdefgh";
        const char* ranks = "12345678";

        short start = Start();
        short start_rank = start >> 3;
        short start_file = start & 7;

        short target = Target();
        short target_rank = target >> 3;
        short target_file = target & 7;

        std::string result = { files[start_file], ranks[start_rank], files[target_file], ranks[target_rank] };

        MoveFlag flag = Flag();
        if ( (flag & MoveFlag::PromoteKnight) != MoveFlag::None )
        {
            if ( flag == MoveFlag::PromoteKnight || flag == MoveFlag::PromoteKnightCapture )
                result.push_back( 'n' );
            else if ( flag == MoveFlag::PromoteBishop || flag == MoveFlag::PromoteBishopCapture )
                result.push_back( 'b' );
            else if ( flag == MoveFlag::PromoteRook || flag == MoveFlag::PromoteRookCapture )
                result.push_back( 'r' );
            else if ( flag == MoveFlag::PromoteQueen || flag == MoveFlag::PromoteQueenCapture )
                result.push_back( 'q' );
        }

        return result;
    }

    Move::Move( int start, int target, MoveFlag flag )
    {
        MoveInfo |= (start);
        MoveInfo |= (target << 6);
        MoveInfo |= (static_cast<int>(flag) << 12);
    }

    bool Move::IsNullMove() const
    {
        return MoveInfo == 0;
    }

    bool Move::operator==( const Move& other ) const
    {
        return this->MoveInfo == other.MoveInfo;
    }
}
