#include <iostream>
#include <filesystem>
#include <format>
#include <chrono>
#include <array>
#include <stdio.h>

#include "Application.h"


static const std::filesystem::path AssetsDir( "Assets" );
static const std::filesystem::path PiecesDir = AssetsDir / "Pieces";

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
    if ( !m_BoardSettings.FlipBoard )
        DrawChessBoard();
    else
        DrawChessBoardFlipped();

    // Draw the selected piece on the mouse
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    DrawPieceSelected( drawList );

    ImGui::End();
}

void ChessApp::DrawChessBoard()
{
    if ( ImGui::IsWindowFocused() && ImGui::IsMouseClicked( 0 ) )
        m_BoardSettings.HighlightedSquares.clear();

    const int BoardSize = 8;

    const std::array<Chess::ChessPiece, 64>& Board = m_BoardSettings.BoardInfo._Board;
    //const Chess::Board& Board = m_Engine.GetBoard();
    
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
            Chess::ChessPiece Piece = Board[CurrSq];

            ImColor CellColor;
            if ( CurrSq == m_BoardSettings.SelectedSquare )
                CellColor = isDark ? Settings.Colors.EvenColorHighlight : Settings.Colors.OddColorHighlight;
            else if ( m_BoardSettings.HighlightedSquares.contains( CurrSq ) )
                CellColor = isDark ? Settings.Colors.EvenColorRed : Settings.Colors.OddColorRed;
            else
                CellColor = isDark ? Settings.Colors.EvenColor : Settings.Colors.OddColor;

            ImU32 CellCol32 = CellColor;

            float MoveCircleRadius = 15.0f;

            ImGui::TableNextColumn();
            //ImGui::TableSetColumnIndex( col ); // Virkar ekki af einhverjum ástæðum?

            if (
                (Piece.IsNullPiece()) ||
                (CurrSq == m_BoardSettings.SelectedSquare && m_BoardSettings.PieceHeld.type != Chess::PieceType::None)
                )   // Þetta er hræðilegt if statement
            {
                ImGui::Dummy( ImVec2( m_BoardSettings.CellSize, m_BoardSettings.CellSize ) ); // Dummy widget fyrir mouse clicks
            }
            else
            {
                const Image& image = m_PieceImages[(int)Piece.color][Piece.type];
                ImGui::Image( image.Texture, ImVec2( m_BoardSettings.CellSize, m_BoardSettings.CellSize ) );   // Annars teiknum við myndina
            }
            if ( ImGui::IsItemHovered() )
            {
                HandleBoardClicks( Piece, CurrSq );
                MoveCircleRadius = 21.0f;
            }

            ImGui::TableSetBgColor( ImGuiTableBgTarget_CellBg, CellCol32 );
        }
    }
    ImGui::EndTable();
    ImGui::PopStyleVar();
}

void ChessApp::DrawChessBoardFlipped()
{
    if ( ImGui::IsWindowFocused() && ImGui::IsMouseClicked( 0 ) )
        m_BoardSettings.HighlightedSquares.clear();

    const int BoardSize = 8;

    const std::array<Chess::ChessPiece, 64>& Board = m_BoardSettings.BoardInfo._Board;
    //const Chess::Board& Board = m_Engine.GetBoard();

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
            int col_inverted = 7 - col;

            int CurrSq = row * 8 + col_inverted;
            Chess::ChessPiece Piece = Board[CurrSq];

            ImColor CellColor;
            if ( CurrSq == m_BoardSettings.SelectedSquare )
                CellColor = isDark ? Settings.Colors.EvenColorHighlight : Settings.Colors.OddColorHighlight;
            else if ( m_BoardSettings.HighlightedSquares.contains( CurrSq ) )
                CellColor = isDark ? Settings.Colors.EvenColorRed : Settings.Colors.OddColorRed;
            else
                CellColor = isDark ? Settings.Colors.EvenColor : Settings.Colors.OddColor;

            ImU32 CellCol32 = CellColor;

            float MoveCircleRadius = 15.0f;

            ImGui::TableNextColumn();
            //ImGui::TableSetColumnIndex( col ); // Virkar ekki af einhverjum ástæðum?

            if (
                (Piece.IsNullPiece()) ||
                (CurrSq == m_BoardSettings.SelectedSquare && m_BoardSettings.PieceHeld.type != Chess::PieceType::None)
                )   // Þetta er hræðilegt if statement
            {
                ImGui::Dummy( ImVec2( m_BoardSettings.CellSize, m_BoardSettings.CellSize ) ); // Dummy widget fyrir mouse clicks
            }
            else
            {
                const Image& image = m_PieceImages[(int)Piece.color][Piece.type];
                ImGui::Image( image.Texture, ImVec2( m_BoardSettings.CellSize, m_BoardSettings.CellSize ) );   // Annars teiknum við myndina
            }
            if ( ImGui::IsItemHovered() )
            {
                HandleBoardClicks( Piece, CurrSq );
                MoveCircleRadius = 21.0f;
            }

            ImGui::TableSetBgColor( ImGuiTableBgTarget_CellBg, CellCol32 );
        }
    }
    ImGui::EndTable();
    ImGui::PopStyleVar();
}

