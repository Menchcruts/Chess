#pragma once
#include <unordered_map>
#include <array>
#include <memory>
#include "PrecomputedData.h"
#include "../Move.h"
#include "../Bitboard.h"
#include "../CastlingRights.h"
#include "../Pieces.h"
#include "Logger.h"

namespace Chess { class Chessboard; }	// For some reason the compiler doesn't know what a Chessboard is unless we define it here as well

namespace Chess::MoveGen
{	
	class MoveGenerator
	{
	private:		
		std::unique_ptr<std::array<Move, 218>> m_Moves;
		//std::array<Move, 218>* m_Moves;
		
		std::unique_ptr< std::array<std::unordered_map<int, Bitboard>, 64> > RookMoves;
		//std::array<std::unordered_map<int, Bitboard>, 64>* RookMoves;

		std::unique_ptr< std::array<std::unordered_map<int, Bitboard>, 64> > BishopMoves;
		//std::array<std::unordered_map<int, Bitboard>, 64>* BishopMoves;

		std::unique_ptr< std::array<std::array<Bitboard, 64>, 64> > InBetweenLookup;
		//std::array<std::array<Bitboard, 64>, 64>* InBetweenLookup;

		std::unique_ptr< std::unordered_map<int, Bitboard> > m_PinRays;
		//std::unordered_map<int, Bitboard>* m_PinRays;

		Logger logger = Logger( "Movegenerator.log", Logger::Debug );

		Chessboard* m_Board = nullptr;

		Bitboard AllPieces;
		Bitboard FriendlyPieces;
		Bitboard EnemyPieces;
		Bitboard EnemyRQ;
		Bitboard EnemyBQ;

		Bitboard AttackMask;
		Bitboard Pinned;
		//Bitboard PinRays;
		Bitboard CheckMask;

		short MoveListTop = 0;
		short FriendlyKingSq = -1;
		Color FriendlyColor = Color::White;
		Color EnemyColor = Color::Black;
		CastlingRights Rights = CastlingRights::None;

		bool InCheck = false;
		bool InDoubleCheck = false;

	private:
		void AddMove( Move move);
		void AddMovesFromBitboard(int Start, Bitboard bb );
		void ClearMoveList();
		
		std::unique_ptr<std::array<std::unordered_map<int,Bitboard>,64>> CreateSlidingMoves(bool Diagonal );
		Bitboard InBetween( int sq1, int sq2 ) const;
		std::unique_ptr < std::array<std::array<Bitboard, 64>, 64>> CreateInBetweenLookup();
		Bitboard GetRookMoveMask( short Sq, Bitboard Occupied ) const;
		Bitboard GetBishopMoveMask( short Sq, Bitboard Occupied ) const;

		Bitboard XRayRookAttacks( Bitboard Occupied, Bitboard Blockers, short Sq );
		Bitboard XRayBishopAttacks( Bitboard Occupied, Bitboard Blockers, short Sq );

		Bitboard GetAlignMask( int Start, int Target ) const;

		void FindPinned();

		void GenerateInfo();

		void GetKingMoves( short Sq );
		void GetKnightMoves( short Sq );
		void GetPawnMoves( short Sq );
		void GetBishopMoves( short Sq );
		void GetRookMoves( short Sq );
		void GetQueenMoves( short Sq );

		bool CanEnPassant( short startSquare, short targetSquare, short epCaptureSquare ) const;

	public:
		MoveGenerator( );

		void GenerateMoves( Chessboard* board );

		/* Returns a copy of the move list */
		std::array<Move, 218> GetMoveList() const;
	};
}
