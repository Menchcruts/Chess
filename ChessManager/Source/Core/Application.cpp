#include <iostream>
#include <filesystem>
#include <format>
#include <chrono>
#include <array>
#include <stdio.h>
#include <thread>

#include "Application.h"
#include "Logger.h"


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

    return { files[file], ranks[rank] };
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
    DrawChessBoard();

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
        m_BoardVisuals.HighlightedSquares = 0;

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

    const Chess::Bitboards& bitboards = m_Chessboard.m_Bitboards;
    
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

            bool isDark = ((row + col) % 2) == 1;

            int r, c;

            if ( m_BoardVisuals.FlipBoard )
            {
                r = row, c = 7 - col;
            }
            else
            {
                r = 7 - row, c = col;
            }

            int CurrSq = r * 8 + c;
            Chess::ChessPiece Piece = bitboards.GetPieceAtSquare(CurrSq);

            ImColor CellColor;
            if ( m_BoardVisuals.HighlightedSquares.IsOccupied( CurrSq ) || (m_BitboardSettings.ShowScreen && m_BitboardSettings.Result.IsOccupied( CurrSq )) )
                CellColor = isDark ? Settings->Colors.EvenColorRed : Settings->Colors.OddColorRed;

            else if ( CurrSq == m_MoveHandling.SelectedSquare || (!LastMove.IsNullMove() && (CurrSq == LastMove.Start() || CurrSq == LastMove.Target())) )
                CellColor = isDark ? Settings->Colors.EvenColorHighlight : Settings->Colors.OddColorHighlight;

            else
                CellColor = isDark ? Settings->Colors.EvenColor : Settings->Colors.OddColor;

            ImU32 CellCol32 = CellColor;

            ImGui::TableNextColumn();

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

    bool ClickedPiece = false;
    bool MenuGoDown = m_Chessboard.m_WhiteToPlay;
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
    
    ImGui::PushStyleColor( ImGuiCol_ButtonHovered, BackgroundColor );
    ImGui::PushStyleColor( ImGuiCol_ButtonActive, BackgroundColor );
    ImGui::PushStyleColor( ImGuiCol_Button, BackgroundColor );
    ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0, 0, 0, 255 ) );

    if ( m_Chessboard.m_WhiteToPlay )
    {
        if ( ImGui::ImageButton( "w_queen", m_PieceImages[0].at( Chess::PieceType::Queen ).Texture, Cell ) )
        {
            PromotionType = Chess::MoveFlag::PromoteQueen;
            ClickedPiece = true;
        }
        if ( ImGui::ImageButton( "w_rook", m_PieceImages[0].at( Chess::PieceType::Rook ).Texture, Cell ) )
        {
            PromotionType = Chess::MoveFlag::PromoteRook;
            ClickedPiece = true;
        }
        if ( ImGui::ImageButton( "w_bishop", m_PieceImages[0].at( Chess::PieceType::Bishop ).Texture, Cell ) )
        {
            PromotionType = Chess::MoveFlag::PromoteBishop;
            ClickedPiece = true;
        }
        if ( ImGui::ImageButton( "w_knight", m_PieceImages[0].at( Chess::PieceType::Knight ).Texture, Cell ) )
        {
            PromotionType = Chess::MoveFlag::PromoteKnight;
            ClickedPiece = true;
        }
        if ( ImGui::Button( "Cancel", ImVec2( CellSize, 0 ) ) )
        {
            std::cout << "Clicked off promoting!\n";
            m_MoveHandling.PieceHeld.MakeNullPiece();
            m_MoveHandling.SelectedSquare = -1;
            m_MoveHandling.SelectedPressed = false;

            m_PromotionHandling.Promoting = false;
            m_BoardVariables->NextMove = Chess::Move();
        }
    }
    else
    {
        if ( ImGui::Button( "Cancel", ImVec2( CellSize, 0 ) ) )
        {
            std::cout << "Clicked off promoting!\n";
            m_MoveHandling.PieceHeld.MakeNullPiece();
            m_MoveHandling.SelectedSquare = -1;
            m_MoveHandling.SelectedPressed = false;

            m_PromotionHandling.Promoting = false;
            m_BoardVariables->NextMove = Chess::Move();
        }
        if ( ImGui::ImageButton( "b_knight", m_PieceImages[1].at( Chess::PieceType::Knight ).Texture, Cell ) )
        {
            PromotionType = Chess::MoveFlag::PromoteKnight;
            ClickedPiece = true;
        }
        if ( ImGui::ImageButton( "b_bishop", m_PieceImages[1].at( Chess::PieceType::Bishop ).Texture, Cell ) )
        {
            PromotionType = Chess::MoveFlag::PromoteBishop;
            ClickedPiece = true;
        }
        if ( ImGui::ImageButton( "b_rook", m_PieceImages[1].at( Chess::PieceType::Rook ).Texture, Cell ) )
        {
            PromotionType = Chess::MoveFlag::PromoteRook;
            ClickedPiece = true;
        }
        if ( ImGui::ImageButton( "b_queen", m_PieceImages[1].at( Chess::PieceType::Queen ).Texture, Cell ) )
        {
            PromotionType = Chess::MoveFlag::PromoteQueen;
            ClickedPiece = true;
        }
    }

    ImGui::PopStyleColor( 4 );
    ImGui::PopStyleVar();

    ImGui::EndChild();
    ImGui::PopStyleColor();

    /*if ( ImGui::IsMouseClicked( 0 ) && !ClickedPiece )
    {
        std::cout << "Clicked off promoting!\n";
        m_MoveHandling.PieceHeld.MakeNullPiece();
        m_MoveHandling.SelectedSquare = -1;
        m_MoveHandling.SelectedPressed = false;

        m_PromotionHandling.Promoting = false;
        m_BoardVariables.NextMove = Chess::Move();
    }*/

    if ( PromotionType != Chess::MoveFlag::None )
    {
        Chess::Move NextMove = m_BoardVariables->NextMove;
        Chess::MoveFlag WasCapture = NextMove.Flag() & Chess::MoveFlag::Capture;
        PromotionType |= WasCapture;
        short Start = NextMove.Start();
        short Target = NextMove.Target();

        m_BoardVariables->NextMove = Chess::Move( Start, Target, PromotionType );
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
                    m_BoardVariables->NextMove = NewMove;
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
                    m_BoardVariables->NextMove = NewMove;
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
        m_MoveHandling = MoveHandling();
        if ( m_BoardVisuals.HighlightedSquares.IsOccupied( CurrentSq ) )
            m_BoardVisuals.HighlightedSquares.RemoveBit( CurrentSq );
        else
            m_BoardVisuals.HighlightedSquares.AddBit( CurrentSq );
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
    const Chess::Move& move = m_BoardVariables->NextMove;
    m_logger.info( "Move({0}, {1})", move.Start(), move.Target() );
    //std::cout << "Move( " << move.Start() << ", " << move.Target() << " )\n";
    m_Chessboard.MakeMove( move );
    m_BoardVisuals.LastMove = move;
    

    if ( m_Chessboard.m_WhiteToPlay )
        m_BlackClock.UpdateLastPoll();
    else
        m_WhiteClock.UpdateLastPoll();

    UpdateBoardInfo();
    Chess::MoveFlag flag = move.Flag();

    std::filesystem::path AudioDir = SoundsDir;
    if ( (flag & Chess::MoveFlag::Capture) != Chess::MoveFlag::None )
    {
        AudioDir /= "capture.mp3";
    }
    else if ( (flag & Chess::MoveFlag::PromoteKnight) != Chess::MoveFlag::None )
    {
        AudioDir /= "promote.mp3";
    }
    else if ( (flag & Chess::MoveFlag::CastleKing) != Chess::MoveFlag::None )
    {
        AudioDir /= "castle.mp3";
    }
    else
    {
        AudioDir /= "move-self.mp3";
    }
    
    ma_engine_play_sound( miniaudio_engine.get(), AudioDir.string().c_str(), NULL );

    m_BoardVariables->MoveHistory.emplace_back( move );  // Add move to history

    m_MoveHandling = MoveHandling();

    m_BoardVariables->NextMove = Chess::Move();

    if ( m_BoardVisuals.AutoFlip )
        m_BoardVisuals.FlipBoard = !m_BoardVisuals.FlipBoard;
}

