#include "ChessWindow.h"
#include "imgui.h"
#include "Chess/Chessboard_new.h"

#include <iostream>

ChessWindow::ChessWindow(std::string Name, Chess_Rework::Chessboard_New& board, assets::ImageManager& images) :
	Window(std::move(Name)), Board(board), Images(images)
{

}

void ChessWindow::Draw()
{
	namespace Chess = Chess_Rework;
	using Chess::Piece, Chess::Square, Chess::Bitboards::is_legal;
	
	static Piece PieceHeld = Piece::NoPiece;
	static int SelectedSquare = -1;
	static bool SelectedAgain = false;
	static int HoveredSquare = -1;

	float CellSide = 100.f;
	ImVec2 Cellsize = ImVec2(CellSide, CellSide);
	ImVec2 Cellsize_Half = ImVec2(CellSide/2, CellSide/2);

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

				bool SquareIsHovered = false;
				if (ImGui::IsItemHovered())
				{
					HoveredSquare = sq;
					SquareIsHovered = true;
					HandlePieceMoving(SelectedSquare, SelectedAgain, PieceHeld, sq, piece);
				}

				if (SelectedSquare != -1 && is_legal(Square(SelectedSquare), Square(sq)))
				{
					std::cout << "Drawing circle\n";
					DrawTargetCircle(SquareIsHovered, piece != Piece::NoPiece, CellSide);
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

std::string ChessWindow::GetPieceImageName(Chess_Rework::Piece piece) const
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

void ChessWindow::DrawSelectedPiece(Chess_Rework::Piece piece) const
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

void ChessWindow::DrawTargetCircle(bool IsHovered, bool PieceOnSquare, float CellSize) const
{
	ImDrawList* DrawList = ImGui::GetWindowDrawList();

	float CellSize_Half = CellSize / 2;

	ImVec2 CursorPos = ImGui::GetCursorScreenPos();
	CursorPos.y -= 4.0f;
	ImVec2 CircleCenter = ImVec2(CursorPos.x + CellSize_Half, CursorPos.y - CellSize_Half);

	ImColor MoveCircle_Color = IM_COL32(255, 0, 0, 127);
	float MoveCircle_Rad;
	if (IsHovered)
		MoveCircle_Rad = 21.f;
	else
		MoveCircle_Rad = 15.f;

	if (PieceOnSquare)
	{
		if (IsHovered)
		{
			MoveCircle_Rad = CellSize_Half - 4.f; // Compensate for circle width
			DrawList->AddCircle(CircleCenter, MoveCircle_Rad, MoveCircle_Color, 0, 9.f);
			return;
		}
		else
		{
			MoveCircle_Rad = CellSize_Half;
		}
	}
	DrawList->AddCircleFilled(CircleCenter, MoveCircle_Rad, MoveCircle_Color);
}

bool ChessWindow::AttemptMakeMove(int sq1, int sq2)
{
	std::cout << "Attempted to make from " << sq1 << " to " << sq2 << "\n";
	return false;
}

void ChessWindow::HandlePieceMoving(int& SelectedSquare, bool& SelectedAgain, Chess_Rework::Piece& PieceHeld, int CurrentSq, Chess_Rework::Piece PieceOnSq)
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
