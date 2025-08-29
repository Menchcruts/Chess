#include "ChessWindow.h"
#include "imgui.h"
#include "Chess/Chessboard_new.h"

#include <iostream>
#include <format>

ChessWindow::ChessWindow(
	std::string Name, 
	Chess::Chessboard_New& board, 
	std::function<void(Chess::Move)> MakeMoveFunc,
	std::function<void()> UnMakeMoveFunc,
	assets::ImageManager& images
) :
	Window(std::move(Name)), Board(board), MakeMove(MakeMoveFunc), UnMakeMove(UnMakeMoveFunc), Images(images)
{
	MoveHistory.reserve(8);
}

void ChessWindow::Draw()
{	
	MouseClicked	= ImGui::IsMouseClicked(ImGuiMouseButton_Left);
	MouseReleased	= ImGui::IsMouseReleased(ImGuiMouseButton_Left);
	LeftMouseDown	= ImGui::IsMouseDown(ImGuiMouseButton_Left);

	ImGui::Begin(WindowName.c_str());

	DrawBoard();

	HandleMoving();

	auto& moves = Board.GetMoves();
	ImGui::Text("Number of moves: %d", moves.size());

	std::string state_name = GetStateName(State);

	ImGui::Text("Hovered square: %d", HoveredSquare);
	ImGui::Text("Current state: %s", state_name.c_str());

	if (ImGui::Button("Unmake last move"))
	{
		UnMakeMove();
	}

	ImGui::End();
}

std::string ChessWindow::GetPieceImageName(Chess::Piece piece) const
{
	using namespace Chess;

	PieceType type = type_of(piece);
	Color color = color_of(piece);

	std::string file_name = color == White ? "w" : "b";

	switch (type)
	{
	case King:
		file_name += "k";
		break;
	case Pawn:
		file_name += "p";
		break;
	case Knight:
		file_name += "n";
		break;
	case Bishop:
		file_name += "b";
		break;
	case Rook:
		file_name += "r";
		break;
	case Queen:
		file_name += "q";
		break;
	default:
		break;
	}
	
	file_name += ".png";
	return file_name;
}

void ChessWindow::DrawBoard()
{
	using Chess::Piece, Chess::Square, Chess::Bitboards::is_legal;

	static bool SelectedAgain = false;

	float CellSide = 100.f;
	ImVec2 CellSize = ImVec2(CellSide, CellSide);

	bool MouseOverBoard = false;
	if (ImGui::BeginTable("Chessboard", 8))
	{
		for (int column = 0; column < 8; column++)
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, CellSide);

		for (int rank = 0; rank < 8; rank++)
		{
			ImGui::TableNextRow(0, CellSide);
			for (int file = 0; file < 8; file++)
			{
				int sq = (7 - rank) * 8 + file;	// We draw in reverse order of the ranks
				bool IsDark = ((rank + file) % 2) == 1;
				ImU32 cellColor = IsDark ? Colors.darkColor : Colors.lightColor;

				if (SelectedSquare == sq)
					cellColor = IsDark ? Colors.darkHLColor : Colors.lightHLColor;

				ImGui::TableNextColumn();
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cellColor);

				Piece piece = Board.GetPiece(Square(sq));
				PopulateCell(sq, CellSize, piece);

				if (ImGui::IsItemHovered())
				{
					HoveredSquare = sq;
					MouseOverBoard = true;
				}

				if (is_legal(Square(SelectedSquare), Square(sq)))
					DrawTargetCircle(HoveredSquare == sq, piece != Piece::NoPiece, CellSide);

				if (State == InputState::Promoting && sq == TargetSquare)
					DrawPromotionWindow(CellSize);
			}
		}

		ImGui::EndTable();
	}

	if (!MouseOverBoard)
		HoveredSquare = -1;
}

