#pragma once

namespace ChessBot 
{
    class Bot
    {
    private:
        int m_Temp;
    public:
        Bot();
        int GetMove() const;
    };
}