#pragma once
#include <array>
#include <string>
#include <vector>
#include <unordered_set>
#include "Bitboard.h"
#include "Pieces.h"
#include "Move.h"
#include "CastlingRights.h"
#include "MoveGeneration/MoveGen.h"


namespace Chess
{
	class Chessboard
	{
	public:
		struct BoardInfo
		{
			std::vector<Move> _LegalMoves;
			Bitboards _Bitboards{ };
			bool _WhiteToPlay = true;
			short _EnPassantSquare = -1;
			short _FullmoveClock = 1;
			short _HalfmoveClock = 0;
			CastlingRights _WhiteCastling = CastlingRights::Both;
			CastlingRights _BlackCastling = CastlingRights::Both;
			short _WhiteKingPos = -1;
			short _BlackKingPos = -1;
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
		std::vector<Move> m_LegalMoves;

		std::unordered_set<short> m_WhitePositions;
		std::unordered_set<short> m_BlackPositions;

		Bitboards m_Bitboards{ };
		Bitboard m_AttackMask;

		CastlingRights m_WhiteCastling;
		CastlingRights m_BlackCastling;

		short m_EnPassantSquare;
		short m_HalfmoveClock;
		short m_FullmoveClock;

		short m_WhiteKingPos = -1;
		short m_BlackKingPos = -1;

		bool m_WhiteToPlay = true;
		
		bool m_InCheck = false;
		bool m_InDoubleCheck = false;
		bool m_EnPassantBlocked = false;

		Bitboard m_PinMask;
		Bitboard m_CheckMask;

		std::vector<ChessPiece> m_PieceHistory;
		std::vector<SpecialInfo> m_InfoHistory;

	private:
		void AddPiece( int Square, Color color, PieceType type );
		void RemovePiece( int Square );
		void MovePiece( int Start, int Target );
		void PromotePiece( int Square, PieceType NewType );

		void GenerateMoves();
		int Perft( int Depth );

	public:
		Chessboard();
		Chessboard( const std::string& FEN_Pos );
		BoardInfo GetBoardInfo() const;
		void LoadFEN( const std::string& FEN_Pos );
		void MakeMove( Move move );
		void UnMakeMove();
		Move GetLastMove() const;

		int RunPerft( int Depth );
	};
}