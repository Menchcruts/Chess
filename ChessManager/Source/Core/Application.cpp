#include <iostream>
#include <filesystem>
#include <format>
#include <chrono>
#include <array>
#include <stdio.h>

#include "Application.h"


static const std::filesystem::path AssetsDir( "Assets" );
static const std::filesystem::path PiecesDir = AssetsDir / "Pieces";
static const std::filesystem::path SoundsDir = AssetsDir / "Sounds";

static std::string GetPieceFileName( Chess::PieceType Piece, Chess::Color Color )
{
    bool IsWhite = Color == Chess::Color::White;
    switch ( Piece )
    {
    case Chess::PieceType::King:
        return IsWhite ? "wk.png" : "bk.png";
    case Chess::PieceType::Pawn:
        return IsWhite ? "wp.png" : "bp.png";
    case Chess::PieceType::Knight:
        return IsWhite ? "wn.png" : "bn.png";
    case Chess::PieceType::Bishop:
        return IsWhite ? "wb.png" : "bb.png";
    case Chess::PieceType::Rook:
        return IsWhite ? "wr.png" : "br.png";
    case Chess::PieceType::Queen:
        return IsWhite ? "wq.png" : "bq.png";
    case Chess::PieceType::None:
    default:
        return "";
    }
}

static std::string GetSquareName( int Square )
{
    if ( Square == -1 )
        return "None";

    int rank = Square >> 3;
    int file = Square & 7;

    const char* files = "abcdefgh";
    const char* ranks = "12345678";

    std::string result;
    result += files[file];
    result += ranks[rank];
    return result;
}

static std::string GetCastlingString( Chess::CastlingRights Rights )
{
    switch ( Rights )
    {
    case Chess::CastlingRights::Kingside:
        return "King side";
    case Chess::CastlingRights::QueenSide:
        return "Queen side";
    case Chess::CastlingRights::Both:
        return "Both";
    case Chess::CastlingRights::None:
    default:
        return "None";
    }
}

static std::string FormatMs( std::chrono::milliseconds time )
{
    using namespace std::chrono;
    
    if ( time >= 1h )
        return std::format( "{:%H:%M:%S}", duration_cast<seconds>(time) );
    else if ( time >= 10s )
        return std::format( "{:%M:%S}", duration_cast<seconds>(time) );
    else
        return std::format( "{:%M:%S}", time ); // To show milliseconds
}



void ChessApp::DrawChessboardScreen()
{
    ImGui::SetNextWindowSizeConstraints(
        ImVec2( 800, 800 ),
        ImVec2( 1250, 1250 )
    );

    ImGui::Begin( "Chessboard" );

    // Draw the board
    if ( !m_BoardVisuals.FlipBoard )
        DrawChessBoard();
    else
        DrawChessBoardFlipped();

    if ( m_PromotionHandling.Promoting )
        PromoteScreen();

    // Draw the selected piece on the mouse
    if ( !m_PromotionHandling.Promoting )
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        DrawPieceSelected( drawList );
    }

    ImGui::End();
}

