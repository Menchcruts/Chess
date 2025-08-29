#pragma once
#include "types.h"
#include <vector>
#include <string>

namespace Chess
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
		const Chessboard* Board;

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
		MoveGenerator(const Chessboard* board);
		void GenerateMoves(std::vector<Move>& moves) const;
		
		[[deprecated("RunPerft deprecated. Users should implement their own Perft function.")]]
		static int RunPerft(...) { return 0; }
		
	private:
		void PerPiece(Square sq, Piece piece, std::vector<Move>& moves) const;
		void AddAttacks(Square sq, PieceType type, Bitboard Attacks, std::vector<Move>& moves) const;
		void AddMove(Square sq, Square to_sq, std::vector<MoveFlag> flags, std::vector<Move>& moves) const;
		void AddMove(Square sq, Square to_sq, MoveFlag flag, std::vector<Move>& moves) const;

		void CreateAttackedBitboard();
		void CreatePinnedBitboard();

		void AddPawnPushes(Square sq, std::vector<Move>& moves) const;
		void AddCastlingMoves(std::vector<Move>& moves) const;
		
		bool CanEPCapture() const;

		static Bitboard RookXRay(Square sq, Bitboard occupied, Bitboard blockers);
		static Bitboard BishopXRay(Square sq, Bitboard occupied, Bitboard blockers);
	};
}