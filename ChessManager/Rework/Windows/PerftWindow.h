#pragma once
#include "Window.h"
#include <vector>
#include <unordered_map>
#include <thread>
#include <memory>
#include <atomic>
#include <chrono>
#include "Chess/types.h"
#include "Chess/Chessboard_new.h"


class PerftWindow : public Window
{
public:
	struct PerftResult
	{
		using Clock = std::chrono::system_clock;

		std::unordered_map<Chess::Move, std::uint64_t> Breakdown{};
		std::string FEN;

		Clock::time_point StartTime{};
		Clock::time_point EndTime{};

		int Depth = 0;

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