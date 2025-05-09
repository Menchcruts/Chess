//#include "ChessTester.h"
//#include "../../Chess/Source/Chessboard.h"
//#include <fstream>
//#include <filesystem>
//
//
//bool ChessTester::Test1()
//{
//    namespace fs = std::filesystem;
//
//    Chess::Chessboard board;
//    
//    const auto moves = board.GetMoveList();
//    const int expected[20] = {};
//
//    const auto log_folder = fs::path( "Logs" );
//
//    std::ofstream out = std::ofstream( log_folder / "Test1_results.log", std::ios::trunc );
//
//    bool passed = true;
//
//    for ( int i = 0; i < 20; i++ )
//    {
//        const auto& move = moves[i];
//        board.MakeMove( move );
//        int result = board.RunPerft( 3 );
//        board.UnMakeMove( move );
//
//        out << std::format( "{}: {} - {}", move.GetRepr(), result, result != expected[i] ? "Passed" : "Failed" );
//    }
//    return passed;
//}