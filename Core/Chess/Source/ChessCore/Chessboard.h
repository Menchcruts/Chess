#pragma once
#include <array>
#include <string>
#include <vector>
#include <unordered_set>
#include "Bitboard.h"
#include "Pieces.h"
#include "Move.h"


namespace Chess
{
	enum class CastlingRights
	{
		None = 0,		// 00 - No castling rights
		Kingside = 1,	// 01 - Can castle king side
		QueenSide = 2,	// 10 - Can castle queen side
		Both = 3		// 11 - Can castle both ways
	};

	CastlingRights operator |( CastlingRights a, CastlingRights b );
	CastlingRights operator &( CastlingRights a, CastlingRights b );
	CastlingRights operator ^( CastlingRights a, CastlingRights b );
	CastlingRights& operator |=( CastlingRights& a, CastlingRights b );
	CastlingRights& operator &=( CastlingRights& a, CastlingRights b );
	CastlingRights& operator ^=( CastlingRights& a, CastlingRights b );


	class Chessboard
	{
	public:
		struct BoardInfo
		{
			std::unordered_set<Move> _LegalMoves;
			Bitboards _Bitboards{ };
			std::array<ChessPiece, 64> _Board{ };
			bool _WhiteToPlay = true;
			int _EnPassantSquare = -1;
			int _FullmoveClock = 1;
			int _HalfmoveClock = 0;
			CastlingRights _WhiteCastling = CastlingRights::Both;
			CastlingRights _BlackCastling = CastlingRights::Both;
		};

	private:
		struct SpecialInfo
		{
			short EnPassant;
			short Halfmove;
			CastlingRights Rights;
		};

	private:
		std::vector<Move> m_MoveHistory;
		std::unordered_set<Move> m_LegalMoves;

		Bitboards m_Bitboards{ };
		std::array<ChessPiece, 64> m_Board{ };

		bool m_WhiteToPlay;
		short m_EnPassantSquare;
		int m_FullmoveClock;

		CastlingRights m_WhiteCastling;
		CastlingRights m_BlackCastling;

		short m_HalfmoveClock;
		std::vector<ChessPiece> m_PieceHistory;

		std::vector<SpecialInfo> m_InfoHistory;

	private:
		ChessPiece& AddPiece( int Square, Color color, PieceType type );
		void RemovePiece( int Square );
		void MovePiece( int Start, int Target );

		void GenerateMoves();

	public:
		Chessboard();
		BoardInfo GetBoardInfo() const;
		void LoadFEN( const std::string& FEN_Pos );
		void MakeMove( Move move );
		void UnMakeMove();

	};
}