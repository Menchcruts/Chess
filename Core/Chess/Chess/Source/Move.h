#pragma once
#include <functional>
#include <cstdint>
#include <string>

namespace Chess_Old
{
	enum class MoveFlag : unsigned char
	{
		None					= 0b0000,
		DoublePawnMove			= 0b0001,

		CastleKing				= 0b0010,
		CastleQueen				= 0b0011,

		Capture					= 0b0100,
		EnPassant				= 0b0101,

		PromoteKnight			= 0b1000,
		PromoteBishop			= 0b1001,
		PromoteRook				= 0b1010,
		PromoteQueen			= 0b1011,

		PromoteKnightCapture	= PromoteKnight | Capture,
		PromoteBishopCapture	= PromoteBishop | Capture,
		PromoteRookCapture		= PromoteRook	| Capture,
		PromoteQueenCapture		= PromoteQueen	| Capture
	};

	constexpr inline MoveFlag operator |( MoveFlag right, MoveFlag left )
	{
		return static_cast<MoveFlag>(static_cast<int>(right) | static_cast<int>(left));
	}
	constexpr inline MoveFlag operator &( MoveFlag right, MoveFlag left )
	{
		return static_cast<MoveFlag>(static_cast<int>(right) & static_cast<int>(left));
	}
	constexpr inline MoveFlag& operator |=( MoveFlag& right, MoveFlag left )
	{
		right = right | left;
		return right;
	}
	constexpr inline MoveFlag& operator &=( MoveFlag& right, MoveFlag left )
	{
		right = right & left;
		return right;
	}


	/*
	0000 000000 000000
	Flag Target Start 
	*/
	struct Move
	{
		uint16_t MoveInfo = 0;

		Move() = default;
		Move( int start, int target, MoveFlag flag = MoveFlag::None );

		int Start() const;
		int Target() const;
		MoveFlag Flag() const;

		bool IsCapture() const;
		bool IsEnPassant() const;
		bool IsCastle() const;
		bool IsPromotion() const;

		void Print() const;
		std::string GetRepr() const;

		bool IsNullMove() const;
		bool operator==( const Move& other ) const;
	};
}

template<>
struct std::hash<Chess_Old::Move>
{
	std::size_t operator()( const Chess_Old::Move& move ) const noexcept
	{
		return std::hash<uint16_t>()(move.MoveInfo);
	}
};