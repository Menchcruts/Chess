#include "Source/Application.h"
#include "Chess/Perft.h"
#include "Chess/Bitboards.h"
#include <iostream>

static std::string GetMoveRepr(Chess_Rework::Move move)
{
	using namespace Chess_Rework;
	
	const char* files = "abcdefgh";
	const char* ranks = "12345678";

	short start = from_square(move);
	short start_rank = start >> 3;
	short start_file = start & 7;

	short target = to_square(move);
	short target_rank = target >> 3;
	short target_file = target & 7;

	std::string result = { files[start_file], ranks[start_rank], files[target_file], ranks[target_rank] };

	MoveFlag flag = move_flag(move);
	if ((flag & MoveFlag::PromoteKnight) != MoveFlag::None)
	{
		if (flag == MoveFlag::PromoteKnight || flag == MoveFlag::PromoteKnightCapture)
			result.push_back('n');
		else if (flag == MoveFlag::PromoteBishop || flag == MoveFlag::PromoteBishopCapture)
			result.push_back('b');
		else if (flag == MoveFlag::PromoteRook || flag == MoveFlag::PromoteRookCapture)
			result.push_back('r');
		else if (flag == MoveFlag::PromoteQueen || flag == MoveFlag::PromoteQueenCapture)
			result.push_back('q');
	}
	return result;
}

int main()
{
    Chess_Rework::Bitboards::init();
    auto result = Chess_Rework::Perft::PerftStartpos();
    std::cout << "Test " << (result.passed ? "passed" : "failed") << "\n";
    for (const auto& [move, move_result] : result.move_results)
		std::cout << GetMoveRepr(move) << ": " << move_result.calculated << " / " << move_result.expected << " - " << (move_result.passed ? "Passed" : "Failed") << "\n";

    //ChessApp App;
    //App.Minimal();
}