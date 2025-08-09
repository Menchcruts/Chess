#include "TestWindow.h"
#include "imgui.h"
#include "Chess/Chessboard_new.h"

TestWindow::TestWindow(const char* Name, Chess_Rework::Chessboard_New& board) :
	Window(Name), Board(board)
{

}

void TestWindow::Draw()
{
	namespace Chess = Chess_Rework;
	using Chess::Piece, Chess::Square;
	
	ImGui::Begin(WindowName);

	if (ImGui::BeginTable("Chessboard", 8))
	{
		for (int column = 0; column < 8; column++)
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);

		for (int rank = 0; rank < 8; rank++)
		{
			for (int file = 0; file < 8; file++)
			{
				int sq = (7 - rank) * 8 + file;

				Piece piece = Board.GetPiece(Square(sq));
			}
		}

		ImGui::EndTable();
	}

	ImGui::End();
}