void ChessApp::DrawChessBoard()
{
    if ( ImGui::IsWindowFocused() && ImGui::IsMouseClicked( 0 ) )
        m_BoardVisuals.HighlightedSquares.clear();

    const float& CellSize = m_BoardVisuals.CellSize;
    const float CellSize_Half = CellSize / 2;

    ImVec2 ImageSize = ImVec2( CellSize, CellSize );

    ImDrawList* DrawList = ImGui::GetWindowDrawList();
    ImVec2 CursorPos;
    ImVec2 CircleCenter;
    ImU32 MoveCircleColor = IM_COL32( 255, 0, 0, 127 );
    float MoveCircleRadius = 0.0f;

    bool DrawMoveCircles = (m_MoveHandling.SelectedSquare != -1) && m_LegalMovesDict.contains( m_MoveHandling.SelectedSquare );
    bool SquareIsHovered;
    bool PieceOnSquare;

    const LegalMovesInfo& info = DrawMoveCircles ? m_LegalMovesDict[m_MoveHandling.SelectedSquare] : LegalMovesInfo();
    const Chess::Move& LastMove = m_BoardVisuals.LastMove;

    const Chess::Bitboards& bitboards = m_BoardVariables.BoardInfo._Bitboards;
    
    ImGui::PushStyleVar( ImGuiStyleVar_CellPadding, ImVec2( 0, 0 ) );
    ImGui::BeginTable( "PieceGrid", 8 );
    for ( int i = 0; i < 8; ++i )
        ImGui::TableSetupColumn( "", ImGuiTableColumnFlags_WidthFixed );

    // Draw the chessboard cells
    for ( int row = 0; row < 8; ++row )
    {
        for ( int col = 0; col < 8; ++col )
        {
            SquareIsHovered = false;
            PieceOnSquare = false;

            // Determine the cell color based on row and column
            bool isDark = ((row + col) % 2) == 1;
            int row_inverted = 7 - row;

            int CurrSq = row_inverted * 8 + col;
            Chess::ChessPiece Piece = bitboards.GetPieceAtSquare(CurrSq);

            ImColor CellColor;
            if ( CurrSq == m_MoveHandling.SelectedSquare || (!LastMove.IsNullMove() && (CurrSq == LastMove.Start() || CurrSq == LastMove.Target())) )
                CellColor = isDark ? Settings.Colors.EvenColorHighlight : Settings.Colors.OddColorHighlight;
            else if ( m_BoardVisuals.HighlightedSquares.contains( CurrSq ) )
                CellColor = isDark ? Settings.Colors.EvenColorRed : Settings.Colors.OddColorRed;
            else
                CellColor = isDark ? Settings.Colors.EvenColor : Settings.Colors.OddColor;

            ImU32 CellCol32 = CellColor;

            ImGui::TableNextColumn();
            //ImGui::TableSetColumnIndex( col ); // Virkar ekki af einhverjum ástæðum?

            if (
                (Piece.IsNullPiece()) ||
                (CurrSq == m_MoveHandling.SelectedSquare && m_MoveHandling.PieceHeld.type != Chess::PieceType::None)
                )   // Þetta er hræðilegt if statement
            {
                ImGui::Dummy( ImageSize ); // Dummy widget fyrir mouse clicks
            }
            else
            {
                const Image& image = m_PieceImages[(int)Piece.color][Piece.type];
                ImGui::Image( image.Texture, ImageSize );   // Annars teiknum við myndina
                PieceOnSquare = true;
            }

            if ( ImGui::IsItemHovered() && !m_PromotionHandling.Promoting )
            {
                HandleBoardClicks( Piece, CurrSq );
                SquareIsHovered = true;
            }

            ImGui::TableSetBgColor( ImGuiTableBgTarget_CellBg, CellCol32 );

            if ( DrawMoveCircles && info.Targets.contains( CurrSq ) && !m_PromotionHandling.Promoting )
            {
                CursorPos = ImGui::GetCursorScreenPos();
                CursorPos.y -= 4.0f;
                CircleCenter = ImVec2( CursorPos.x + CellSize_Half, CursorPos.y - CellSize_Half );

                if ( SquareIsHovered )
                    MoveCircleRadius = 21.0f;
                else
                    MoveCircleRadius = 15.0f;

                if ( PieceOnSquare )
                {
                    if ( !SquareIsHovered )
                    {
                        MoveCircleRadius = CellSize_Half - 4.0f;    // Compensate for circle width
                        DrawList->AddCircle( CircleCenter, MoveCircleRadius, MoveCircleColor, 0, 9.0f );
                        continue;
                    }
                    else
                    {
                        MoveCircleRadius = CellSize_Half;
                    }
                }
                DrawList->AddCircleFilled( CircleCenter, MoveCircleRadius, MoveCircleColor );
            }
        }
    }
    ImGui::EndTable();
    ImGui::PopStyleVar();
}

