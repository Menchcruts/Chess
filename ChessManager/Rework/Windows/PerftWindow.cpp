#include "PerftWindow.h"
#include <imgui.h>
#include <iostream>
#include <format>
#include "../../Assets/Fonts/Icons/IconsFontAwesome5Pro.h"

PerftWindow::PerftWindow(std::string name, const Chess::Chessboard_New& board) noexcept :
	Window(name), Board(board)
{

}

PerftWindow::~PerftWindow() noexcept
{
	for (auto& job : Jobs) job.thread.request_stop();
	Jobs.clear();
}

void PerftWindow::Draw() noexcept
{
	static int depth = 0;

	if (ImGui::Begin(WindowName.c_str()))
	{
		ImGui::SetNextItemWidth(80.f);
		if (ImGui::InputInt("##Depth", &depth))
		{
			if (depth < 0) depth = 0;
		}
		ImGui::SameLine();

		std::string button_label = std::format("Run at depth {}##RunPerft", depth);
		if (ImGui::Button(button_label.c_str()))
		{
			StartTest(depth);
		}

		std::string sep_label = std::format("{} Results##Results", Jobs.size());
		ImGui::SeparatorText(sep_label.c_str());
		if (ImGui::Button("Clear all"))
		{
			for (auto& job : Jobs) job.thread.request_stop();
			Jobs.clear();
		}
		if (ImGui::BeginChild("results_scroller"), ImVec2(0.f, 0.f), ImGuiChildFlags_Border)
		{
			DrawResults();
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void PerftWindow::StartTest(int Depth)
{
	auto result = std::make_shared<PerftResult>(Board, Board.ExportFEN());

	Jobs.emplace_back(
		result,
		std::jthread([result, Depth](std::stop_token st)
			{
				result->Nodes = perft(result->Board, Depth, st);
				result->Running = false;
			})
	);
}

void PerftWindow::DrawResults()
{
	for (size_t i = 0; i < Jobs.size(); i++)
	{
		DrawResult(*Jobs[i].result, int(i));
	}
}

void PerftWindow::DrawResult(PerftResult& Result, int idx)
{
	std::string label = std::format("Result #{}", idx + 1);
	ImGui::SeparatorText(label.c_str());

	ImGui::Text("FEN: %s", Result.FEN.c_str());
	ImGui::SameLine();

	float font_size = ImGui::GetFontSize();

	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.f);
	
	ImGui::PushFont(nullptr, font_size*1.1f);
	if (ImGui::SmallButton(std::format("Copy {}##Copy{}", ICON_FA_COPY, idx).c_str()))
		ImGui::SetClipboardText(Result.FEN.c_str());
	ImGui::PopFont();
	
	ImGui::PopStyleVar(1);

	if (Result.Running.load())
	{
		if (ImGui::Button(std::format("Cancel##{}", idx).c_str()))
		{
			Jobs[idx].thread.request_stop();
		}
		ImGui::SameLine();
		ImGui::TextUnformatted("Perft test running...");
	}
	else
	{
		ImGui::Text("Nodes: %I64u", Result.Nodes.load());
	}
}

std::uint64_t PerftWindow::perft(Chess::Chessboard_New& Board, int Depth, std::stop_token st)
{
	if (st.stop_requested())
		return 0;

	if (Depth <= 0)
		return 1;

	std::uint64_t nodes = 0;
	auto moves = Board.GetMoves();
	for (auto move : moves)
	{
		if (st.stop_requested()) break;
		Board.MakeMove(move);
		nodes += perft(Board, Depth - 1, st);
		Board.UnMakeMove(move);
	}
	return nodes;
}
