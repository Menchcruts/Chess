#pragma once
#include "types.h"
#include "Bitboards.h"
#include <array>
#include <vector>
#include <string_view>

namespace Chess_Rework
{
	class Chessboard_New
	{
		struct PieceBitboards
		{
			Bitboard WKings		= 0;
			Bitboard BKings		= 0;
			Bitboard WQueens	= 0;
			Bitboard BQueens	= 0;
			Bitboard WRooks		= 0;
			Bitboard BRooks		= 0;
			Bitboard WBishops	= 0;
			Bitboard BBishops	= 0;
			Bitboard WKnights	= 0;
			Bitboard BKnights	= 0;
			Bitboard WPawns		= 0;
			Bitboard BPawns		= 0;

			void ToggleBit(Square sq, Piece piece)
			{
				Bitboard sq_bb = Bitboards::from_sq(sq);
				switch (piece)
				{
					case Piece::W_King:   WKings	^= sq_bb; break;
					case Piece::B_King:   BKings	^= sq_bb; break;
					case Piece::W_Queen:  WQueens	^= sq_bb; break;
					case Piece::B_Queen:  BQueens	^= sq_bb; break;
					case Piece::W_Rook:   WRooks	^= sq_bb; break;
					case Piece::B_Rook:   BRooks	^= sq_bb; break;
					case Piece::W_Bishop: WBishops	^= sq_bb; break;
					case Piece::B_Bishop: BBishops	^= sq_bb; break;
					case Piece::W_Knight: WKnights	^= sq_bb; break;
					case Piece::B_Knight: BKnights	^= sq_bb; break;
					case Piece::W_Pawn:   WPawns	^= sq_bb; break;
					case Piece::B_Pawn:   BPawns	^= sq_bb; break;
					default: break;
				}
			}
			void SetBit(Square sq, Piece piece)
			{
				Bitboard sq_bb = Bitboards::from_sq(sq);
				switch (piece)
				{
					case Piece::W_King:   WKings	|= sq_bb; break;
					case Piece::B_King:   BKings	|= sq_bb; break;
					case Piece::W_Queen:  WQueens	|= sq_bb; break;
					case Piece::B_Queen:  BQueens	|= sq_bb; break;
					case Piece::W_Rook:   WRooks	|= sq_bb; break;
					case Piece::B_Rook:   BRooks	|= sq_bb; break;
					case Piece::W_Bishop: WBishops	|= sq_bb; break;
					case Piece::B_Bishop: BBishops	|= sq_bb; break;
					case Piece::W_Knight: WKnights	|= sq_bb; break;
					case Piece::B_Knight: BKnights	|= sq_bb; break;
					case Piece::W_Pawn:   WPawns	|= sq_bb; break;
					case Piece::B_Pawn:   BPawns	|= sq_bb; break;
					default: break;
				}
			}
			void ClearBit(Square sq, Piece piece)
			{
				Bitboard sq_bb = Bitboards::from_sq(sq);
				switch (piece)
				{
					case Piece::W_King:   WKings	&= ~sq_bb; break;
					case Piece::B_King:   BKings	&= ~sq_bb; break;
					case Piece::W_Queen:  WQueens	&= ~sq_bb; break;
					case Piece::B_Queen:  BQueens	&= ~sq_bb; break;
					case Piece::W_Rook:   WRooks	&= ~sq_bb; break;
					case Piece::B_Rook:   BRooks	&= ~sq_bb; break;
					case Piece::W_Bishop: WBishops	&= ~sq_bb; break;
					case Piece::B_Bishop: BBishops	&= ~sq_bb; break;
					case Piece::W_Knight: WKnights	&= ~sq_bb; break;
					case Piece::B_Knight: BKnights	&= ~sq_bb; break;
					case Piece::W_Pawn:   WPawns	&= ~sq_bb; break;
					case Piece::B_Pawn:   BPawns	&= ~sq_bb; break;
					default: break;
				}
			}
			
			Bitboard GetAllPieces() const
			{
				return	WKings | WPawns | WKnights | WBishops | WRooks | WQueens | 
						BKings | BPawns | BKnights | BBishops | BRooks | BQueens;
			}
			Bitboard GetPieces(Color c) const
			{
				return c == White ? WKings | WPawns | WKnights | WBishops | WRooks | WQueens :
									BKings | BPawns | BKnights | BBishops | BRooks | BQueens;
			}
		};

		struct PrevState
		{
			int HalfMoveClock = 0;
			Square EP_Square = NoSquare;
			CastlingRights CastlingRights = CastlingRights::No_Castling;
			Piece CapturedPiece = NoPiece; // Possibly move this to separate vector later
		};

		template<typename T>
		static inline constexpr std::vector<T> make_reserved_vector(size_t reserved_space)
		{
			std::vector<T> result;
			result.reserve(reserved_space);
			return result;
		}

	public:
		void MakeMove(Move move);
		void UnMakeMove(Move move);
		void LoadFEN(const std::string_view& FEN_String);
		void ResetBoard();
		void PlacePiece(Square sq, Piece piece);
		Piece RemovePiece(Square sq);
		Piece GetPiece(Square sq) const;

		CastlingRights GetCastlingRights() const
		{
			return m_CastlingRights;
		}
		bool IsWhiteToMove() const
		{
			return m_WhiteToMove;
		}
		Square GetEnPassantSquare() const
		{
			return m_EP_Square;
		}
		int GetHalfMoveClock() const
		{
			return m_HalfMoveClock;
		}
		int GetFullMoveClock() const
		{
			return m_FullMoveClock;
		}
		const PieceBitboards& GetBitboards() const
		{
			return m_Bitboards;
		}
		const Move& GetLastMove() const
		{
			return m_MoveHistory.empty() ? NullMove : m_MoveHistory.back();
		}

		const std::vector<Move>& GetMoves() const
		{
			return m_Moves;
		}

		friend class MoveGenerator;
		friend class MoveGeneratorNonStatic;

	private:
		// Piece placements
		std::vector<Move> m_Moves = make_reserved_vector<Move>(218);
		PieceBitboards m_Bitboards;
		std::array<Piece, 64> m_BoardArray{};

		// Board state
		std::vector<PrevState> m_PrevStates = make_reserved_vector<PrevState>(128);
		std::vector<Move> m_MoveHistory = make_reserved_vector<Move>(128);

		int m_HalfMoveClock = 0;
		int m_FullMoveClock = 1;

		bool m_WhiteToMove = true;
		Square m_EP_Square = NoSquare;
		
		CastlingRights m_CastlingRights = CastlingRights::No_Castling;

	private:
		void GenerateMoves();
	};
}