void ChessApp::DrawChessBoardFlipped()
{
    if ( ImGui::IsWindowFocused() && ImGui::IsMouseClicked( 0 ) )
        m_BoardVisuals.HighlightedSquares.clear();

    const float& CellSize = m_BoardVisuals.CellSize;
    const float CellSize_Half = CellSize / 2;

    ImVec2 ImageSize = ImVec2( CellSize, CellSize );

    ImDrawList* DrawList = ImGui::GetWindowDrawList();
    ImVec2 CursorPos;
    ImVec2 CircleCenter;
    ImU32 MoveCircleColor = IM_COL32( 255, 0, 0, 127 );
    float MoveCircleRadius = 0.0f;

    bool DrawMoveCircles = (m_MoveHandling.SelectedSquare != -1) && m_LegalMovesDict.contains( m_MoveHandling.SelectedSquare );
    bool SquareIsHovered;
    bool PieceOnSquare;

    const LegalMovesInfo& info = DrawMoveCircles ? m_LegalMovesDict[m_MoveHandling.SelectedSquare] : LegalMovesInfo();
    const Chess::Move& LastMove = m_BoardVisuals.LastMove;

    const Chess::Bitboards& bitboards = m_BoardVariables.BoardInfo._Bitboards;

    ImGui::PushStyleVar( ImGuiStyleVar_CellPadding, ImVec2( 0, 0 ) );
    ImGui::BeginTable( "PieceGrid", 8 );
    for ( int i = 0; i < 8; ++i )
        ImGui::TableSetupColumn( "", ImGuiTableColumnFlags_WidthFixed );

    // Draw the chessboard cells
    for ( int row = 0; row < 8; ++row )
    {
        for ( int col = 0; col < 8; ++col )
        {
            SquareIsHovered = false;
            PieceOnSquare = false;

            // Determine the cell color based on row and column
            bool isDark = ((row + col) % 2) == 1;
            int col_inverterd = 7 - col;

            int CurrSq = row * 8 + col_inverterd;
            Chess::ChessPiece Piece = bitboards.GetPieceAtSquare( CurrSq );

            ImColor CellColor;
            if ( CurrSq == m_MoveHandling.SelectedSquare || (!LastMove.IsNullMove() && (CurrSq == LastMove.Start() || CurrSq == LastMove.Target())) )
                CellColor = isDark ? Settings.Colors.EvenColorHighlight : Settings.Colors.OddColorHighlight;
            else if ( m_BoardVisuals.HighlightedSquares.contains( CurrSq ) )
                CellColor = isDark ? Settings.Colors.EvenColorRed : Settings.Colors.OddColorRed;
            else
                CellColor = isDark ? Settings.Colors.EvenColor : Settings.Colors.OddColor;

            ImU32 CellCol32 = CellColor;

            ImGui::TableNextColumn();
            //ImGui::TableSetColumnIndex( col ); // Virkar ekki af einhverjum ástæðum?

            if (
                (Piece.IsNullPiece()) ||
                (CurrSq == m_MoveHandling.SelectedSquare && m_MoveHandling.PieceHeld.type != Chess::PieceType::None)
                )   // Þetta er hræðilegt if statement
            {
                ImGui::Dummy( ImageSize ); // Dummy widget fyrir mouse clicks
            }
            else
            {
                const Image& image = m_PieceImages[(int)Piece.color][Piece.type];
                ImGui::Image( image.Texture, ImageSize );   // Annars teiknum við myndina
                PieceOnSquare = true;
            }

            if ( ImGui::IsItemHovered() && !m_PromotionHandling.Promoting )
            {
                HandleBoardClicks( Piece, CurrSq );
                SquareIsHovered = true;
            }

            ImGui::TableSetBgColor( ImGuiTableBgTarget_CellBg, CellCol32 );

            if ( DrawMoveCircles && info.Targets.contains( CurrSq ) && !m_PromotionHandling.Promoting )
            {
                CursorPos = ImGui::GetCursorScreenPos();
                CursorPos.y -= 4.0f;
                CircleCenter = ImVec2( CursorPos.x + CellSize_Half, CursorPos.y - CellSize_Half );

                if ( SquareIsHovered )
                    MoveCircleRadius = 21.0f;
                else
                    MoveCircleRadius = 15.0f;

                if ( PieceOnSquare )
                {
                    if ( !SquareIsHovered )
                    {
                        MoveCircleRadius = CellSize_Half - 4.0f;    // Compensate for circle width
                        DrawList->AddCircle( CircleCenter, MoveCircleRadius, MoveCircleColor, 0, 9.0f );
                        continue;
                    }
                    else
                    {
                        MoveCircleRadius = CellSize_Half;
                    }
                }
                DrawList->AddCircleFilled( CircleCenter, MoveCircleRadius, MoveCircleColor );
            }
        }
    }
    ImGui::EndTable();
    ImGui::PopStyleVar();
}

Chess::Move ChessApp::IsValidMove( int Start, int Target ) const
{
    Chess::Move NewMove;
    for ( const auto& move : m_LegalMovesDict.at(Start).Moves )
    {
        if ( move.Target() == Target )
        {
            NewMove = move;
            break;
        }
    }
    return NewMove;
}

