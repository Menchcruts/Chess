#include "Bitboards.h"
#include <array>
#include <vector>
#include <iostream>

namespace Chess_Rework::Bitboards
{
	static bool initialized = false;

	Bitboard BetweenBB[64][64];
	Bitboard LineBB[64][64];
	std::uint8_t SquareDistances[64][64];

	Bitboard KingAttacks[64];
	Bitboard KnightAttacks[64];

	Bitboard BishopAttacks[64];
	Bitboard RookAttacks[64];
	Bitboard QueenAttacks[64];

	Bitboard RookBlockers[64];
	Bitboard BishopBlockers[64];

	Magic Magics[64][2];

	static Bitboard safe_destination( Square x, int dx, int tol = 2 )
	{
		Square to = Square( x + dx );
		return (is_ok(to) && distance(x, to) <= tol) ? from_sq(to) : 0;
	}

	static Bitboard blocker_mask( Square sq, PieceType pt )
	{
		assert( pt == PieceType::Bishop || pt == PieceType::Rook );
		
		Bitboard mask = 0;

		std::array<Direction, 4> dirs = pt == PieceType::Rook ?
			std::array<Direction, 4>{ Dir_North, Dir_South, Dir_East, Dir_West } :
			std::array<Direction, 4>{ Dir_NorthEast, Dir_NorthWest, Dir_SouthEast, Dir_SouthWest };

		for ( Direction dir : dirs )
		{
			Bitboard center = from_sq( sq );
			Bitboard shifted = center;
			for ( int i = 0; i < (Squares_to_Edge( sq, dir ) - 1); ++i )
			{
				shifted = shift( shifted, dir );
				mask |= shifted;
			}
		}
		return mask;
	}

	static std::vector<Bitboard> blocker_perms( Bitboard mask, bool is_rook )
	{
		std::vector<Bitboard> result;

		if ( is_rook )
			result.reserve( 4096 );
		else
			result.reserve( 512 );

		std::vector<int> indexes;
		indexes.reserve( 12 );

		Bitboard temp = mask;
		while ( temp )
		{
			indexes.push_back( bitscan_forward_auto(temp) );
		}

		int numPatterns = 1 << indexes.size();

		result.reserve( numPatterns );

		for ( int pattern = 0; pattern < numPatterns; ++pattern )
		{
			Bitboard currentMask = 0;
			Bitboard tempBit = 0;
			for ( int i = 0; i < int(indexes.size()); ++i )
			{
				tempBit = (pattern >> i) & 1;
				currentMask |= (tempBit << indexes[i]);
			}
			result.push_back( currentMask );
		}

		return result;
	}

	static Bitboard get_move_bb_from_blocker( Square sq, Bitboard blocker, PieceType pt )
	{
		assert( pt == PieceType::Bishop || pt == PieceType::Rook );
		
		std::array<Direction, 4> dirs = pt == PieceType::Rook ?
			std::array<Direction, 4>{ Dir_North, Dir_South, Dir_East, Dir_West } :
			std::array<Direction, 4>{ Dir_NorthEast, Dir_NorthWest, Dir_SouthEast, Dir_SouthWest };

		Bitboard result = 0;

		for ( Direction dir : dirs )
		{
			Bitboard shifted = from_sq( sq );
			for ( int i = 0; i < Squares_to_Edge(sq, dir); ++i )
			{
				shifted = shift( shifted, dir );
				result |= shifted;

				if ( blocker & shifted )
					break;
			}
		}
		return result;
	}

