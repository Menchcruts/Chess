#pragma once
#include "imgui.h"
#include "Chess.h"

void DrawChessboardScreen();
void DrawChessBoard(ImDrawList* DrawList, const Chess::Board& Board );
void HandleBoardClicks( Chess::Piece Piece, int CurrentSq );
void DrawPieceSelected( ImDrawList* DrawList );