void ChessApp::PromoteScreen()
{        
    Chess::MoveFlag PromotionType = Chess::MoveFlag::None;
    
    const float& CellSize = m_BoardVisuals.CellSize;
    ImVec2 Cell = ImVec2( CellSize, CellSize );

    ImVec2 StartPos = ImVec2( m_PromotionHandling.PromotionScreenPos.x, m_PromotionHandling.PromotionScreenPos.y - 4.0f - CellSize );

    ImVec2 ChildHeight = ImVec2( CellSize, CellSize * 4 + ImGui::GetFontSize() * 2 + 10.0f );

    bool MenuGoDown = m_BoardVariables.BoardInfo._WhiteToPlay;
    if ( m_BoardVisuals.FlipBoard )
        MenuGoDown = !MenuGoDown;

    if ( !MenuGoDown )
        StartPos.y -= (ChildHeight.y - CellSize);

    ImVec4 ButtonColor = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];
    ImVec4 BackgroundColor = ImVec4( 255, 255, 255, 255 );

    ImGui::SetCursorScreenPos( StartPos );
    ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0, 0 ) );
    ImGui::PushStyleColor( ImGuiCol_ChildBg, BackgroundColor );
    ImGui::BeginChild( "PromotionSelect", ChildHeight );

    if ( !ImGui::IsWindowFocused() && ImGui::IsMouseClicked( 0 ) )
    {
        std::cout << "Clicked off promoting!\n";
        m_MoveHandling.PieceHeld.MakeNullPiece();
        m_MoveHandling.SelectedSquare = -1;
        m_MoveHandling.SelectedPressed = false;

        m_PromotionHandling.Promoting = false;
        m_BoardVariables.NextMove = Chess::Move();
    }
    
    ImGui::PushStyleColor( ImGuiCol_ButtonHovered, BackgroundColor );
    ImGui::PushStyleColor( ImGuiCol_ButtonActive, BackgroundColor );
    ImGui::PushStyleColor( ImGuiCol_Button, BackgroundColor );
    ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0, 0, 0, 255 ) );

    if ( m_BoardVariables.BoardInfo._WhiteToPlay )
    {
        if ( ImGui::ImageButton( "w_queen", m_PieceImages[0].at( Chess::PieceType::Queen ).Texture, Cell ) )
        {
            PromotionType = Chess::MoveFlag::PromoteQueen;
        }
        if ( ImGui::ImageButton( "w_rook", m_PieceImages[0].at( Chess::PieceType::Rook ).Texture, Cell ) )
        {
            PromotionType = Chess::MoveFlag::PromoteRook;
        }
        if ( ImGui::ImageButton( "w_bishop", m_PieceImages[0].at( Chess::PieceType::Bishop ).Texture, Cell ) )
        {
            PromotionType = Chess::MoveFlag::PromoteBishop;
        }
        if ( ImGui::ImageButton( "w_knight", m_PieceImages[0].at( Chess::PieceType::Knight ).Texture, Cell ) )
        {
            PromotionType = Chess::MoveFlag::PromoteKnight;
        }
        ImGui::Button( "Cancel", ImVec2( CellSize, 0 ) );
    }
    else
    {
        ImGui::Button( "Cancel", ImVec2( CellSize, 0 ) );
        if ( ImGui::ImageButton( "b_knight", m_PieceImages[1].at( Chess::PieceType::Knight ).Texture, Cell ) )
        {
            PromotionType = Chess::MoveFlag::PromoteKnight;
        }
        if ( ImGui::ImageButton( "b_bishop", m_PieceImages[1].at( Chess::PieceType::Bishop ).Texture, Cell ) )
        {
            PromotionType = Chess::MoveFlag::PromoteBishop;
        }
        if ( ImGui::ImageButton( "b_rook", m_PieceImages[1].at( Chess::PieceType::Rook ).Texture, Cell ) )
        {
            PromotionType = Chess::MoveFlag::PromoteRook;
        }
        if ( ImGui::ImageButton( "b_queen", m_PieceImages[1].at( Chess::PieceType::Queen ).Texture, Cell ) )
        {
            PromotionType = Chess::MoveFlag::PromoteQueen;
        }
    }

    ImGui::PopStyleColor( 4 );
    ImGui::PopStyleVar();

    ImGui::EndChild();
    ImGui::PopStyleColor();

    if ( PromotionType != Chess::MoveFlag::None )
    {
        Chess::Move NextMove = m_BoardVariables.NextMove;
        Chess::MoveFlag WasCapture = NextMove.Flag() & Chess::MoveFlag::Capture;
        PromotionType |= WasCapture;
        short Start = NextMove.Start();
        short Target = NextMove.Target();

        m_BoardVariables.NextMove = Chess::Move( Start, Target, PromotionType );
        m_PromotionHandling.Promoting = false;
        return;
    }
}

