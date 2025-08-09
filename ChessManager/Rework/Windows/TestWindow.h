#pragma once
#include "Window.h"
#include "Chess/types.h"

class TestWindow : public Window
{
private:
	Chess_Rework::Chessboard_New& Board;

public:
	TestWindow(const char* Name, Chess_Rework::Chessboard_New& board);

	void Draw();
};