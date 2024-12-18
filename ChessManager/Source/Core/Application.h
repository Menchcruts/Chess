#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Chess.h"
#include <chrono>
#include <array>
#include <unordered_set>
#include <unordered_map>
#include "Images/Images.h"

#include "Clock.h"
#include "Square.h"


using namespace std::chrono_literals;


class ChessApp
{
private:
	GLFWwindow* m_Window;

	bool m_EnableViewports = true;
	
	// Chessboard variables
	ImColor m_EvenColor = ImColor( 115, 149, 82 );			// 'Dark' color
	ImColor m_EvenColorHighlight = ImColor( 186, 203, 67 );	// 'Dark' color highlight
	ImColor m_EvenColorRed = ImColor( 211, 108, 80 );		// 'Dark' color red

	ImColor m_OddColor = ImColor( 235, 236, 208 );			// 'Light' color
	ImColor m_OddColorHighlight = ImColor( 246, 246, 130 );	// 'Light' color highlight
	ImColor m_OddColorRed = ImColor(235, 125, 106);			// 'Light' color red

	float m_CellSize = 100.f;
	Clock m_Clock;

	int m_SelectedSq = -1;
	bool m_SelectedPressed = false;
	Chess::Pieces::Piece m_PieceHeld{ };

	std::array<Square, 64> m_Board{ };

	std::unordered_set<int> m_SelectedSquares;

	std::array<std::unordered_map<Chess::PieceType, Image>, 2> m_PieceImages;

	ImFont* m_Gabarito;
	ImFont* m_Lexend;
	ImFont* m_Outfit;
	
private:
	void LoadFonts();
	void LoadPieceImages();
	void tempLoadBoard();

	void SetupDockspace();
	
	void DrawChessboardScreen();
	void DrawChessBoard( ImDrawList* DrawList );
	void HandleBoardClicks( Square Sq );
	void DrawPieceSelected(ImDrawList* DrawList) const;

	void DrawDebugScreen();

public:
	ChessApp();
	~ChessApp();

	void Run();
};