void ChessApp::HandleBoardClicks( Chess::ChessPiece Piece, int CurrentSq )
{
    if ( ImGui::IsMouseClicked( 0 ) )
    {
        std::cout << "Mouse clicked on " << CurrentSq << "\n";
        if ( m_BoardSettings.SelectedSquare != -1 && CurrentSq != m_BoardSettings.SelectedSquare )  // Check for making a move
        {
            if ( true ) // Future check for valid move
            {
                MakeMove( m_BoardSettings.SelectedSquare, CurrentSq );
                m_BoardSettings.SelectedSquare = -1;
                m_BoardSettings.PieceHeld.MakeNullPiece();
            }
            return;
        }
        
        if ( !Piece.IsNullPiece() ) // Clicked a piece
        {
            if ( CurrentSq == m_BoardSettings.SelectedSquare )
                m_BoardSettings.SelectedPressed = true;
            else
                m_BoardSettings.SelectedSquare = CurrentSq;
            m_BoardSettings.PieceHeld = Piece;
        }
        else
        {
            m_BoardSettings.SelectedSquare = -1;
            m_BoardSettings.PieceHeld.MakeNullPiece();
        }
    }
    else if ( ImGui::IsMouseReleased( 0 ) )
    {
        std::cout << "Mouse released on " << CurrentSq << "\n";

        if ( !m_BoardSettings.PieceHeld.IsNullPiece() && CurrentSq != m_BoardSettings.SelectedSquare )  // Check for making a move
        {
            if ( true ) // Future check for valid move
            {
                MakeMove( m_BoardSettings.SelectedSquare, CurrentSq );

                m_BoardSettings.SelectedSquare = -1;
                m_BoardSettings.PieceHeld.MakeNullPiece();
                m_BoardSettings.SelectedPressed = false;
            }
            return;
        }

        if ( m_BoardSettings.SelectedPressed && CurrentSq == m_BoardSettings.SelectedSquare )
        {
            m_BoardSettings.SelectedSquare = -1;
            m_BoardSettings.SelectedPressed = false;
        }
        m_BoardSettings.PieceHeld = Chess::ChessPiece();
    }
    else if ( ImGui::IsMouseClicked( 1 ) )  // Highlight logic
    {
        if ( m_BoardSettings.HighlightedSquares.contains( CurrentSq ) )
            m_BoardSettings.HighlightedSquares.erase( CurrentSq );
        else
            m_BoardSettings.HighlightedSquares.insert( CurrentSq );
    }
}

void ChessApp::DrawPieceSelected( ImDrawList* DrawList ) const
{
    if ( !m_BoardSettings.PieceHeld.IsNullPiece() && ImGui::IsMouseDown(0) )
    {
        const Image& image = m_PieceImages[(int)m_BoardSettings.PieceHeld.color].at( m_BoardSettings.PieceHeld.type);   // Stupid en virkar
        ImVec2 MousePos = ImGui::GetMousePosOnOpeningCurrentPopup();

        float size = m_BoardSettings.CellSize;
        ImVec2 UpLeft( MousePos.x - size / 2, MousePos.y - size / 2 );
        ImVec2 DownRight( MousePos.x + size / 2, MousePos.y + size / 2 );

        DrawList->AddImage( image.Texture, UpLeft, DownRight );
    }
}