void ChessApp::HandleBoardClicks( Chess::ChessPiece Piece, int CurrentSq )
{
    if ( ImGui::IsMouseClicked( 0 ) && !m_PromotionHandling.Promoting )
    {
        //std::cout << "Mouse clicked on " << CurrentSq << "\n";
        
        if ( m_MoveHandling.SelectedSquare != -1 && CurrentSq != m_MoveHandling.SelectedSquare )  // Check for making a move
        {
            if ( m_LegalMovesDict.contains( m_MoveHandling.SelectedSquare ) )
            {
                Chess::Move NewMove = IsValidMove( m_MoveHandling.SelectedSquare, CurrentSq );
                if ( !NewMove.IsNullMove() ) // Future check for valid move
                {
                    m_BoardVariables.NextMove = NewMove;
                    if ( (NewMove.Flag() & Chess::MoveFlag::PromoteKnight) != Chess::MoveFlag::None )   // Promoting
                    {
                        m_PromotionHandling.Promoting = true;
                        m_PromotionHandling.PromotionScreenPos = ImGui::GetCursorScreenPos();
                        m_MoveHandling.PieceHeld = Piece;
                        return;
                    }
                    m_MoveHandling.SelectedSquare = -1;
                    m_MoveHandling.PieceHeld.MakeNullPiece();
                    return;
                }
            }
        }
        
        if ( !Piece.IsNullPiece() ) // Clicked a piece
        {
            if ( CurrentSq == m_MoveHandling.SelectedSquare )
                m_MoveHandling.SelectedPressed = true;
            else
                m_MoveHandling.SelectedSquare = CurrentSq;
            m_MoveHandling.PieceHeld = Piece;
        }
        else
        {
            m_MoveHandling.SelectedSquare = -1;
            m_MoveHandling.PieceHeld.MakeNullPiece();
        }
    }
    else if ( ImGui::IsMouseReleased( 0 ) && !m_PromotionHandling.Promoting )
    {
        //std::cout << "Mouse released on " << CurrentSq << "\n";

        if ( !m_MoveHandling.PieceHeld.IsNullPiece() && CurrentSq != m_MoveHandling.SelectedSquare )  // Check for making a move
        {
            if ( m_LegalMovesDict.contains( m_MoveHandling.SelectedSquare ) )
            {
                Chess::Move NewMove = IsValidMove( m_MoveHandling.SelectedSquare, CurrentSq );
                if ( !NewMove.IsNullMove() ) // Future check for valid move
                {
                    m_BoardVariables.NextMove = NewMove;
                    if ( (NewMove.Flag() & Chess::MoveFlag::PromoteKnight) != Chess::MoveFlag::None )   // Promoting
                    {
                        m_PromotionHandling.Promoting = true;
                        m_PromotionHandling.PromotionScreenPos = ImGui::GetCursorScreenPos();
                        m_MoveHandling.SelectedPressed = false;
                        return;
                    }
                    m_MoveHandling.SelectedSquare = -1;
                    m_MoveHandling.PieceHeld.MakeNullPiece();
                    m_MoveHandling.SelectedPressed = false;
                }
            }
        }

        if ( m_MoveHandling.SelectedPressed && CurrentSq == m_MoveHandling.SelectedSquare )
        {
            m_MoveHandling.SelectedSquare = -1;
            m_MoveHandling.SelectedPressed = false;
        }
        m_MoveHandling.PieceHeld = Chess::ChessPiece();
    }
    else if ( ImGui::IsMouseClicked( 1 ) && !m_PromotionHandling.Promoting )  // Highlight logic
    {
        if ( m_BoardVisuals.HighlightedSquares.contains( CurrentSq ) )
            m_BoardVisuals.HighlightedSquares.erase( CurrentSq );
        else
            m_BoardVisuals.HighlightedSquares.insert( CurrentSq );
    }
}

void ChessApp::DrawPieceSelected( ImDrawList* DrawList ) const
{
    if ( !m_MoveHandling.PieceHeld.IsNullPiece() && ImGui::IsMouseDown(0) )
    {
        const Image& image = m_PieceImages[(int)m_MoveHandling.PieceHeld.color].at( m_MoveHandling.PieceHeld.type);   // Stupid en virkar
        ImVec2 MousePos = ImGui::GetMousePosOnOpeningCurrentPopup();

        float size = m_BoardVisuals.CellSize / 2;
        ImVec2 UpLeft( MousePos.x - size, MousePos.y - size );
        ImVec2 DownRight( MousePos.x + size, MousePos.y + size );

        DrawList->AddImage( image.Texture, UpLeft, DownRight );
    }
}

void ChessApp::DrawLegalTargets( ImDrawList* DrawList ) const
{
    if ( m_MoveHandling.SelectedSquare == -1 )
        return;

    const auto& info = m_LegalMovesDict.at( m_MoveHandling.SelectedSquare );

    for ( auto& square : info.Targets )
    {

    }

}

void ChessApp::MakeMove()
{
    const Chess::Move& move = m_BoardVariables.NextMove;
    std::cout << "Move( " << move.Start() << ", " << move.Target() << " )\n";
    m_Chessboard.MakeMove( move );
    m_BoardVisuals.LastMove = move;

    if ( m_BoardVariables.BoardInfo._WhiteToPlay )
        m_BlackClock.UpdateLastPoll();
    else
        m_WhiteClock.UpdateLastPoll();

    UpdateBoardInfo();
    Chess::MoveFlag flag = move.Flag();
    if ( (flag & Chess::MoveFlag::Capture) != Chess::MoveFlag::None )
    {
        m_SoundEngine->play2D( (SoundsDir / "capture.mp3").string().c_str() );
    }
    else if ( (flag & Chess::MoveFlag::PromoteKnight) != Chess::MoveFlag::None )
    {
        m_SoundEngine->play2D( (SoundsDir / "promote.mp3").string().c_str() );
    }
    else if ( (flag & Chess::MoveFlag::CastleKing) != Chess::MoveFlag::None )
    {
        m_SoundEngine->play2D( (SoundsDir / "castle.mp3").string().c_str() );
    }
    else
    {
        m_SoundEngine->play2D( (SoundsDir / "move-self.mp3").string().c_str() );
    }

    m_BoardVisuals.HighlightedSquares.clear();
    m_MoveHandling = MoveHandling();

    m_BoardVariables.NextMove = Chess::Move();

    if ( m_BoardVisuals.AutoFlip )
        m_BoardVisuals.FlipBoard = !m_BoardVisuals.FlipBoard;
}

