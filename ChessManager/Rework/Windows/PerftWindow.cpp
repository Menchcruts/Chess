#include "PerftWindow.h"
#include <imgui.h>
#include <iostream>
#include <format>
#include <bitset>
#include "../../Assets/Fonts/Icons/IconsFontAwesome5Pro.h"

PerftWindow::PerftWindow(std::string name, const Chess::Chessboard& board) noexcept :
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
		std::string button2_label = std::format("Run perft 1-5##RunPerftMany", depth);
		if (ImGui::Button(button_label.c_str()))
		{
			StartTest(depth);
		}
		
		if (ImGui::Button(button2_label.c_str()))
		{
			for (int i = 1; i < 6; i++)
				StartTest(i);
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

	auto func = [result, Depth](std::stop_token st)
		{
			if (Depth <= 0)
			{
				result->Nodes = 1;
				result->Running = false;
				return;
			}

			auto moves = result->Board.GetMoves();
			for (auto& move : moves)
			{
				std::uint64_t nodes = 0;

				if (st.stop_requested()) break;

				result->Board.MakeMove(move);
				nodes = perft(result->Board, Depth - 1, st);
				result->Nodes += nodes;
				result->Board.UnMakeMove(move);

				result->Breakdown.emplace(move, nodes);
			}
			result->Running = false;
		};

	Jobs.emplace_back(
		result,
		std::jthread(func)
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

	ImGui::PushFont(nullptr, font_size * 1.1f);
	if (ImGui::SmallButton(std::format("Copy {}##Copy{}", ICON_FA_COPY, idx).c_str()))
		ImGui::SetClipboardText(Result.FEN.c_str());
	ImGui::PopFont();

	ImGui::PopStyleVar(1);

	if (Result.Running.load())	// Perft still running
	{
		if (ImGui::Button(std::format("Cancel##{}", idx).c_str()))
		{
			Jobs[idx].thread.request_stop();
		}
		ImGui::SameLine();
		ImGui::TextUnformatted("Perft test running...");
	}
	else // Draw results
	{
		ImGui::Text("Total nodes: %I64u", Result.Nodes.load());
		std::string header_label = std::format("Per move breakdown##breakdown{}", idx);
		if (ImGui::CollapsingHeader(header_label.c_str()))
		{
			std::string table_label = std::format("BreakdownTable##BreakdownTable{}", idx);
			std::string copy_label = std::format("Copy breakdown##CopyBreakdownTable{}", idx);
			if (ImGui::SmallButton(copy_label.c_str()))
			{
				std::string result = "";
				for (auto& [move, nodes] : Result.Breakdown)
				{
					result += std::format("{}: {}\n", Chess::GetMoveRepr(move), nodes);
				}
				ImGui::SetClipboardText(result.c_str());
			}

			if (ImGui::BeginTable(table_label.c_str(), 5))
			{
				ImGui::TableSetupColumn("Move");
				ImGui::TableSetupColumn("Nodes");
				ImGui::TableSetupColumn("From");
				ImGui::TableSetupColumn("To");
				ImGui::TableSetupColumn("Flag");

				ImGui::TableHeadersRow();
				
				ImGui::TableNextRow();
				for (auto& [move, nodes] : Result.Breakdown)
				{
					ImGui::TableNextColumn();
					ImGui::Text("%s", Chess::GetMoveRepr(move).c_str());

					ImGui::TableNextColumn();
					ImGui::Text("%I64u", nodes);

					ImGui::TableNextColumn();
					ImGui::Text("%d", Chess::from_square(move));

					ImGui::TableNextColumn();
					ImGui::Text("%d", Chess::to_square(move));

					ImGui::TableNextColumn();
					ImGui::Text("%s", std::bitset<4>(Chess::move_flag(move)).to_string().c_str());
				}

				ImGui::EndTable();
			}

		}
	}
}

std::uint64_t PerftWindow::perft(Chess::Chessboard& Board, int Depth, std::stop_token st)
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
