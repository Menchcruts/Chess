#pragma once
#include "types.h"
#include "Chessboard_new.h"
#include "MoveGen.h"
#include <vector>
#include <unordered_map>
#include <iostream>


namespace Chess
{
	class Perft
	{
		struct PerftMoveResult
		{
			int expected;
			int calculated;
			bool passed;
		};
		
		struct PerftResult
		{
			std::unordered_map<Move, PerftMoveResult> move_results;
			int total_nodes = 0;
			bool passed = true;
		};
		
		static int RunPerft(Chessboard& board, int depth)
		{
			if (depth <= 0)
				return 1;

			std::vector<Move> moves;
			MoveGenerator generator(board);
			generator.GenMoves(moves);

			int result = 0;
			for (const auto& move : moves)
			{
				board.MakeMove(move);
				result += RunPerft(board, depth - 1);
				board.UnMakeMove(move);
			}
			return result;
		}

	public:
		static PerftResult PerftStartpos()
		{
			std::cout << "Running PerftStartpos...\n";

			Chessboard board;
			board.LoadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
			MoveGenerator generator(board);

			std::vector<Move> moves;
			generator.GenMoves(moves);

			int Expected[] = {
				181046,
				215255,
				222861,
				328511,
				402988,
				178889,
				217210,
				181044,
				217832,
				216145,
				240082,
				361790,
				405385,
				198473,
				214048,
				218829,
				198572,
				234656,
				233491,
				198502
			};

			int depth = 5;
			int move_result = 0;

			bool all_passed = true;

			PerftResult result;

			for (int idx = 0; idx < moves.size(); idx++)
			{
				auto expected = Expected[idx];
				auto move = moves[idx];
				board.MakeMove(move);
				move_result = RunPerft(board, depth - 1);
				board.UnMakeMove(move);
				result.move_results[move] = { .expected = expected, .calculated = move_result, .passed = move_result == expected };
				if (move_result != expected)
					all_passed = false;
			}

			result.passed = all_passed;

			return result;
		}
	};
}