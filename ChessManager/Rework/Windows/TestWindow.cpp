#include "TestWindow.h"
#include "imgui.h"
#include "Chess/Chessboard_new.h"

#include <iostream>

TestWindow::TestWindow(std::string Name, Chess_Rework::Chessboard_New& board, assets::ImageManager& images) :
	Window(std::move(Name)), Board(board), Images(images)
{

}

void TestWindow::Draw()
{
	namespace Chess = Chess_Rework;
	using Chess::Piece, Chess::Square;
	
	static Piece PieceHeld = Piece::NoPiece;
	static int SelectedSquare = -1;
	static bool SelectedAgain = false;
	static int HoveredSquare = -1;

	ImVec2 Cellsize = ImVec2(100.f, 100.f);

	ImGui::Begin(WindowName.c_str());

	if (ImGui::BeginTable("Chessboard", 8))
	{
		for (int column = 0; column < 8; column++)
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);

		for (int rank = 0; rank < 8; rank++)
		{
			for (int file = 0; file < 8; file++)
			{
				int sq = (7 - rank) * 8 + file;
				bool IsDark = ((rank + file) % 2) == 1;
				ImU32 cellColor = IsDark ? Colors.darkColor : Colors.lightColor;

				if (SelectedSquare == sq)
					cellColor = IsDark ? Colors.darkHLColor : Colors.lightHLColor;

				ImGui::TableNextColumn();
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cellColor);

				Piece piece = Board.GetPiece(Square(sq));
				if ((sq != SelectedSquare || PieceHeld == Piece::NoPiece) && piece != Piece::NoPiece)
				{
					std::string image_path = GetPieceImageName(piece);
					assets::ImageManager::Ptr piece_image = Images.get("Pieces://" + image_path);
					ImGui::Image(piece_image->ImGuiID(), Cellsize);
				}
				else
				{
					ImGui::Dummy(Cellsize);
				}

				if (ImGui::IsItemHovered())
				{
					HoveredSquare = sq;
					HandlePieceMoving(SelectedSquare, SelectedAgain, PieceHeld, sq, piece);
				}
			}
		}

		ImGui::EndTable();
	}

	if (ImGui::IsMouseDown(0) && PieceHeld != Piece::NoPiece)
		DrawSelectedPiece(PieceHeld);

	ImGui::Text("Square selected: %d", SelectedSquare);
	ImGui::Text("Selected again: %s", SelectedAgain ? "True" : "False");
	ImGui::Text("Square hovered: %d", HoveredSquare);

	HoveredSquare = -1;

	ImGui::End();
}

std::string TestWindow::GetPieceImageName(Chess_Rework::Piece piece) const
{
	using namespace Chess_Rework;

	PieceType type = type_of(piece);
	Color color = color_of(piece);

	std::string file_name = color == White ? "w" : "b";

	switch (type)
	{
	case Chess_Rework::King:
		file_name += "k";
		break;
	case Chess_Rework::Pawn:
		file_name += "p";
		break;
	case Chess_Rework::Knight:
		file_name += "n";
		break;
	case Chess_Rework::Bishop:
		file_name += "b";
		break;
	case Chess_Rework::Rook:
		file_name += "r";
		break;
	case Chess_Rework::Queen:
		file_name += "q";
		break;
	default:
		break;
	}
	
	file_name += ".png";
	return file_name;
}

void TestWindow::DrawSelectedPiece(Chess_Rework::Piece piece) const
{
	std::string image_path = GetPieceImageName(piece);
	assets::ImageManager::Ptr piece_image = Images.get("Pieces://" + image_path);

	ImVec2 MousePos = ImGui::GetMousePosOnOpeningCurrentPopup();
	float half_cell = 50.f;

	ImVec2 UpLeft(MousePos.x - half_cell, MousePos.y - half_cell);
	ImVec2 DownRight(MousePos.x + half_cell, MousePos.y + half_cell);

	ImDrawList* drawlist = ImGui::GetWindowDrawList();
	drawlist->AddImage(piece_image->ImGuiID(), UpLeft, DownRight);
}

bool TestWindow::AttemptMakeMove(int sq1, int sq2)
{
	std::cout << "Attempted to make from " << sq1 << " to " << sq2 << "\n";
	return false;
}

void TestWindow::HandlePieceMoving(int& SelectedSquare, bool& SelectedAgain, Chess_Rework::Piece& PieceHeld, int CurrentSq, Chess_Rework::Piece PieceOnSq)
{
	using Chess_Rework::Piece;
	
	if (ImGui::IsMouseClicked(0))
	{
		if (SelectedSquare != -1 && CurrentSq != SelectedSquare)
		{
			if (PieceOnSq != Piece::NoPiece) { SelectedSquare = CurrentSq; SelectedAgain = false; PieceHeld = PieceOnSq; }
			else
			{
				bool success = AttemptMakeMove(SelectedSquare, CurrentSq);
				if (!success) { SelectedSquare = -1; PieceHeld = Piece::NoPiece; }
			}
		}
		else if (PieceOnSq != Piece::NoPiece)
		{
			if (CurrentSq == SelectedSquare) { SelectedAgain = true; }
			else { SelectedSquare = CurrentSq; }
			PieceHeld = PieceOnSq;
		}
		else { SelectedSquare = -1; PieceHeld = Piece::NoPiece; }
	}
	else if (ImGui::IsMouseReleased(0))
	{
		if (PieceHeld != Piece::NoPiece && SelectedSquare != CurrentSq && SelectedSquare != -1) { bool success = AttemptMakeMove(SelectedSquare, CurrentSq); }
		if (SelectedAgain && CurrentSq == SelectedSquare) { SelectedSquare = -1; SelectedAgain = false; }

		PieceHeld = Piece::NoPiece;
	}
}