void ChessApp::UnMakeMove()
{
    m_Chessboard.UnMakeMove();
    m_BoardVisuals.LastMove = m_Chessboard.GetLastMove();
    m_MoveHandling = MoveHandling();
    UpdateBoardInfo();
}

void ChessApp::DrawDebugScreen()
{
    using namespace std::chrono;

    ImGuiIO& io = ImGui::GetIO();

    ImGui::Begin( "Debug" );

    ImGui::SeparatorText( "Basic info" );
    ImGui::Text( "FPS: %f", io.Framerate );
    ImGui::Text( "Current font used: %s", io.Fonts->Fonts[0]->ConfigData->Name );

    ImGui::SeparatorText( "Chessboard window Info" );

    if ( m_MoveHandling.SelectedSquare != -1 )
        ImGui::Text( "Square selected: %d", m_MoveHandling.SelectedSquare );
    else
        ImGui::Text( "Square selected: None " );

    ImGui::Text( "Piece held: %s", m_MoveHandling.PieceHeld.GetPieceRepr().c_str());


    ImGui::SeparatorText( "Engine" );

    ImGui::Text( "%s", m_BoardVariables.GameStarted ? "Game started" : "Game paused" );
    std::string ButtonText = m_BoardVariables.GameStarted ? "Pause game" : "Start game";
    if ( ImGui::Button( ButtonText.c_str() ) )
    {
        m_BoardVariables.GameStarted = !m_BoardVariables.GameStarted;
        if ( m_BoardVariables.BoardInfo._WhiteToPlay )
            m_WhiteClock.UpdateLastPoll();
        else
            m_BlackClock.UpdateLastPoll();
    }
    ImGui::NewLine();

    ImGui::Text( "%s to play", m_BoardVariables.BoardInfo._WhiteToPlay ? "White" : "Black" );
    ImGui::Text( "En Passant square: %s | %d", m_BoardVariables.BoardInfo._EnPassantSquare == -1 ? "None" : Chess::GetSquareRepr( m_BoardVariables.BoardInfo._EnPassantSquare ), m_BoardVariables.BoardInfo._EnPassantSquare );
    ImGui::Text( "Fullmove clock: %d", m_BoardVariables.BoardInfo._FullmoveClock );
    ImGui::Text( "Halfmove clock: %d", m_BoardVariables.BoardInfo._HalfmoveClock );
    ImGui::Text( "White castling rights: %s", GetCastlingString( m_BoardVariables.BoardInfo._WhiteCastling ).c_str() );
    ImGui::Text( "Black castling rights: %s", GetCastlingString( m_BoardVariables.BoardInfo._BlackCastling ).c_str() );
    ImGui::Text( "White king position: %d", m_BoardVariables.BoardInfo._WhiteKingPos );
    ImGui::Text( "Black king position: %d", m_BoardVariables.BoardInfo._BlackKingPos );

    if ( ImGui::Button( "Load default position" ) )
    {
        ResetBoard();
    }

    if ( ImGui::Button( "Undo last move" ) )
    {
        UnMakeMove();
    }

    if ( ImGui::InputText( "Load new FEN position", m_BoardVariables.NewFen, IM_ARRAYSIZE( m_BoardVariables.NewFen ), ImGuiInputTextFlags_EnterReturnsTrue ) )
    {
        std::cout << "Loading FEN: " << m_BoardVariables.NewFen << "\n";
        m_Chessboard.LoadFEN( std::string( m_BoardVariables.NewFen ) );
        UpdateBoardInfo();
        memset( m_BoardVariables.NewFen, 0, sizeof( m_BoardVariables.NewFen ) );
    }

    ImGui::SeparatorText( "Chessboard Variables" );

    ImGui::ColorEdit4( "Even Color", &Settings.Colors.EvenColor.Value.x );
    ImGui::ColorEdit4( "Even Highlight", &Settings.Colors.EvenColorHighlight.Value.x );
    ImGui::ColorEdit4( "Even Color Red", &Settings.Colors.EvenColorRed.Value.x );

    ImGui::ColorEdit4( "Odd Color", &Settings.Colors.OddColor.Value.x );
    ImGui::ColorEdit4( "Odd Highlight", &Settings.Colors.OddColorHighlight.Value.x );
    ImGui::ColorEdit4( "Odd Color Red", &Settings.Colors.OddColorRed.Value.x );

    ImGui::DragFloat( "Cell size", &m_BoardVisuals.CellSize, 1.f, 50.f, 150.f);

    ImGui::Checkbox( "Flip board", &m_BoardVisuals.FlipBoard );
    ImGui::Checkbox( "Auto flip board", &m_BoardVisuals.AutoFlip );

    ImGui::SeparatorText( "Time" );

    auto time = round<nanoseconds>( 
        duration<float>( io.DeltaTime ) 
    );
    ImGui::TextUnformatted( std::format( "Deltatime: {:%S}s", time ).c_str() );

    ImGui::Text( "Time left for white: %s", FormatMs( m_WhiteClock.GetTimeLeft() ).c_str() );
    ImGui::InputInt( "Set time for white (ms)", &m_BoardVariables.NewWhiteTime, 100 );
    if ( ImGui::IsItemDeactivatedAfterEdit() )
    {
        if ( m_BoardVariables.NewWhiteTime >= 0 )
            m_WhiteClock.SetTimeLeft( milliseconds( m_BoardVariables.NewWhiteTime ) );
        m_BoardVariables.NewWhiteTime = 0;
    }

    ImGui::NewLine();
    
    ImGui::Text( "Time left for black: %s", FormatMs( m_BlackClock.GetTimeLeft() ).c_str() );
    ImGui::InputInt( "Set time for black (ms)", &m_BoardVariables.NewBlackTime, 100 );
    if ( ImGui::IsItemDeactivatedAfterEdit() )
    {
        if ( m_BoardVariables.NewBlackTime >= 0 )
            m_BlackClock.SetTimeLeft( milliseconds( m_BoardVariables.NewBlackTime ) );
        m_BoardVariables.NewBlackTime = 0;
    }

    ImGui::End();
}

