#include "Core/Application.h"
#include "Chess.h"
#include "ChessCore/Stopwatch/Stopwatch.h"
#include <iostream>

int main()
{
    /*ChessApp App;
    App.Run();*/

    std::cout << "\n\n\nRunning Perft benchmark...\n";
    Chess::Chessboard Board;
    auto temp = StopWatch( "Perft" );
    int n_nodes = Board.RunPerft( 5 );
    std::cout << "Perft result: " << n_nodes << " nodes\n";
}