#pragma once
#include "../../Chess/Source/Chessboard.h"
#include <fstream>
#include <filesystem>


namespace ChessTester
{
	static bool Test1()
    {
        namespace fs = std::filesystem;

        Chess_Old::Chessboard board;

        const auto moves = board.GetMoveList();
        const int expected[20] = {
            8457,
            9345,
            9272,
            11959,
            13134,
            8457,
            9345,
            8457,
            9329,
            9332,
            9744,
            12435,
            13160,
            8929,
            9328,
            9329,
            8885,
            9755,
            9748,
            8881
        };

        const auto log_folder = fs::path( "Logs" );

        std::ofstream out = std::ofstream( log_folder / "Test1_results.log", std::ios::trunc );

        bool passed = true;

        for ( int i = 0; i < 20; i++ )
        {
            const auto& move = moves[i];
            board.MakeMove( move );
            int result = board.Perft( 3 );
            board.UnMakeMove( move );

            out << std::format( "({2}) {0}: {1}\n", move.GetRepr(), result, result != expected[i] ? "Passed" : "Failed" );
        }
        return passed;
    }
}