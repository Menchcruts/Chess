#include "ChessCore.h"

namespace Chess_Old
{
    const char* GetSquareRepr( int Square )
    {
        return SquareReprs[Square].data();
    }
}