void ChessApp::UnMakeMove()
{
    if ( m_BoardVariables->MoveHistory.size() <= 0 )
        return;

    Chess::Move move = m_BoardVariables->MoveHistory.back();
    m_BoardVariables->MoveHistory.pop_back();

    m_Chessboard.UnMakeMove( move );
    
    if ( m_BoardVariables->MoveHistory.size() > 0 )
        m_BoardVisuals.LastMove = m_BoardVariables->MoveHistory.back();
    else
        m_BoardVisuals.LastMove = Chess::Move();

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

    ImGui::Text( "%s", m_BoardVariables->GameStarted ? "Game started" : "Game paused" );
    std::string ButtonText = m_BoardVariables->GameStarted ? "Pause game" : "Start game";
    if ( ImGui::Button( ButtonText.c_str() ) )
    {
        m_BoardVariables->GameStarted = !m_BoardVariables->GameStarted;
        if ( m_Chessboard.m_WhiteToPlay )
            m_WhiteClock.UpdateLastPoll();
        else
            m_BlackClock.UpdateLastPoll();
    }
    ImGui::Text( "Number of legal moves: %d", m_BoardVariables->NumberOfLegalMoves );
    if ( ImGui::Button( "Run perft benchmark" ) )
    {
        m_PerftSettings.ShowScreen = true;
    }
    if ( ImGui::Button( "Open Bitboard selector" ) )
    {
        m_BitboardSettings = BitboardSettings();
        m_BitboardSettings.ShowScreen = true;
    }
    ImGui::NewLine();

    ImGui::Text( "%s to play", m_Chessboard.m_WhiteToPlay ? "White" : "Black" );
    ImGui::Text( "En Passant square: %s | %d", m_Chessboard.m_EnPassantSquare == -1 ? "None" : Chess::GetSquareRepr( m_Chessboard.m_EnPassantSquare ), m_Chessboard.m_EnPassantSquare );
    ImGui::Text( "Fullmove clock: %d", m_Chessboard.m_FullmoveClock );
    ImGui::Text( "Halfmove clock: %d", m_Chessboard.m_HalfmoveClock );
    ImGui::Text( "White castling rights: %s", GetCastlingString( m_Chessboard.m_WhiteCastling ).c_str() );
    ImGui::Text( "Black castling rights: %s", GetCastlingString( m_Chessboard.m_BlackCastling ).c_str() );
    ImGui::Text( "White king position: %d", m_Chessboard.m_Bitboards.KingWhite.BitscanForward() );
    ImGui::Text( "Black king position: %d", m_Chessboard.m_Bitboards.KingBlack.BitscanForward() );

    if ( ImGui::Button( "Load default position" ) )
    {
        ResetBoard();
    }

    if ( ImGui::Button( "Undo last move" ) )
    {
        UnMakeMove();
    }

    if ( ImGui::InputText( "Load new FEN position", m_BoardVariables->NewFen, IM_ARRAYSIZE( m_BoardVariables->NewFen ), ImGuiInputTextFlags_EnterReturnsTrue ) )
    {
        m_logger.info( "Loading FEN: '{}'", m_BoardVariables->NewFen );
        //std::cout << "Loading FEN: " << m_BoardVariables->NewFen << "\n";
        m_Chessboard.LoadFEN( std::string( m_BoardVariables->NewFen ) );
        UpdateBoardInfo();
        memset( m_BoardVariables->NewFen, 0, sizeof( m_BoardVariables->NewFen ) );
    }

    ImGui::SeparatorText( "Chessboard Variables" );

    ImGui::ColorEdit4( "Even Color", &Settings->Colors.EvenColor.Value.x );
    ImGui::ColorEdit4( "Even Highlight", &Settings->Colors.EvenColorHighlight.Value.x );
    ImGui::ColorEdit4( "Even Color Red", &Settings->Colors.EvenColorRed.Value.x );

    ImGui::ColorEdit4( "Odd Color", &Settings->Colors.OddColor.Value.x );
    ImGui::ColorEdit4( "Odd Highlight", &Settings->Colors.OddColorHighlight.Value.x );
    ImGui::ColorEdit4( "Odd Color Red", &Settings->Colors.OddColorRed.Value.x );

    ImGui::DragFloat( "Cell size", &m_BoardVisuals.CellSize, 1.f, 50.f, 150.f);

    ImGui::Checkbox( "Flip board", &m_BoardVisuals.FlipBoard );
    ImGui::Checkbox( "Auto flip board", &m_BoardVisuals.AutoFlip );

    ImGui::SeparatorText( "Time" );

    auto time = round<nanoseconds>( 
        duration<float>( io.DeltaTime ) 
    );
    ImGui::TextUnformatted( std::format( "Deltatime: {:%S}s", time ).c_str() );

    ImGui::Text( "Time left for white: %s", FormatMs( m_WhiteClock.GetTimeLeft() ).c_str() );
    ImGui::InputInt( "Set time for white (ms)", &m_BoardVariables->NewWhiteTime, 100 );
    if ( ImGui::IsItemDeactivatedAfterEdit() )
    {
        if ( m_BoardVariables->NewWhiteTime >= 0 )
            m_WhiteClock.SetTimeLeft( milliseconds( m_BoardVariables->NewWhiteTime ) );
        m_BoardVariables->NewWhiteTime = 0;
    }

    ImGui::NewLine();
    
    ImGui::Text( "Time left for black: %s", FormatMs( m_BlackClock.GetTimeLeft() ).c_str() );
    ImGui::InputInt( "Set time for black (ms)", &m_BoardVariables->NewBlackTime, 100 );
    if ( ImGui::IsItemDeactivatedAfterEdit() )
    {
        if ( m_BoardVariables->NewBlackTime >= 0 )
            m_BlackClock.SetTimeLeft( milliseconds( m_BoardVariables->NewBlackTime ) );
        m_BoardVariables->NewBlackTime = 0;
    }

    ImGui::End();
}

