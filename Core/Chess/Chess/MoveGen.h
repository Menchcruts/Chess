#pragma once
#include "types.h"
#include <vector>
#include <string>

namespace Chess_Rework
{
	class MoveGenerator
	{
		enum CheckStatus : std::uint8_t
		{
			NoCheck,
			Check,
			DoubleCheck
		};

		friend inline constexpr CheckStatus operator |(CheckStatus lhs, CheckStatus rhs) { return CheckStatus(int(lhs) | int(rhs)); }
		friend inline constexpr CheckStatus& operator |=(CheckStatus& lhs, CheckStatus rhs) { return lhs = lhs | rhs; }

	private:
		const Chessboard_New& Board;

		Bitboard AttackedSquares;
		Bitboard CheckRays;
		Bitboard PinnedPieces;
		Bitboard AllPieces;
		Bitboard FriendlyPieces;
		Bitboard EnemyPieces;

		Square KingSquare;

		Color FriendlyColor;
		Color EnemyColor;
		bool WhiteToMove;
		CastlingRights CastleRights;

		CheckStatus Check_Status;

	public:
		MoveGenerator(const Chessboard_New& board);
		void GenMoves(std::vector<Move>& moves) const;
		
		static void GenerateMoves(const Chessboard_New& board, std::vector<Move>& moves);
		static int RunPerft(const std::string& FEN, int depth);
	private:
		void CreateAttackedBitboard();
		void CreatePinnedBitboard();
		void AddPawnPushes(Square sq, std::vector<Move>& moves) const;
		void AddCastlingMoves(std::vector<Move>& moves) const;
		bool CanEPCapture() const;

		static Bitboard GetAttackedSquares(const Chessboard_New& board, Color enemy_color, Square king_sq, CheckStatus& check_status, Bitboard& check_rays);
		static Bitboard GetPinnedPieces(const Chessboard_New& board, Color friendly_color, Square king_sq);
		static Bitboard RookXRay(Square sq, Bitboard occupied, Bitboard blockers);
		static Bitboard BishopXRay(Square sq, Bitboard occupied, Bitboard blockers);

		static int Perft(Chessboard_New& board, int depth);
	};
}