void ChessApp::ResetBoard()
{
    m_Chessboard.LoadFEN( "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" );
    UpdateBoardInfo();

    m_BoardVisuals.HighlightedSquares.clear();
    m_MoveHandling.SelectedSquare = -1;
    m_MoveHandling.PieceHeld.MakeNullPiece();

    m_BoardVariables.NextMove = Chess::Move();
}

void ChessApp::UpdateBoardInfo()
{
    m_BoardVariables.BoardInfo = m_Chessboard.GetBoardInfo();
    CreateMoveDict();
}

void ChessApp::CreateMoveDict()
{
    m_LegalMovesDict.clear();

    for ( auto& move : m_BoardVariables.BoardInfo._LegalMoves )
    {
        int Start = move.Start();

        if ( !m_LegalMovesDict.contains( Start ) )
        {
            m_LegalMovesDict[Start] = LegalMovesInfo();
            m_LegalMovesDict[Start].Moves.reserve( 32 );
            m_LegalMovesDict[Start].Targets.reserve( 32 );
        }
        m_LegalMovesDict[Start].Moves.insert( move );
        m_LegalMovesDict[Start].Targets.insert( move.Target() );
    }
}

void ChessApp::LoadFonts()
{
    namespace fs = std::filesystem;
    ImGuiIO& io = ImGui::GetIO();
    //io.Fonts->AddFontDefault();

    std::cout << "Loading fonts... ";

    fs::path FontsPath = ".\\Assets\\Fonts";
    
    fs::path Noto_SansPath = FontsPath / fs::path( "Noto_Sans\\static\\NotoSans-Regular.ttf" );
    fs::path GabaritoPath = FontsPath / fs::path( "Gabarito\\static\\Gabarito-Regular.ttf" );
    fs::path LexendPath = FontsPath / fs::path( "Lexend\\static\\Lexend-Regular.ttf" );
    fs::path OutfitPath = FontsPath / fs::path( "Outfit\\static\\Outfit-Regular.ttf" );

    Settings.Fonts.Gabarito = io.Fonts->AddFontFromFileTTF( GabaritoPath.string().c_str(), 24.0f );
    Settings.Fonts.Noto_Sans = io.Fonts->AddFontFromFileTTF( Noto_SansPath.string().c_str(), 24.0f );
    Settings.Fonts.Lexend = io.Fonts->AddFontFromFileTTF( LexendPath.string().c_str(), 24.0f );
    Settings.Fonts.Outfit = io.Fonts->AddFontFromFileTTF( OutfitPath.string().c_str(), 24.0f );

    io.Fonts->Build();

    std::cout << "Fonts loaded!\n";
}

