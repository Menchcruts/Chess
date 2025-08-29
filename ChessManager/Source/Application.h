#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "miniaudio.h"

#include "Chess/Chess.h"
#include <chrono>
#include <array>
#include <unordered_set>
#include <unordered_map>
#include <thread>
#include <memory>
#include <functional>

#include "Images/Images.h"
#include "Utils/Logger.h"

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
		Chess_Old::ChessPiece PieceHeld = Chess_Old::ChessPiece();
	};

	struct BoardVariables
	{
		char NewFen[128] = "";
		std::array<Chess_Old::Move, 218> LegalMoves;
		std::vector<Chess_Old::Move> MoveHistory;
		int NewWhiteTime = 0;
		int NewBlackTime = 0;
		int NumberOfLegalMoves = 0;
		Chess_Old::Move NextMove;
		bool GameStarted = false;
	};

	struct BoardVisuals
	{
		Chess_Old::Bitboard HighlightedSquares;
		float CellSize = 100.0f;
		Chess_Old::Move LastMove;
		bool FlipBoard = false;
		bool AutoFlip = false;
	};

	struct PromotionHandling
	{
		bool Promoting = false;
		ImVec2 PromotionScreenPos;
		Chess_Old::PieceType PromotionType = Chess_Old::PieceType::None;
	};

	struct LegalMovesInfo
	{
		std::unordered_set<Chess_Old::Move> Moves;
		std::unordered_set<int> Targets;
	};

	struct PerftResult
	{
		int Nodes = 0;
		int Captures = 0;
		int EnPassants = 0;
		int Castles = 0;
		int Promotions = 0;
	};

	struct PerftSettings
	{
		std::thread PerftThread;
		PerftResult Result;

		int ExpectedResult = 0;
		int InitalDepth = 1;

		bool Running = false;
		bool Finished = false;
		bool ShowScreen = false;
		bool CancelSearch = false;
		bool Verbose = true;
	};

	struct BitboardSettings
	{
		Chess_Old::Bitboard Result;
		bool ShowScreen = false;

		bool WhiteKing = false;
		bool WhitePawn = false;
		bool WhiteKnight = false;
		bool WhiteBishop = false;
		bool WhiteRook = false;
		bool WhiteQueen = false;

		bool BlackKing = false;
		bool BlackPawn = false;
		bool BlackKnight = false;
		bool BlackBishop = false;
		bool BlackRook = false;
		bool BlackQueen = false;

		bool AllWhite = false;
		bool AllBlack = false;
	};

private:
	GLFWwindow* m_Window;

	AppSettings* Settings;

	Clock m_WhiteClock;
	Clock m_BlackClock;

	BoardVariables* m_BoardVariables;
	BoardVisuals m_BoardVisuals;
	PromotionHandling m_PromotionHandling;

	PerftSettings m_PerftSettings;
	BitboardSettings m_BitboardSettings;

	std::unordered_map<int, LegalMovesInfo> m_LegalMovesDict;	// Moves are mapped with start -> set of moves

	Chess_Old::Chessboard m_Chessboard;
	
	std::array<std::unordered_map<Chess_Old::PieceType, Image>, 2> m_PieceImages;

	std::unique_ptr<ma_engine> miniaudio_engine;

	Logger m_logger = Logger( "App.log", Logger::Debug );

	std::thread m_TestThread;

	MoveHandling m_MoveHandling;

	ma_result miniaudio_result;
	bool m_EnableViewports = true;
	bool m_TestFinished = false;
	bool m_TestRunning = false;

private:
	void LoadFonts();
	void LoadPieceImages();

	void SetupDockspace();
	void PreFrame();
	void PostFrame();
	void Draw();

	void DrawMinimal();
	void LookupTableTests();
	void MoveTableTests();
	void ChessboardTests();
	void MoveGenTests();

	void DrawChessboardScreen();
	void DrawChessBoard( );
	void DrawPieceSelected(ImDrawList* DrawList) const;
	void DrawLegalTargets( ImDrawList* DrawList ) const;

	void HandleBoardClicks( Chess_Old::ChessPiece Piece, int CurrentSq );
	Chess_Old::Move IsValidMove( int Start, int Target ) const;
	void PromoteScreen( );
	void MakeMove( );
	void UnMakeMove();

	void DrawDebugScreen();
	void ResetBoard();

	void UpdateBoardInfo();
	void HandleMoveList();

	void DrawBitboardScreen();

	void StartPerftTest();
	int PerftTest( Chess_Old::Chessboard& Board, int Depth, bool* Cancel, bool Verbose, PerftResult* Result );
	void DrawPerftScreen();

	template<typename T>
	void RunTest( T(*test)() );

public:
	ChessApp();
	~ChessApp();

	void Run();
	void Minimal();
};