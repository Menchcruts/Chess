#include "BoardVisuals.h"
#include "Images/Images.h"

void DrawChessboardScreen()
{
    ImGui::SetNextWindowSizeConstraints(
        ImVec2( 800, 800 ),
        ImVec2( 1250, 1250 )
    );
    // Begin a new window for the chessboard
    ImGui::Begin( "Chessboard" );

    // Get the current ImGui window's drawing list and draw the board
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    DrawChessBoard( drawList, _placeholder_ );

    // Draw the selected piece on the mouse
    DrawPieceSelected( drawList );

    ImGui::End();
}

void DrawChessBoard( ImDrawList* DrawList, const Chess::Board& Board )
{
    if ( ImGui::IsWindowFocused() && ImGui::IsMouseClicked( 0 ) )
        m_BoardSettings.HighlightedSquares.clear();

    const int BoardSize = 8;


    ImVec2 p = ImGui::GetCursorScreenPos(); // Top-left corner of where we can draw inside the window

    ImGui::PushStyleVar( ImGuiStyleVar_CellPadding, ImVec2( 0, 0 ) );
    ImGui::BeginTable( "PieceGrid", BoardSize );
    for ( int i = 0; i < BoardSize; ++i )
        ImGui::TableSetupColumn( "", ImGuiTableColumnFlags_WidthFixed );

    // Draw the chessboard cells
    for ( int row = 0; row < BoardSize; ++row )
    {
        for ( int col = 0; col < BoardSize; ++col )
        {
            // Determine the cell color based on row and column
            bool isDark = ((row + col) % 2) == 1;
            int row_inverted = 7 - row;

            int CurrSq = row_inverted * 8 + col;
            Chess::Piece Piece = Board[CurrSq];

            // Calculate the cell's corners
            ImVec2 cellMin = ImVec2( p.x + col * m_BoardSettings.CellSize, p.y + row * m_BoardSettings.CellSize );
            ImVec2 cellMax = ImVec2( cellMin.x + m_BoardSettings.CellSize, cellMin.y + m_BoardSettings.CellSize );

            float radius = 15.0f;

            ImGui::TableNextColumn();

            ImGui::Dummy( ImVec2( m_BoardSettings.CellSize, m_BoardSettings.CellSize ) ); // Dummy widget to get hover states
            if ( ImGui::IsItemHovered() )
            {
                HandleBoardClicks( Piece, CurrSq );
                radius = 21.0f;
            }

            // Draw the cell
            ImColor CellColor;
            if ( CurrSq == m_BoardSettings.SelectedSquare )
                CellColor = isDark ? m_Colors.EvenColorHighlight : m_Colors.OddColorHighlight;
            else if ( m_BoardSettings.HighlightedSquares.contains( CurrSq ) )
                CellColor = isDark ? m_Colors.EvenColorRed : m_Colors.OddColorRed;
            else
                CellColor = isDark ? m_Colors.EvenColor : m_Colors.OddColor;

            ImU32 col32 = CellColor;
            DrawList->AddRectFilled( cellMin, cellMax, col32 );

            if ( CurrSq == 1 || CurrSq == 2 )
            {
                ImVec2 center = ImVec2( (cellMax.x - cellMin.x) / 2 + cellMin.x, (cellMax.y - cellMin.y) / 2 + cellMin.y );
                DrawList->AddCircleFilled( center, radius, (ImU32)ImColor( 95, 95, 95, 63 ) );
            }

            // Draw the piece on the square
            if ( Piece.color == Chess::Color::None || Piece.type == Chess::PieceType::None )
                continue;

            if ( CurrSq == m_BoardSettings.SelectedSquare && m_BoardSettings.PieceHeld.type != Chess::PieceType::None )
                continue;   // No need to draw what is being held

            const Image& image = m_PieceImages[(int)Piece.color][Piece.type];
            DrawList->AddImage( image.Texture, cellMin, cellMax );
        }
    }
    ImGui::EndTable();
    ImGui::PopStyleVar();
}

void HandleBoardClicks( Chess::Piece Piece, int CurrentSq )
{

}

void DrawPieceSelected( ImDrawList* DrawList )
{

}
