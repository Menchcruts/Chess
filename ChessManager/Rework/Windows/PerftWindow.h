#pragma once
#include "Window.h"
#include <vector>
#include <unordered_map>
#include <thread>
#include <memory>
#include <atomic>
#include "Chess/types.h"
#include "Chess/Chessboard_new.h"


class PerftWindow : public Window
{
public:
	struct PerftResult
	{
		Chess::Chessboard Board;

		std::string FEN;

		std::unordered_map<Chess::Move, std::uint64_t> Breakdown{};

		std::atomic<std::uint64_t>  Nodes{ 0 };
		std::atomic<bool>			Running{ true };
	};

	struct Job
	{
		std::shared_ptr<PerftResult> result;
		std::jthread				 thread;
	};

public:
	PerftWindow(std::string name, const Chess::Chessboard& board) noexcept;
	~PerftWindow() noexcept;
	void Draw() noexcept;

private:
	void StartTest(int Depth);
	void DrawResults();
	void DrawResult(PerftResult& Result, int idx);

	static std::uint64_t perft(Chess::Chessboard& Board, int Depth, std::stop_token st);

private:
	std::vector<Job> Jobs;
	const Chess::Chessboard& Board;
};