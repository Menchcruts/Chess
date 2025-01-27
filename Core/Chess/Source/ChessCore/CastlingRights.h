#pragma once

namespace Chess
{
	enum class CastlingRights
	{
		None = 0,		// 00 - No castling rights
		Kingside = 1,	// 01 - Can castle king side
		QueenSide = 2,	// 10 - Can castle queen side
		Both = 3		// 11 - Can castle both ways
	};

	inline CastlingRights operator |( CastlingRights a, CastlingRights b )
	{
		return static_cast<CastlingRights>(static_cast<int>(a) | static_cast<int>(b));
	}
	inline CastlingRights operator &( CastlingRights a, CastlingRights b )
	{
		return static_cast<CastlingRights>(static_cast<int>(a) & static_cast<int>(b));
	}
	inline CastlingRights operator^( CastlingRights a, CastlingRights b )
	{
		return static_cast<CastlingRights>(static_cast<int>(a) ^ static_cast<int>(b));
	}

	inline CastlingRights& operator |=( CastlingRights& a, CastlingRights b )
	{
		a = a | b;
		return a;
	}
	inline CastlingRights& operator &=( CastlingRights& a, CastlingRights b )
	{
		a = a & b;
		return a;
	}
	inline CastlingRights& operator ^=( CastlingRights& a, CastlingRights b )
	{
		a = a ^ b;
		return a;
	}
}