void ChessWindow::DrawPromotionWindow(ImVec2 CellSize)
{
	using Chess::Piece, Chess::PieceType;
	
	bool WhiteToPlay = Board.IsWhiteToMove();

	ImVec2 Cursor = ImGui::GetCursorScreenPos();
	auto flags =	ImGuiWindowFlags_NoCollapse | 
					ImGuiWindowFlags_NoMove | 
					ImGuiWindowFlags_NoResize | 
					ImGuiWindowFlags_NoTitleBar | 
					ImGuiWindowFlags_NoSavedSettings |
					ImGuiWindowFlags_NoScrollbar;
	
	float WindowSizeX = CellSize.x * 1.08f;
	float WindowSizeY = CellSize.y * 1.08f;
	ImVec2 WindowSize = ImVec2(WindowSizeX, WindowSizeY * 4);

	ImVec2 WindowPos = ImVec2(Cursor.x - CellSize.x * 0.04f, Cursor.y - CellSize.y * 1.06f);
	if (!WhiteToPlay)
		WindowPos.y -= WindowSize.y - CellSize.x * 1.04f;

	ImU32 White = IM_COL32(255, 255, 255, 255);

	ImGui::SetNextWindowSize(WindowSize);
	ImGui::SetNextWindowPos(WindowPos);
	
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.f, 0.f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));
	ImGui::PushStyleVar(ImGuiStyleVar_ImageBorderSize, 0.f);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, White);

	ImGui::Begin("PromotionWindow", nullptr, flags);

	auto queen = Images.get("Pieces://" + GetPieceImageName(WhiteToPlay ? Piece::W_Queen : Piece::B_Queen));
	auto rook = Images.get("Pieces://" + GetPieceImageName(WhiteToPlay ? Piece::W_Rook : Piece::B_Rook));
	auto bishop = Images.get("Pieces://" + GetPieceImageName(WhiteToPlay ? Piece::W_Bishop : Piece::B_Bishop));
	auto knight = Images.get("Pieces://" + GetPieceImageName(WhiteToPlay ? Piece::W_Knight : Piece::B_Knight));

	if (!ImGui::IsWindowFocused())
		State = InputState::PromotionCancel;

	float diff = 0.95f;
	float NewX = WindowSizeX * diff;
	ImVec2 ImgSize = ImVec2(NewX, NewX);

	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, White);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, White);
	ImGui::PushStyleColor(ImGuiCol_Button, White);

	if (WhiteToPlay)
	{
		assets::ImageManager::Ptr imgs[4] = {queen, rook, bishop, knight};
		PieceType types[4] = {PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight};
		
		for (int idx = 0; idx < 4; idx++)
			if (ImGui::ImageButton(std::format("img##{}", idx).c_str(), imgs[idx]->ImGuiID(), ImgSize))
				PromotionType = types[idx];
	}
	else
	{
		assets::ImageManager::Ptr imgs[4] = { knight, bishop, rook, queen };
		PieceType types[4] = { PieceType::Knight, PieceType::Bishop, PieceType::Rook, PieceType::Queen };

		for (int idx = 0; idx < 4; idx++)
			if (ImGui::ImageButton(std::format("img##{}", idx).c_str(), imgs[idx]->ImGuiID(), ImgSize))
				PromotionType = types[idx];
	}
	ImGui::PopStyleColor(3);

	ImGui::End();
	
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(5);
}

void ChessWindow::PopulateCell(int sq, ImVec2 Cellsize, Chess::Piece piece) const
{
	using Chess::Piece;

	bool SkipSquare = sq == SelectedSquare && (State == InputState::Dragging || State == InputState::Promoting);

	if (!SkipSquare && piece != Piece::NoPiece)
	{
		std::string image_path = GetPieceImageName(piece);
		assets::ImageManager::Ptr piece_image = Images.get("Pieces://" + image_path);
		ImGui::Image(piece_image->ImGuiID(), Cellsize);
	}
	else
	{
		ImGui::Dummy(Cellsize);
	}
}

void ChessWindow::DrawSelectedPiece() const
{
	std::string image_path = GetPieceImageName(PieceHeld);
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
	if (PieceOnSquare && !IsHovered)
	{
		float Thickness = 0.10f * CellSize;
		float Offset = (Thickness - 1) / 2;
		// We subtract the offset to compensate for circle thickness
		DrawList->AddCircle(CircleCenter, CellSize_Half - Offset, MoveCircle_Color, 0, Thickness);
		return;
	}

	float MoveCircle_Rad;
	if (PieceOnSquare && IsHovered)
		MoveCircle_Rad = CellSize_Half;
	else if (IsHovered)
		MoveCircle_Rad = 0.225 * CellSize;	// 22.5f
	else
		MoveCircle_Rad = 0.15 * CellSize;	// 15.0f

	DrawList->AddCircleFilled(CircleCenter, MoveCircle_Rad, MoveCircle_Color);
}

