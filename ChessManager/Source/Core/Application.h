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

#include "Clock/Clock.h"


class ChessApp
{
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
		int NewWhiteTime = 0;
		int NewBlackTime = 0;
		float CellSize = 100.0f;
		int SelectedSquare = -1;
		Chess::ChessPiece PieceHeld{ };
		bool SelectedPressed = false;
		bool FlipBoard = false;
		bool AutoFlip = false;
		bool GameStarted = false;
		std::unordered_set<int> HighlightedSquares;
		char NewFen[128] = "";
		//Chess::Engine::EngineInfo EngineInfo;
		Chess::Chessboard::BoardInfo BoardInfo;
	};

	struct AppFonts
	{
		ImFont* Gabarito = nullptr;
		ImFont* Lexend = nullptr;
		ImFont* Outfit = nullptr;
	};

	struct AppSettings
	{
		AppColors Colors;
		AppFonts Fonts;
	};

private:
	GLFWwindow* m_Window;
	irrklang::ISoundEngine* m_SoundEngine;

	AppSettings Settings;

	bool m_EnableViewports = true;
	
	Clock m_WhiteClock;
	Clock m_BlackClock;

	BoardSettings m_BoardSettings;

	//Chess::Engine m_Engine;
	Chess::Chessboard m_Chessboard;
	
	std::array<std::unordered_map<Chess::PieceType, Image>, 2> m_PieceImages;

private:
	void LoadFonts();
	void LoadPieceImages();

	void SetupDockspace();
	
	void DrawChessboardScreen();
	void DrawChessBoard( );
	void DrawChessBoardFlipped();

	void HandleBoardClicks( Chess::ChessPiece Piece, int CurrentSq );
	void DrawPieceSelected(ImDrawList* DrawList) const;
	void MakeMove( int Start, int Target, Chess::MoveFlag flag = Chess::MoveFlag::None );

	void DrawDebugScreen();
	void ResetBoard();

public:
	ChessApp();
	~ChessApp();

	void Run();
};