#include "Board.h"
#include <stdexcept>

namespace Chess
{
	Square Chess::Board::GetSquare( int Square )
	{
		return { (*this)[Square] };
	}

	void Board::Reset()
	{
		_Board.fill( { Color::None, PieceType::None } );
	}

	Piece& Board::operator[]( std::size_t index )
	{
		if ( index < 0 || index >= _Board.size() )
			throw std::out_of_range::out_of_range( "Index out of range for Chess::Board" );

		return _Board[index];
	}

	const Piece& Board::operator[]( std::size_t index ) const
	{
		if ( index < 0 || index >= _Board.size() )
			throw std::out_of_range::out_of_range( "Index out of range for Chess::Board" );

		return _Board[index];
	}
}