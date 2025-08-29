#pragma once
#include "types.h"
#include <array>
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
		std::array<Bitboard, 64> LegalTargets{};
		std::array<Bitboard, 64> PromoMask{};
		
		const Chessboard* Board		= nullptr;

		Bitboard AttackedSquares	= 0;
		Bitboard CheckRays			= 0;
		Bitboard PinnedPieces		= 0;
		Bitboard AllPieces			= 0;
		Bitboard FriendlyPieces		= 0;
		Bitboard EnemyPieces		= 0;

		Square KingSquare			= Square::NoSquare;

		Color FriendlyColor			= White;
		Color EnemyColor			= Black;
		bool WhiteToMove			= true;
		CastlingRights CastleRights = No_Castling;

		CheckStatus Check_Status	= NoCheck;

	public:
		std::vector<Move> GenerateMoves(const Chessboard* board);

		[[deprecated("RunPerft deprecated. Users should implement their own Perft function.")]]
		static int RunPerft(...) { return 0; }
		
		bool IsLegalMove(Square from, Square to) const;
		bool IsPromotionMove(Square from, Square to) const;

	private:
		void Load(const Chessboard* board);

		void PerPiece(Square sq, Piece piece, std::vector<Move>& moves);
		void AddAttacks(Square sq, PieceType type, Bitboard Attacks, std::vector<Move>& moves);
		void AddMove(Square sq, Square to_sq, std::vector<MoveFlag> flags, std::vector<Move>& moves) const;
		void AddMove(Square sq, Square to_sq, MoveFlag flag, std::vector<Move>& moves) const;

		void CreateAttackedBitboard();
		void CreatePinnedBitboard();

		void AddPawnPushes(Square sq, std::vector<Move>& moves);
		void AddCastlingMoves(std::vector<Move>& moves);
		
		bool CanEPCapture() const;

		static Bitboard RookXRay(Square sq, Bitboard occupied, Bitboard blockers);
		static Bitboard BishopXRay(Square sq, Bitboard occupied, Bitboard blockers);
	};
}