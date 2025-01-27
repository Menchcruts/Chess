#include "Move.h"
#include <iostream>
#include <string>

namespace Chess
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

    void Move::Print() const
    {
        std::string flag_string;
        switch ( Flag() )
        {
        case Chess::MoveFlag::DoublePawnMove:
            flag_string = "DoublePawnMove";
            break;
        case Chess::MoveFlag::CastleKing:
            flag_string = "CastleKing";
            break;
        case Chess::MoveFlag::CastleQueen:
            flag_string = "CastleQueen";
            break;
        case Chess::MoveFlag::Capture:
            flag_string = "Capture";
            break;
        case Chess::MoveFlag::EnPassant:
            flag_string = "EnPassant";
            break;
        case Chess::MoveFlag::PromoteKnight:
            flag_string = "PromoteKnight";
            break;
        case Chess::MoveFlag::PromoteBishop:
            flag_string = "PromoteBishop";
            break;
        case Chess::MoveFlag::PromoteRook:
            flag_string = "PromoteRook";
            break;
        case Chess::MoveFlag::PromoteQueen:
            flag_string = "PromoteQueen";
            break;
        case Chess::MoveFlag::PromoteKnightCapture:
            flag_string = "PromoteKnightCapture";
            break;
        case Chess::MoveFlag::PromoteBishopCapture:
            flag_string = "PromoteBishopCapture";
            break;
        case Chess::MoveFlag::PromoteRookCapture:
            flag_string = "PromoteRookCapture";
            break;
        case Chess::MoveFlag::PromoteQueenCapture:
            flag_string = "PromoteQueenCapture";
            break;
        case Chess::MoveFlag::None:
        default:
            flag_string = "None";
            break;
        }

        std::cout << "Move( " << Start() << ", " << Target() << ", " << flag_string << " )\n";
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
