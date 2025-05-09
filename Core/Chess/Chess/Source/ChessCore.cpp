#include "ChessCore.h"

namespace Chess
{
    const char* GetSquareRepr( int Square )
    {
        return SquareReprs[Square].data();
    }
}
