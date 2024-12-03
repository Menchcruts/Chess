#include "ChessBot.h"
#include <iostream>

namespace ChessBot 
{
    Bot::Bot()
    {
        m_Temp = 1;
    }

    int Bot::GetMove() const 
    {
        return m_Temp;
    }
}