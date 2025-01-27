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

	struct AppFonts
	{
		ImFont* Noto_Sans = nullptr;
		ImFont* Gabarito = nullptr;
		ImFont* Lexend = nullptr;
		ImFont* Outfit = nullptr;
	};

	struct AppSettings
	{
		AppColors Colors;
		AppFonts Fonts;
	};


	struct MoveHandling
	{
		short SelectedSquare = -1;
		bool SelectedPressed = false;
		Chess::ChessPiece PieceHeld = Chess::ChessPiece();
	};

	struct BoardVariables
	{
		Chess::Chessboard::BoardInfo BoardInfo;
		char NewFen[128] = "";
		int NewWhiteTime = 0;
		int NewBlackTime = 0;
		Chess::Move NextMove;
		bool GameStarted = false;
	};

	struct BoardVisuals
	{
		std::unordered_set<int> HighlightedSquares;
		float CellSize = 100.0f;
		Chess::Move LastMove;
		bool FlipBoard = false;
		bool AutoFlip = false;
	};

	struct PromotionHandling
	{
		bool Promoting = false;
		ImVec2 PromotionScreenPos;
		Chess::PieceType PromotionType = Chess::PieceType::None;
	};

	struct LegalMovesInfo
	{
		std::unordered_set<Chess::Move> Moves;
		std::unordered_set<int> Targets;
	};

private:
	GLFWwindow* m_Window;
	irrklang::ISoundEngine* m_SoundEngine;

	AppSettings Settings;

	bool m_EnableViewports = true;
	
	Clock m_WhiteClock;
	Clock m_BlackClock;

	BoardVariables m_BoardVariables;
	BoardVisuals m_BoardVisuals;
	MoveHandling m_MoveHandling;
	PromotionHandling m_PromotionHandling;

	std::unordered_map<int, LegalMovesInfo> m_LegalMovesDict;	// Moves are mapped with start -> set of moves

	Chess::Chessboard m_Chessboard/* = Chess::Chessboard( "R1r4k/6b1/8/4Q3/2N5/2K5/8/8 w - - 0 1" )*/;
	
	std::array<std::unordered_map<Chess::PieceType, Image>, 2> m_PieceImages;

private:
	void LoadFonts();
	void LoadPieceImages();

	void SetupDockspace();
	
	void DrawChessboardScreen();
	void DrawChessBoard( );
	void DrawChessBoardFlipped();
	void DrawPieceSelected(ImDrawList* DrawList) const;
	void DrawLegalTargets( ImDrawList* DrawList ) const;

	void HandleBoardClicks( Chess::ChessPiece Piece, int CurrentSq );
	Chess::Move IsValidMove( int Start, int Target ) const;
	void PromoteScreen( );
	void MakeMove( );
	void UnMakeMove();

	void DrawDebugScreen();
	void ResetBoard();

	void UpdateBoardInfo();
	void CreateMoveDict();

public:
	ChessApp();
	~ChessApp();

	void Run();
};