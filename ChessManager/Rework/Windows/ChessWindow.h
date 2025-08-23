#pragma once
#include "Window.h"
#include "Chess/types.h"
#include "../asset_manager.hpp"
#include <string>
#include <vector>

class ChessWindow : public Window
{
public:
	ChessWindow(std::string Name, Chess_Rework::Chessboard_New& board, assets::ImageManager& images);
	void Draw();

private:
	enum class InputState { Idle, Selected, Dragging, Promoting, PromotionCancel };
	
	struct WindowColors
	{
		ImU32 lightColor = IM_COL32(235, 236, 208, 255);
		ImU32 lightRedColor = IM_COL32(235, 125, 106, 255);

		ImU32 darkColor = IM_COL32(115, 149, 82, 255);
		ImU32 darkRedColor = IM_COL32(211, 108, 80, 255);

		ImU32 lightHLColor = IM_COL32(246, 246, 130, 255);
		ImU32 darkHLColor = IM_COL32(186, 203, 67, 255);
	};

private:
	std::string GetPieceImageName(Chess_Rework::Piece piece) const;
	
	void DrawBoard();
	void PopulateCell(int sq, ImVec2 Cellsize, Chess_Rework::Piece piece) const;
	void DrawSelectedPiece() const;
	void DrawTargetCircle(bool IsHovered, bool PieceOnSquare, float CellSize) const;
	void DrawPromotionWindow(ImVec2 CellSize);

	void HandleMoving();
	void HandleMoving_Idle();
	void HandleMoving_Dragging();
	void HandleMoving_Selected();
	void HandleMoving_Promotion();
	void HandleMoving_PromotionCancel();
	const char* GetStateName(InputState state) const;

private:
	std::vector<Chess_Rework::Move> MoveHistory;

	WindowColors Colors;

	Chess_Rework::Chessboard_New& Board;
	assets::ImageManager& Images;

	InputState State = InputState::Idle;

	int SelectedSquare = -1;
	int HoveredSquare = -1;
	int TargetSquare = -1;
	Chess_Rework::Piece PieceHeld = Chess_Rework::Piece::NoPiece;
	Chess_Rework::PieceType PromotionType = Chess_Rework::PieceType::NoPieceType;
	bool SelectedAgain	= false;
	bool MouseClicked	= false;
	bool MouseReleased	= false;
	bool LeftMouseDown	= false;
};