void ChessApp::LoadPieceImages()
{
    namespace fs = std::filesystem;
    for ( int c = 0; c < 2; ++c )
    {
        Chess::Color color = (Chess::Color)c;
        for ( int p = 1; p < 7; ++p )
        {
            Chess::PieceType piece = (Chess::PieceType)p;
            fs::path ImagePath = PiecesDir / GetPieceFileName( piece, color );

            Image image;
            if ( !LoadImageTexture( ImagePath, &image, 0.67f ) )
            {
                std::cerr << "Error loading image!\n";
                throw std::exception();
            }
            m_PieceImages[c][piece] = image;
        }
    }
}

void ChessApp::SetupDockspace()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos( viewport->WorkPos );
    ImGui::SetNextWindowSize( viewport->WorkSize );
    ImGui::SetNextWindowViewport( viewport->ID );

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0.0f );
    ImGui::PushStyleVar( ImGuiStyleVar_WindowBorderSize, 0.0f );

    ImGui::Begin( "MainDockSpaceWindow", NULL, window_flags );
    ImGui::PopStyleVar( 2 );

    // Set the docking space
    ImGuiID dockspace_id = ImGui::GetID( "MainDockSpace" );
    ImGui::DockSpace( dockspace_id, ImVec2( 0.0f, 0.0f ), ImGuiDockNodeFlags_PassthruCentralNode );
    ImGui::End();
}



ChessApp::ChessApp()
{
    using namespace std::chrono_literals;

    if ( !glfwInit() )
    {
        std::cerr << "Failed to initialize GLFW.\n";
        throw std::exception();
    }

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

    m_Window = glfwCreateWindow( 1920, 1080, "Chess Manager", NULL, NULL );
    if ( m_Window == NULL )
    {
        std::cerr << "Failed to create GLFW window.\n";
        throw std::exception();
    }
    glfwMakeContextCurrent( m_Window );
    glfwSwapInterval( 1 );

    if ( glewInit() != GLEW_OK )
    {
        std::cerr << "Error!" << std::endl;
        glfwTerminate();
        throw std::exception();
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    // Enable Docking and Viewports
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    if ( m_EnableViewports )
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;     // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL( m_Window, true );
    ImGui_ImplOpenGL3_Init( "#version 330 core" );

    printf( "OpenGL version: %s\n", glGetString( GL_VERSION ) );

    m_SoundEngine = irrklang::createIrrKlangDevice();

    LoadFonts();
    LoadPieceImages();

    //m_Chessboard.LoadFEN( "R1r4k/6b1/8/4B3/2N5/2K5/8/8 w - - 0 1" );

    UpdateBoardInfo();
}

ChessApp::~ChessApp()
{
    // Cleanup

    m_SoundEngine->drop();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow( m_Window );
    glfwTerminate();
}
void ChessApp::Run()
{
    // Main loop
    while ( !glfwWindowShouldClose( m_Window ) )
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiIO& io = ImGui::GetIO();

        if ( m_BoardVariables.GameStarted )
        {
            if ( m_BoardVariables.BoardInfo._WhiteToPlay )
                m_WhiteClock.Update();
            else
                m_BlackClock.Update();
        }

        // Create a fullscreen dock space
        SetupDockspace();

        DrawDebugScreen();

        //ImGui::ShowDemoWindow();

        //if ( ImGui::Begin( "Example Window" ) )
        //{
        //    // Begin a child window of a fixed size, enabling the border.
        //    // The size can be set to fit just the text, or you can size it as desired.
        //    ImU32 FillColor = IM_COL32( 150, 100, 200, 255 );
        //    ImVec4 var = ImGui::ColorConvertU32ToFloat4( FillColor );
        //    ImGui::PushStyleColor( ImGuiCol_ChildBg, var );
        //    ImGui::BeginChild( "TextBox", ImVec2( 150, 40 ), ImGuiChildFlags_Border /*border*/ );

        //    ImGui::TextColored( ImVec4( 0.f, 0.f, 0.f, 1.f ), "Hello, ImGui!" );
        //    
        //    ImGui::EndChild();
        //    ImGui::PopStyleColor();
        //}
        //ImGui::End();

        DrawChessboardScreen();
        if ( !m_BoardVariables.NextMove.IsNullMove() && !m_PromotionHandling.Promoting )
        {
            MakeMove();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize( m_Window, &display_w, &display_h );
        glViewport( 0, 0, display_w, display_h );
        glClearColor( 0.45f, 0.55f, 0.60f, 1.00f );
        glClear( GL_COLOR_BUFFER_BIT );

        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

        if ( m_EnableViewports && (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) )
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent( backup_current_context );
        }

        glfwSwapBuffers( m_Window );
    }
}