void ChessApp::ResetBoard()
{
    m_Chessboard.LoadFEN( "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" );
    UpdateBoardInfo();

    m_BoardVisuals.HighlightedSquares = 0;
    m_BoardVisuals.LastMove = Chess::Move();

    m_MoveHandling = MoveHandling();
    m_PromotionHandling = PromotionHandling();

    m_BoardVariables->NextMove = Chess::Move();
    m_BoardVariables->MoveHistory.clear();
    m_BoardVariables->GameStarted = false;
}

void ChessApp::UpdateBoardInfo()
{
    HandleMoveList();
}

void ChessApp::HandleMoveList()
{
    m_LegalMovesDict.clear();

    m_BoardVariables->LegalMoves = m_Chessboard.GetMoveList();
    m_BoardVariables->NumberOfLegalMoves = 0;

    int Start;
    for ( auto& move : m_BoardVariables->LegalMoves )
    {
        if ( move.IsNullMove() )
            break;
        ++m_BoardVariables->NumberOfLegalMoves;

        Start = move.Start();

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

void ChessApp::DrawBitboardScreen()
{
    m_BitboardSettings.Result = 0;
    const auto& bitboards = m_Chessboard.m_Bitboards;

    ImGui::Begin( "Bitboard selector", &m_BitboardSettings.ShowScreen );

    ImGui::BeginTable( "SelectorTable", 2 );
    ImGui::TableNextColumn();

    if ( ImGui::Button( "Toggle white" ) )
    {
        m_BitboardSettings.AllWhite = !m_BitboardSettings.AllWhite;

        m_BitboardSettings.WhiteKing = m_BitboardSettings.AllWhite;
        m_BitboardSettings.WhitePawn = m_BitboardSettings.AllWhite;
        m_BitboardSettings.WhiteKnight = m_BitboardSettings.AllWhite;
        m_BitboardSettings.WhiteBishop = m_BitboardSettings.AllWhite;
        m_BitboardSettings.WhiteRook = m_BitboardSettings.AllWhite;
        m_BitboardSettings.WhiteQueen = m_BitboardSettings.AllWhite;
    }
    ImGui::TableNextColumn();

    if ( ImGui::Button( "Toggle black" ) )
    {
        m_BitboardSettings.AllBlack = !m_BitboardSettings.AllBlack;

        m_BitboardSettings.BlackKing = m_BitboardSettings.AllBlack;
        m_BitboardSettings.BlackPawn = m_BitboardSettings.AllBlack;
        m_BitboardSettings.BlackKnight = m_BitboardSettings.AllBlack;
        m_BitboardSettings.BlackBishop = m_BitboardSettings.AllBlack;
        m_BitboardSettings.BlackRook = m_BitboardSettings.AllBlack;
        m_BitboardSettings.BlackQueen = m_BitboardSettings.AllBlack;
    }
    ImGui::TableNextColumn();

    ImGui::Checkbox( "White King", &m_BitboardSettings.WhiteKing );
    ImGui::TableNextColumn();

    ImGui::Checkbox( "Black King", &m_BitboardSettings.BlackKing );
    ImGui::TableNextColumn();

    ImGui::Checkbox( "White Pawn", &m_BitboardSettings.WhitePawn );
    ImGui::TableNextColumn();

    ImGui::Checkbox( "Black Pawn", &m_BitboardSettings.BlackPawn );
    ImGui::TableNextColumn();

    ImGui::Checkbox( "White Knight", &m_BitboardSettings.WhiteKnight );
    ImGui::TableNextColumn();

    ImGui::Checkbox( "Black Knight", &m_BitboardSettings.BlackKnight );
    ImGui::TableNextColumn();

    ImGui::Checkbox( "White Bishop", &m_BitboardSettings.WhiteBishop );
    ImGui::TableNextColumn();

    ImGui::Checkbox( "Black Bishop", &m_BitboardSettings.BlackBishop );
    ImGui::TableNextColumn();

    ImGui::Checkbox( "White Rook", &m_BitboardSettings.WhiteRook );
    ImGui::TableNextColumn();

    ImGui::Checkbox( "Black Rook", &m_BitboardSettings.BlackRook );
    ImGui::TableNextColumn();

    ImGui::Checkbox( "White Queen", &m_BitboardSettings.WhiteQueen );
    ImGui::TableNextColumn();

    ImGui::Checkbox( "Black Queen", &m_BitboardSettings.BlackQueen );

    ImGui::EndTable();
    
    /*if ( ImGui::Button( "Close selector" ) )
    {
        m_BitboardSettings.ShowScreen = false;
    }*/

    ImGui::End();

    {
        if ( m_BitboardSettings.WhiteKing )
        {
            m_BitboardSettings.Result ^= bitboards.KingWhite;
        }
        if ( m_BitboardSettings.WhitePawn )
        {
            m_BitboardSettings.Result ^= bitboards.PawnWhite;
        }
        if ( m_BitboardSettings.WhiteKnight )
        {
            m_BitboardSettings.Result ^= bitboards.KnightWhite;
        }
        if ( m_BitboardSettings.WhiteBishop )
        {
            m_BitboardSettings.Result ^= bitboards.BishopWhite;
        }
        if ( m_BitboardSettings.WhiteRook )
        {
            m_BitboardSettings.Result ^= bitboards.RookWhite;
        }
        if ( m_BitboardSettings.WhiteQueen )
        {
            m_BitboardSettings.Result ^= bitboards.QueenWhite;
        }

        if ( m_BitboardSettings.BlackKing )
        {
            m_BitboardSettings.Result ^= bitboards.KingBlack;
        }
        if ( m_BitboardSettings.BlackPawn )
        {
            m_BitboardSettings.Result ^= bitboards.PawnBlack;
        }
        if ( m_BitboardSettings.BlackKnight )
        {
            m_BitboardSettings.Result ^= bitboards.KnightBlack;
        }
        if ( m_BitboardSettings.BlackBishop )
        {
            m_BitboardSettings.Result ^= bitboards.BishopBlack;
        }
        if ( m_BitboardSettings.BlackRook )
        {
            m_BitboardSettings.Result ^= bitboards.RookBlack;
        }
        if ( m_BitboardSettings.BlackQueen )
        {
            m_BitboardSettings.Result ^= bitboards.QueenBlack;
        }
    }
}

void ChessApp::StartPerftTest()
{
    m_PerftSettings.Result = PerftResult();
    m_PerftSettings.ShowScreen = true;
    m_PerftSettings.CancelSearch = false;

    m_PerftSettings.Running = true;
    m_PerftSettings.Finished = false;

    m_PerftSettings.PerftThread = std::thread(
        [this]()
        {
            Chess::Chessboard TestBoard = Chess::Chessboard( this->m_Chessboard );
            this->m_PerftSettings.Result.Nodes = PerftTest( TestBoard, this->m_PerftSettings.InitalDepth, &this->m_PerftSettings.CancelSearch, this->m_PerftSettings.Verbose, &this->m_PerftSettings.Result );
            this->m_PerftSettings.Running = false;
            this->m_PerftSettings.Finished = true;

            Logger logger = Logger( "perft_result.log" );

            logger.info( "Running perft benchmark" );
            if ( !m_PerftSettings.CancelSearch )
            {
                logger.info( "Perft benchmark finished." );
                logger.info( "Nodes: {}", this->m_PerftSettings.Result.Nodes );
                logger.info( "Captures: {}", this->m_PerftSettings.Result.Captures );
                logger.info( "E.P.: {}", this->m_PerftSettings.Result.EnPassants );
                logger.info( "Castles: {}", this->m_PerftSettings.Result.Castles );
                logger.info( "Promotions: {}", this->m_PerftSettings.Result.Promotions );
            }
            else
                logger.info( "Perft benchmark canceled." );
        }
    );
}

int ChessApp::PerftTest( Chess::Chessboard& Board, int Depth, bool* Cancel, bool Verbose, PerftResult* Result )
{
    if ( Depth < 0 )
        throw std::exception( "wtf bro | Depth parameter for method Chessboard::Perft cannot be negative." );

    if ( Depth == 0 || (*Cancel) )
        return 1;

    int n_nodes = 0;
    int move_nodes = 0;
    std::array<Chess::Move, 218> Moves = Board.GetMoveList();
    for ( const auto& Move : Moves )
    {
        if ( Move.IsNullMove() )
            break;

        if ( Move.IsCapture() )
            ++Result->Captures;
        if ( Move.IsEnPassant() )
            ++Result->EnPassants;
        if ( Move.IsCastle() )
            ++Result->Castles;
        if ( Move.IsPromotion() )
            ++Result->Promotions;

        Board.MakeMove( Move );
        move_nodes = PerftTest( Board, Depth - 1, Cancel, false, Result );
        if ( Verbose && !(*Cancel) )
        {
            std::cout << Move.GetRepr() << ": " << move_nodes << "\n";
        }
        n_nodes += move_nodes;
        Board.UnMakeMove( Move );
    }
    return n_nodes;
}

void ChessApp::DrawPerftScreen()
{
    ImGui::Begin( "Perft Benchmark", &m_PerftSettings.ShowScreen );

    ImGui::DragInt( "Perft depth", &m_PerftSettings.InitalDepth, 0.025f, 1, 10 );
    ImGui::DragInt( "Expected result", &m_PerftSettings.ExpectedResult );
    ImGui::Checkbox( "Verbose", &m_PerftSettings.Verbose );

    if ( m_PerftSettings.Running )
    {
        ImGui::Text( "Perft result: Calculating..." );
    }
    else if ( m_PerftSettings.Finished )
    {
        ImGui::Text( "Perft result: %d nodes.", m_PerftSettings.Result.Nodes );
        if ( m_PerftSettings.ExpectedResult != 0 )
        {
            ImGui::SameLine();
            if ( m_PerftSettings.Result.Nodes == m_PerftSettings.ExpectedResult )
                ImGui::Text( "Benchmark passed." );
            else
                ImGui::Text( "Benchmark failed. Difference: %d", m_PerftSettings.Result.Nodes - m_PerftSettings.ExpectedResult );
        }
    }
    else
    {
        ImGui::Text( "Perft result: Start benchmark to see results." );
    }

    if ( ImGui::Button( "Start benchmark" ) )
    {
        StartPerftTest();
    }
    /*if ( ImGui::Button( "Close benchmark window" ) )
    {
        m_PerftSettings.ShowScreen = false;
    }*/

    ImGui::End();
}

void ChessApp::LoadFonts()
{
    namespace fs = std::filesystem;
    ImGuiIO& io = ImGui::GetIO();

    fs::path FontsPath = fs::path(".") / "Assets" / "Fonts";
    
    fs::path Noto_SansPath = FontsPath / fs::path( "Noto_Sans\\static\\NotoSans-Regular.ttf" );
    fs::path GabaritoPath = FontsPath / fs::path( "Gabarito\\static\\Gabarito-Regular.ttf" );
    fs::path LexendPath = FontsPath / fs::path( "Lexend\\static\\Lexend-Regular.ttf" );
    fs::path OutfitPath = FontsPath / fs::path( "Outfit\\static\\Outfit-Regular.ttf" );

    Settings->Fonts.Gabarito = io.Fonts->AddFontFromFileTTF( GabaritoPath.string().c_str(), 24.0f );
    Settings->Fonts.Noto_Sans = io.Fonts->AddFontFromFileTTF( Noto_SansPath.string().c_str(), 24.0f );
    Settings->Fonts.Lexend = io.Fonts->AddFontFromFileTTF( LexendPath.string().c_str(), 24.0f );
    Settings->Fonts.Outfit = io.Fonts->AddFontFromFileTTF( OutfitPath.string().c_str(), 24.0f );

    io.Fonts->Build();

    m_logger.info( "Fonts loaded" );
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
                m_logger.error( "Error loading image at path {}", ImagePath.string() );
                throw std::exception();
            }
            m_logger.debug( "Loaded image {}", ImagePath.string() );
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

    m_logger.info( "OpenGL version: {}", (const char*)glGetString( GL_VERSION ) );

    miniaudio_engine = std::make_unique<ma_engine>();
    miniaudio_result = ma_engine_init( NULL, miniaudio_engine.get() );
    if ( miniaudio_result != MA_SUCCESS )
    {
        m_logger.error( "Error initializing miniaudio soundengine" );
        throw std::runtime_error( "Error initializing miniaudio soundengine" );
    }

    Settings = new AppSettings();
    m_BoardVariables = new BoardVariables();

    LoadFonts();
    LoadPieceImages();

    m_BoardVariables->MoveHistory.reserve( 128 );

    UpdateBoardInfo();
}

ChessApp::~ChessApp()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow( m_Window );
    glfwTerminate();

    ma_engine_uninit( miniaudio_engine.get() );

    delete Settings;
    delete m_BoardVariables;

    if ( m_PerftSettings.PerftThread.joinable() )
    {
        m_PerftSettings.CancelSearch = true;
        m_logger.info( "Canceling perft benchmark in progress..." );
        m_PerftSettings.PerftThread.join();
    }
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

        if ( m_BoardVariables->GameStarted )
        {
            if ( m_Chessboard.m_WhiteToPlay )
                m_WhiteClock.Update();
            else
                m_BlackClock.Update();
        }

        if ( m_PerftSettings.Finished && m_PerftSettings.PerftThread.joinable() )
            m_PerftSettings.PerftThread.join();

        // Create a fullscreen dock space
        SetupDockspace();

        if ( m_PerftSettings.Finished )
        {
            m_PerftSettings.Running = false;
        }

        if ( m_PerftSettings.ShowScreen )
        {
            DrawPerftScreen();
        }
        if ( m_BitboardSettings.ShowScreen )
        {
            DrawBitboardScreen();
        }

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
        if ( !m_BoardVariables->NextMove.IsNullMove() && !m_PromotionHandling.Promoting )
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