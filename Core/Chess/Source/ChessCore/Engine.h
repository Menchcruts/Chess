#pragma once
#include "ChessCore.h"
#include "Board.h"
#include "Square.h"
#include <array>
#include <string>


namespace Chess
{
	enum class CastlingRights
	{
		None		= 0,	// 00 - No castling rights
		Kingside	= 1,	// 01 - Can castle king side
		QueenSide	= 2,	// 10 - Can castle queen side
		BOTH		= 3		// 11 - Can castle both ways
	};

	inline CastlingRights operator |( CastlingRights a, CastlingRights b );
	inline CastlingRights operator &( CastlingRights a, CastlingRights b );
	inline CastlingRights& operator |=( CastlingRights& a, CastlingRights b );
	inline CastlingRights& operator &=( CastlingRights& a, CastlingRights b );

	class Engine
	{
	private:
		Board m_Board;

		bool m_WhiteToPlay;

		int m_EnPassantSq;
		int m_FullmoveClock;
		int m_HalfmoveClock;

		CastlingRights m_WhiteCastling = CastlingRights::None;
		CastlingRights m_BlackCastling = CastlingRights::None;

	public:
		struct EngineInfo
		{
			bool WhiteToPlay;

			int EnPassantSquare;
			int FullmoveClock;
			int HalfmoveClock;

			CastlingRights WhiteCastling;
			CastlingRights BlackCastling;
		};

		/*typedef struct
		{
			bool WhiteToPlay;

			int EnPassantSquare;
			int FullmoveClock;
			int HalfmoveClock;

			CastlingRights WhiteCastling;
			CastlingRights BlackCastling;

		} EngineInfo;*/

		const Board& GetBoard() const;
		EngineInfo GetEngineInfo() const;

		void MakeMove( int Start, int Target );

		void LoadFEN( const std::string& FEN_Pos );
	};
}