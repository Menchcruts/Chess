#pragma once
#include "Window.h"
#include "Chess/types.h"
#include "../asset_manager.hpp"
#include <string>


class TestWindow : public Window
{
public:
	TestWindow(std::string Name, Chess_Rework::Chessboard_New& board, assets::ImageManager& images);
	void Draw();

private:
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
	void DrawSelectedPiece(Chess_Rework::Piece piece) const;
	bool AttemptMakeMove(int sq1, int sq2);
	void HandlePieceMoving(int& SelectedSquare, bool& SelectedAgain, Chess_Rework::Piece& PieceHeld, int CurrentSq, Chess_Rework::Piece PieceOnSq);

private:
	WindowColors Colors;

	Chess_Rework::Chessboard_New& Board;
	assets::ImageManager& Images;
};