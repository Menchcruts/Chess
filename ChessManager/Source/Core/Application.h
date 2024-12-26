#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "irrKlang.h"

#include "Chess.h"
#include <chrono>
#include <array>
#include <unordered_set>
#include <unordered_map>
#include "Images/Images.h"

#include "Clock.h"


struct AppColors
{
	ImColor EvenColor{ 115, 149, 82 };			// 'Dark' color
	ImColor EvenColorHighlight{ 186, 203, 67 };	// 'Dark' color highlight
	ImColor EvenColorRed{ 211, 108, 80 };		// 'Dark' color red

	ImColor OddColor{ 235, 236, 208 };			// 'Light' color
	ImColor OddColorHighlight{ 246, 246, 130 };	// 'Light' color highlight
	ImColor OddColorRed{ 235, 125, 106 };		// 'Light' color red
};

struct BoardSettings
{
	float CellSize = 100.0f;
	int SelectedSquare = -1;
	bool SelectedPressed = false;
	Chess::Piece PieceHeld{ };
	std::unordered_set<int> HighlightedSquares;
	int LastMove[2] = { -1, -1 };
	char NewFen[128] = "";
	Chess::Engine::EngineInfo EngineInfo;
};

struct AppFonts
{
	ImFont* Gabarito;
	ImFont* Lexend;
	ImFont* Outfit;
};


class ChessApp
{
private:
	GLFWwindow* m_Window;
	irrklang::ISoundEngine* m_SoundEngine;

	bool m_EnableViewports = true;
	
	Clock m_Clock;
	// Chessboard variables
	AppColors m_Colors;

	BoardSettings m_BoardSettings;

	Chess::Engine m_Engine;
	
	std::array<std::unordered_map<Chess::PieceType, Image>, 2> m_PieceImages;

	AppFonts m_Fonts;

private:
	void LoadFonts();
	void LoadPieceImages();

	void SetupDockspace();
	
	void DrawChessboardScreen();
	void DrawChessBoard( ImDrawList* DrawList );
	void HandleBoardClicks( Chess::Piece Piece, int CurrentSq );
	void DrawPieceSelected(ImDrawList* DrawList) const;
	void MakeMove(int Start, int Target);

	void DrawDebugScreen();

public:
	ChessApp();
	~ChessApp();

	void Run();
};