void ChessApp::MakeMove( int Start, int Target, Chess::MoveFlag flag )
{
    std::cout << "Move( " << Start << ", " << Target << " )\n";
    Chess::Move move( Start, Target, flag );
    m_Chessboard.MakeMove( move );


    if ( m_BoardSettings.BoardInfo._WhiteToPlay )
        m_BlackClock.UpdateLastPoll();
    else
        m_WhiteClock.UpdateLastPoll();

    m_BoardSettings.BoardInfo = m_Chessboard.GetBoardInfo();
    m_SoundEngine->play2D( "Assets/Sounds/move-self.mp3" );

    if ( m_BoardSettings.AutoFlip )
        m_BoardSettings.FlipBoard = !m_BoardSettings.FlipBoard;
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

    if ( m_BoardSettings.SelectedSquare != -1 )
        ImGui::Text( "Square selected: %d", m_BoardSettings.SelectedSquare );
    else
        ImGui::Text( "Square selected: None " );

    ImGui::Text( "Piece held: %s", m_BoardSettings.PieceHeld.GetPieceRepr().c_str());


    ImGui::SeparatorText( "Engine" );

    ImGui::Text( "%s", m_BoardSettings.GameStarted ? "Game started" : "Game paused" );
    std::string ButtonText = m_BoardSettings.GameStarted ? "Pause game" : "Start game";
    if ( ImGui::Button( ButtonText.c_str() ) )
    {
        m_BoardSettings.GameStarted = !m_BoardSettings.GameStarted;
        if ( m_BoardSettings.BoardInfo._WhiteToPlay )
            m_WhiteClock.UpdateLastPoll();
        else
            m_BlackClock.UpdateLastPoll();
    }
    ImGui::NewLine();

    ImGui::Text( "%s to play", m_BoardSettings.BoardInfo._WhiteToPlay ? "White" : "Black" );
    ImGui::Text( "En Passant square: %s | %d", m_BoardSettings.BoardInfo._EnPassantSquare == -1 ? "None" : Chess::GetSquareRepr( m_BoardSettings.BoardInfo._EnPassantSquare ), m_BoardSettings.BoardInfo._EnPassantSquare );
    ImGui::Text( "Fullmove clock: %d", m_BoardSettings.BoardInfo._FullmoveClock );
    ImGui::Text( "Halfmove clock: %d", m_BoardSettings.BoardInfo._HalfmoveClock );
    ImGui::Text( "White castling rights: %s", GetCastlingString( m_BoardSettings.BoardInfo._WhiteCastling ).c_str() );
    ImGui::Text( "Black castling rights: %s", GetCastlingString( m_BoardSettings.BoardInfo._BlackCastling ).c_str() );

    if ( ImGui::Button( "Load default position" ) )
    {
        ResetBoard();
    }

    if ( ImGui::Button( "Undo last move" ) )
    {
        m_Chessboard.UnMakeMove();
        m_BoardSettings.BoardInfo = m_Chessboard.GetBoardInfo();
    }

    if ( ImGui::InputText( "Load new FEN position", m_BoardSettings.NewFen, IM_ARRAYSIZE( m_BoardSettings.NewFen ), ImGuiInputTextFlags_EnterReturnsTrue ) )
    {
        std::cout << "Loading FEN: " << m_BoardSettings.NewFen << "\n";
        //m_Engine.LoadFEN( std::string( m_BoardSettings.NewFen ) );
        m_Chessboard.LoadFEN( std::string( m_BoardSettings.NewFen ) );
        //m_BoardSettings.EngineInfo = m_Engine.GetEngineInfo();
        m_BoardSettings.BoardInfo = m_Chessboard.GetBoardInfo();
        memset( m_BoardSettings.NewFen, 0, sizeof( m_BoardSettings.NewFen ) );
    }

    if ( ImGui::Button( "Play test move" ) )
    {
        MakeMove( 36, 45, Chess::MoveFlag::EnPassant );
    }

    ImGui::SeparatorText( "Chessboard Variables" );

    ImGui::ColorEdit4( "Even Color", &Settings.Colors.EvenColor.Value.x );
    ImGui::ColorEdit4( "Even Highlight", &Settings.Colors.EvenColorHighlight.Value.x );
    ImGui::ColorEdit4( "Even Color Red", &Settings.Colors.EvenColorRed.Value.x );

    ImGui::ColorEdit4( "Odd Color", &Settings.Colors.OddColor.Value.x );
    ImGui::ColorEdit4( "Odd Highlight", &Settings.Colors.OddColorHighlight.Value.x );
    ImGui::ColorEdit4( "Odd Color Red", &Settings.Colors.OddColorRed.Value.x );

    ImGui::DragFloat( "Cell size", &m_BoardSettings.CellSize, 1.f, 50.f, 150.f);

    ImGui::Checkbox( "Flip board", &m_BoardSettings.FlipBoard );
    ImGui::Checkbox( "Auto flip board", &m_BoardSettings.AutoFlip );

    ImGui::SeparatorText( "Time" );

    auto time = round<nanoseconds>( 
        duration<float>( io.DeltaTime ) 
    );
    ImGui::TextUnformatted( std::format( "Deltatime: {:%S}s", time ).c_str() );

    ImGui::Text( "Time left for white: %s", FormatMs( m_WhiteClock.GetTimeLeft() ).c_str() );
    ImGui::InputInt( "Set time for white (ms)", &m_BoardSettings.NewWhiteTime, 100 );
    if ( ImGui::IsItemDeactivatedAfterEdit() )
    {
        if ( m_BoardSettings.NewWhiteTime >= 0 )
            m_WhiteClock.SetTimeLeft( milliseconds( m_BoardSettings.NewWhiteTime ) );
        m_BoardSettings.NewWhiteTime = 0;
    }

    ImGui::NewLine();
    
    ImGui::Text( "Time left for black: %s", FormatMs( m_BlackClock.GetTimeLeft() ).c_str() );
    ImGui::InputInt( "Set time for black (ms)", &m_BoardSettings.NewBlackTime, 100 );
    if ( ImGui::IsItemDeactivatedAfterEdit() )
    {
        if ( m_BoardSettings.NewBlackTime >= 0 )
            m_BlackClock.SetTimeLeft( milliseconds( m_BoardSettings.NewBlackTime ) );
        m_BoardSettings.NewBlackTime = 0;
    }

    ImGui::End();
}

