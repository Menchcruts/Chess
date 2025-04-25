#pragma once
#include <array>
#include <string>
#include <vector>
#include <unordered_set>
#include "Bitboard.h"
#include "Pieces.h"
#include "Move.h"
#include "CastlingRights.h"
//#include "MoveGeneration/MoveGen.h"
#include "MoveGeneration/Generator.h"


namespace Chess
{
	class Chessboard
	{
	private:
		struct SpecialInfo
		{
			short EnPassant;
			short Halfmove;
			ChessPiece PieceCaptured;
			CastlingRights Rights;
		};

	public:
		Bitboards m_Bitboards{ };

		std::vector<SpecialInfo> m_InfoHistory;

		MoveGen::MoveGenerator m_MoveGenerator;

		Bitboard m_AttackMask;
		
		Bitboard m_PinMask;
		Bitboard m_CheckMask;
		
		short m_EnPassantSquare;
		short m_HalfmoveClock;
		short m_FullmoveClock;
		
		CastlingRights m_WhiteCastling;
		CastlingRights m_BlackCastling;

		bool m_WhiteToPlay = true;
		
		bool m_InCheck = false;
		bool m_InDoubleCheck = false;
		bool m_EnPassantBlocked = false;

	private:
		void Init( const std::string& FEN_Pos );

		void AddPiece( int Square, Color color, PieceType type );
		void RemovePiece( int Square );
		void MovePiece( int Start, int Target );
		void PromotePiece( int Square, PieceType NewType );

		int Perft(int Depth, bool FirstPass );

		void GenerateMoves();

	public:
		Chessboard();
		Chessboard( const std::string& FEN_Pos );

		std::array<Move, 218> GetMoveList() const;

		void LoadFEN( const std::string& FEN_Pos );
		void MakeMove( Move move );
		void UnMakeMove( Move move );

		int RunPerft(int Depth, bool ShowInfo = true	 );
	};
}