void ChessWindow::HandleMoving()
{
	switch (State)
	{
	case InputState::Idle:
		HandleMoving_Idle();
		break;
	case InputState::Selected:
		HandleMoving_Selected();
		break;
	case InputState::Dragging:
		HandleMoving_Dragging();
		break;
	case InputState::Promoting:
		HandleMoving_Promotion();
		break;
	case InputState::PromotionCancel:
		HandleMoving_PromotionCancel();
		break;
	default: break;
	}
}

void ChessWindow::HandleMoving_Idle()
{
	using Chess::Square, Chess::Piece;

	if (MouseClicked)
	{
		if (auto piece = Board.GetPiece(Square(HoveredSquare)); piece != Piece::NoPiece)
		{
			SelectedSquare = HoveredSquare;
			PieceHeld = piece;
			State = InputState::Dragging;
		}
	}
}

void ChessWindow::HandleMoving_Dragging()
{
	using Chess::Square, Chess::Piece, Chess::Move;
	using Chess::Bitboards::is_legal, Chess::Bitboards::is_promotion_move;

	DrawSelectedPiece();

	if (MouseReleased)
	{
		PieceHeld = Piece::NoPiece;
		if (is_legal(Square(SelectedSquare), Square(HoveredSquare)))
		{
			TargetSquare = HoveredSquare;
			SelectedAgain = false;
			if (is_promotion_move(Square(SelectedSquare), Square(TargetSquare)))
			{
				State = InputState::Promoting;
				return;
			}
			// Making a move
			Move move = Board.CreateMove(Square(SelectedSquare), Square(TargetSquare));
			MakeMove(move);

			SelectedSquare = TargetSquare = -1;
			State = InputState::Idle;
			return;
		}
		else if (SelectedAgain && SelectedSquare == HoveredSquare)
		{
			SelectedSquare = -1;
			SelectedAgain = false;
			State = InputState::Idle;
		}
		else
		{
			State = InputState::Selected;
		}
	}
}

void ChessWindow::HandleMoving_Selected()
{
	using Chess::Square, Chess::Piece, Chess::Move;
	using Chess::Bitboards::is_legal, Chess::Bitboards::is_promotion_move;

	if (MouseClicked)
	{
		SelectedAgain = false;
		if (HoveredSquare == SelectedSquare)
		{
			PieceHeld = Board.GetPiece(Square(SelectedSquare));
			SelectedAgain = true;
			State = InputState::Dragging;
			return;
		}
		if (is_legal(Square(SelectedSquare), Square(HoveredSquare)))
		{
			TargetSquare = HoveredSquare;
			SelectedAgain = false;
			if (is_promotion_move(Square(SelectedSquare), Square(TargetSquare)))
			{
				State = InputState::Promoting;
				return;
			}
			// Making a move
			Move move = Board.CreateMove(Square(SelectedSquare), Square(TargetSquare));
			MakeMove(move);

			SelectedSquare = TargetSquare = -1;
			State = InputState::Idle;
			return;
		}

		if (auto piece = Board.GetPiece(Square(HoveredSquare)); piece != Piece::NoPiece)
		{
			PieceHeld = piece;
			SelectedSquare = HoveredSquare;
			State = InputState::Dragging;
		}
		else
		{
			SelectedSquare = -1;
			State = InputState::Idle;
		}
	}
}

void ChessWindow::HandleMoving_Promotion()
{
	using Chess::Square, Chess::PieceType, Chess::Move;

	if (PromotionType != PieceType::NoPieceType)
	{		
		Move move = Board.CreateMove(Square(SelectedSquare), Square(TargetSquare), PromotionType);
		MakeMove(move);

		PromotionType = PieceType::NoPieceType;
		SelectedSquare = TargetSquare = -1;
		State = InputState::Idle;
	}
}

void ChessWindow::HandleMoving_PromotionCancel()
{
	PromotionType = Chess::PieceType::NoPieceType;
	SelectedSquare = TargetSquare = -1;
	State = InputState::Idle;
}

const char* ChessWindow::GetStateName(InputState state) const
{
	switch (state)
	{
	case ChessWindow::InputState::Idle:
		return "Idle";
	case ChessWindow::InputState::Selected:
		return "Selected";
	case ChessWindow::InputState::Dragging:
		return "Dragging";
	case ChessWindow::InputState::Promoting:
		return "Promoting";
	case ChessWindow::InputState::PromotionCancel:
		return "PromotionCancel";
	default:
		return "Unknonw";
	}
}