	static void init_magics()
	{
		constexpr const short RookShifts[64] = { 52, 52, 52, 52, 52, 52, 52, 52, 53, 53, 53, 54, 53, 53, 54, 53, 53, 54, 54, 54, 53, 53, 54, 53, 53, 54, 53, 53, 54, 54, 54, 53, 52, 54, 53, 53, 53, 53, 54, 53, 52, 53, 54, 54, 53, 53, 54, 53, 53, 54, 54, 54, 53, 53, 54, 53, 52, 53, 53, 53, 53, 53, 53, 52 };
		constexpr const Bitboard RookMagics[64] = { 468374916371625120, 18428729537625841661, 2531023729696186408, 6093370314119450896, 13830552789156493815, 16134110446239088507, 12677615322350354425, 5404321144167858432, 2111097758984580, 18428720740584907710, 17293734603602787839, 4938760079889530922, 7699325603589095390, 9078693890218258431, 578149610753690728, 9496543503900033792, 1155209038552629657, 9224076274589515780, 1835781998207181184, 509120063316431138, 16634043024132535807, 18446673631917146111, 9623686630121410312, 4648737361302392899, 738591182849868645, 1732936432546219272, 2400543327507449856, 5188164365601475096, 10414575345181196316, 1162492212166789136, 9396848738060210946, 622413200109881612, 7998357718131801918, 7719627227008073923, 16181433497662382080, 18441958655457754079, 1267153596645440, 18446726464209379263, 1214021438038606600, 4650128814733526084, 9656144899867951104, 18444421868610287615, 3695311799139303489, 10597006226145476632, 18436046904206950398, 18446726472933277663, 3458977943764860944, 39125045590687766, 9227453435446560384, 6476955465732358656, 1270314852531077632, 2882448553461416064, 11547238928203796481, 1856618300822323264, 2573991788166144, 4936544992551831040, 13690941749405253631, 15852669863439351807, 18302628748190527413, 12682135449552027479, 13830554446930287982, 18302628782487371519, 7924083509981736956, 4734295326018586370 };

		constexpr const short BishopShift[64] = { 58, 60, 59, 59, 59, 59, 60, 58, 60, 59, 59, 59, 59, 59, 59, 60, 59, 59, 57, 57, 57, 57, 59, 59, 59, 59, 57, 55, 55, 57, 59, 59, 59, 59, 57, 55, 55, 57, 59, 59, 59, 59, 57, 57, 57, 57, 59, 59, 60, 60, 59, 59, 59, 59, 60, 60, 58, 60, 59, 59, 59, 59, 59, 58 };
		constexpr const Bitboard BishopMagics[64] = { 16509839532542417919, 14391803910955204223, 1848771770702627364, 347925068195328958, 5189277761285652493, 3750937732777063343, 18429848470517967340, 17870072066711748607, 16715520087474960373, 2459353627279607168, 7061705824611107232, 8089129053103260512, 7414579821471224013, 9520647030890121554, 17142940634164625405, 9187037984654475102, 4933695867036173873, 3035992416931960321, 15052160563071165696, 5876081268917084809, 1153484746652717320, 6365855841584713735, 2463646859659644933, 1453259901463176960, 9808859429721908488, 2829141021535244552, 576619101540319252, 5804014844877275314, 4774660099383771136, 328785038479458864, 2360590652863023124, 569550314443282, 17563974527758635567, 11698101887533589556, 5764964460729992192, 6953579832080335136, 1318441160687747328, 8090717009753444376, 16751172641200572929, 5558033503209157252, 17100156536247493656, 7899286223048400564, 4845135427956654145, 2368485888099072, 2399033289953272320, 6976678428284034058, 3134241565013966284, 8661609558376259840, 17275805361393991679, 15391050065516657151, 11529206229534274423, 9876416274250600448, 16432792402597134585, 11975705497012863580, 11457135419348969979, 9763749252098620046, 16960553411078512574, 15563877356819111679, 14994736884583272463, 9441297368950544394, 14537646123432199168, 9888547162215157388, 18140215579194907366, 18374682062228545019 };

		
		for (Square sq = SQ_A1; sq <= SQ_H8; ++sq )
		{
			// Rook magic
			Bitboard rookMask = blocker_mask(sq, PieceType::Rook);
			std::vector<Bitboard> rookPerms = blocker_perms(rookMask, true);

			RookBlockers[sq] = rookMask;

			Magic magic = Magic(RookMagics[sq], RookShifts[sq], RookBlockers[sq]);
			for (const Bitboard& blockers : rookPerms)
			{
				magic.attacks[magic.index(sq, blockers)] = get_move_bb_from_blocker(sq, blockers, PieceType::Rook);
			}
			Magics[sq][1] = magic;

			// Bishop magic
			Bitboard bishopMask = blocker_mask(sq, PieceType::Bishop);
			std::vector<Bitboard> bishopPerms = blocker_perms(bishopMask, false);

			BishopBlockers[sq] = bishopMask;

			magic = Magic(BishopMagics[sq], BishopShift[sq], BishopBlockers[sq]);
			for (const Bitboard& blockers : bishopPerms)
			{
				magic.attacks[magic.index(sq, blockers)] = get_move_bb_from_blocker(sq, blockers, PieceType::Bishop);
			}
			Magics[sq][0] = magic;
		}
	}

	static void init_lookups()
	{
		for (auto pt : { Rook, Bishop })
		{
			const auto& pseudo_attacks = (pt == Rook) ? RookAttacks : BishopAttacks;

			for (Square sq1 = SQ_A1; sq1 <= SQ_H8; ++sq1)
			{
				for (Square sq2 = SQ_A1; sq2 <= SQ_H8; ++sq2)
				{					
					if (pseudo_attacks[sq1] & from_sq(sq2))
					{
						BetweenBB[sq1][sq2] = attacks(sq1, pt, from_sq(sq2)) & attacks(sq2, pt, from_sq(sq1));
						LineBB[sq1][sq2] = (pseudo_attacks[sq1] & pseudo_attacks[sq2]) | from_sq(sq1) | from_sq(sq2);
					}
				}
			}
		}
	}

	void init()
	{
		if ( initialized )
			return;
		initialized = true;
		
		init_magics();
		
		for ( Square sq1 = SQ_A1; sq1 <= SQ_H8; ++sq1 )
		{
			for ( Square sq2 = SQ_A1; sq2 <= SQ_H8; ++sq2 )
			{
				SquareDistances[sq1][sq2] = std::max( distance<File>( sq1, sq2 ), distance<Rank>( sq1, sq2 ) );
			}

			for ( int step : {8, 9, 1, -7, -8, -9, -1, 7} )
				KingAttacks[sq1] |= safe_destination( sq1, step );

			for ( int step : {6, 10, 15, 17, -17, -15, -10, -6} )
				KnightAttacks[sq1] |= safe_destination( sq1, step );

			BishopAttacks[sq1] = attacks(sq1, Bishop, 0);
			RookAttacks[sq1] = attacks(sq1, Rook, 0);
			QueenAttacks[sq1] = RookAttacks[sq1] | BishopAttacks[sq1];
		}

		init_lookups();
	}
}