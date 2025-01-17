#include "Move.h"
#include <iostream>

namespace Chess
{
    MoveFlag operator |( MoveFlag right, MoveFlag left )
    {
        return static_cast<MoveFlag>(static_cast<int>(right) | static_cast<int>(left));
    }
    MoveFlag operator &( MoveFlag right, MoveFlag left )
    {
        return static_cast<MoveFlag>(static_cast<int>(right) & static_cast<int>(left));
    }
    MoveFlag& operator |=( MoveFlag& right, MoveFlag left )
    {
        right = right | left;
        return right;
    }
    MoveFlag& operator &=( MoveFlag& right, MoveFlag left )
    {
        right = right & left;
        return right;
    }


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

    Move::Move( int start, int target, MoveFlag flag )
    {
        MoveInfo |= (start);
        MoveInfo |= (target << 6);
        MoveInfo |= (static_cast<int>(flag) << 12);
    }

    bool Move::operator==( const Move& other ) const
    {
        return this->MoveInfo == other.MoveInfo;
    }
}