void ChessApp::ResetBoard()
{
    m_Chessboard.LoadFEN( "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" );
    m_BoardSettings.BoardInfo = m_Chessboard.GetBoardInfo();
    
    /*m_Engine.LoadFEN( "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" );
    m_BoardSettings.EngineInfo = m_Engine.GetEngineInfo();*/

    m_BoardSettings.HighlightedSquares.clear();
    m_BoardSettings.SelectedSquare = -1;
    m_BoardSettings.PieceHeld.MakeNullPiece();
}

void ChessApp::LoadFonts()
{
    namespace fs = std::filesystem;
    ImGuiIO& io = ImGui::GetIO();
    //io.Fonts->AddFontDefault();

    std::cout << "Loading fonts... ";

    fs::path GabaritoPath = fs::path( ".\\Assets\\Fonts\\Gabarito\\static\\Gabarito-Regular.ttf" );
    fs::path LexendPath = fs::path( ".\\Assets\\Fonts\\Lexend\\static\\Lexend-Regular.ttf" );
    fs::path OutfitPath = fs::path( ".\\Assets\\Fonts\\Outfit\\static\\Outfit-Regular.ttf" );

    Settings.Fonts.Gabarito = io.Fonts->AddFontFromFileTTF( GabaritoPath.string().c_str(), 24.0f );
    Settings.Fonts.Lexend = io.Fonts->AddFontFromFileTTF( LexendPath.string().c_str(), 24.0f );
    Settings.Fonts.Outfit = io.Fonts->AddFontFromFileTTF( OutfitPath.string().c_str(), 24.0f );

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

    // Decide GL+GLSL versions
    // Here we pick OpenGL 3.3 Core Profile, but you can adjust as needed
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

    // Create window with GLFW
    m_Window = glfwCreateWindow( 1920, 1080, "Chess Manager", NULL, NULL );
    if ( m_Window == NULL )
    {
        std::cerr << "Failed to create GLFW window.\n";
        throw std::exception();
    }
    glfwMakeContextCurrent( m_Window );
    glfwSwapInterval( 1 ); // Enable vsync

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
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    if ( m_EnableViewports )
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Adjust style to make multi-viewport windows look better
    ImGuiStyle& style = ImGui::GetStyle();
    if ( m_EnableViewports && (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) )
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL( m_Window, true );
    ImGui_ImplOpenGL3_Init( "#version 330 core" );

    printf( "OpenGL version: %s\n", glGetString( GL_VERSION ) );

    m_SoundEngine = irrklang::createIrrKlangDevice();

    //auto time_start = 3620000ms;    // 01:00:20.000

    LoadFonts();
    LoadPieceImages();

    m_BoardSettings.BoardInfo = m_Chessboard.GetBoardInfo();

    /*m_Engine.LoadFEN( "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" );
    m_BoardSettings.EngineInfo = m_Engine.GetEngineInfo();*/
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

        if ( m_BoardSettings.GameStarted )
        {
            if ( m_BoardSettings.BoardInfo._WhiteToPlay )
                m_WhiteClock.Update();
            else
                m_BlackClock.Update();
        }

        // Create a fullscreen dock space
        SetupDockspace();

        DrawDebugScreen();

        //1ImGui::ShowDemoWindow();

        DrawChessboardScreen();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize( m_Window, &display_w, &display_h );
        glViewport( 0, 0, display_w, display_h );
        glClearColor( 0.45f, 0.55f, 0.60f, 1.00f );
        glClear( GL_COLOR_BUFFER_BIT );

        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

        // Update and Render additional Platform Windows
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
