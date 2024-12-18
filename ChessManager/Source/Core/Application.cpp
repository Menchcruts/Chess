#include <iostream>
#include <filesystem>
#include <format>
#include <chrono>
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

static std::string FormatMs( std::chrono::milliseconds time )
{
    using namespace std::chrono;
    
    auto h = duration_cast<hours>(time);
    time -= h;
    auto m = duration_cast<minutes>(time);
    time -= m;
    auto s = duration_cast<seconds>(time);
    time -= s;

    if ( h.count() > 0 )
        return std::format( "{}:{:0>2}:{:0>2}", h.count(), m.count(), s.count() );
    else if ( m.count() > 0 )
        return std::format( "{}:{:0>2}", m.count(), s.count() );
    else
        return std::format( "{}:{:0>3}", s.count(), time.count() );
}


void ChessApp::DrawChessboardScreen()
{
    ImGui::SetNextWindowSizeConstraints(
        ImVec2( 800, 800 ),
        ImVec2( 1250, 1250 )
    );
    // Begin a new window for the chessboard
    ImGui::Begin( "Chessboard" );

    // Get the current ImGui window's drawing list and draw the board
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    DrawChessBoard( drawList );

    // Draw the selected piece on the mouse
    DrawPieceSelected( drawList );

    ImGui::End();
}

void ChessApp::DrawChessBoard( ImDrawList* DrawList )
{
    if ( ImGui::IsWindowFocused() && ImGui::IsMouseClicked( 0 ) )
        m_SelectedSquares.clear();

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
            Square Sq = m_Board[CurrSq];
            Sq.Sq = CurrSq;

            // Calculate the cell's corners
            ImVec2 cellMin = ImVec2( p.x + col * m_CellSize, p.y + row * m_CellSize );
            ImVec2 cellMax = ImVec2( cellMin.x + m_CellSize, cellMin.y + m_CellSize );

            ImGui::TableNextColumn();

            ImGui::Dummy( ImVec2( m_CellSize, m_CellSize ) ); // Dummy widget to get hover states
            if ( ImGui::IsItemHovered() )
            {
                HandleBoardClicks( Sq );
            }

            // Draw the cell
            ImColor CellColor;
            if ( CurrSq == m_SelectedSq )
                CellColor = isDark ? m_EvenColorHighlight : m_OddColorHighlight;
            else if ( m_SelectedSquares.contains( CurrSq ) )
                CellColor = isDark ? m_EvenColorRed : m_OddColorRed;
            else
                CellColor = isDark ? m_EvenColor : m_OddColor;

            ImU32 col32 = CellColor;
            DrawList->AddRectFilled( cellMin, cellMax, col32 );

            // No need to add a piece to the screen if there is no piece
            if ( Sq.Piece.Type != Chess::PieceType::None && (!ImGui::IsMouseDown( 0 ) || CurrSq != m_SelectedSq) )
            {
                Image image = m_PieceImages[(int)Sq.Piece.Color][Sq.Piece.Type];
                DrawList->AddImage( image.Texture, cellMin, cellMax );
            }
        }
    }
    ImGui::EndTable();
    ImGui::PopStyleVar();
}

void ChessApp::HandleBoardClicks( Square Sq )
{
    const int CurrentSq = Sq.Sq;
    if ( ImGui::IsMouseClicked( 0 ) && Sq.Piece.Type != Chess::PieceType::None )
    {
        std::cout << "Mouse down on " << CurrentSq << "\n";
        if ( CurrentSq == m_SelectedSq )
            m_SelectedPressed = true;
        else
            m_SelectedSq = CurrentSq;
        m_PieceHeld = Sq.Piece;
    }
    else if ( ImGui::IsMouseReleased( 0 ) )
    {
        std::cout << "Mouse released on " << CurrentSq << "\n";

        if ( m_PieceHeld.Type != Chess::PieceType::None ) {}

        if ( m_SelectedPressed && CurrentSq == m_SelectedSq )
        {
            m_SelectedSq = -1;
            m_SelectedPressed = false;
        }
        m_PieceHeld = Chess::Pieces::Piece();
    }
    else if ( ImGui::IsMouseClicked( 1 ) )
    {
        if ( m_SelectedSquares.contains( CurrentSq ) )
            m_SelectedSquares.erase( CurrentSq );
        else
            m_SelectedSquares.insert( CurrentSq );
    }
}

void ChessApp::DrawPieceSelected( ImDrawList* DrawList ) const
{
    if ( m_PieceHeld.Type != Chess::PieceType::None && ImGui::IsMouseDown( 0 ) )
    {
        Image image = m_PieceImages[(int)m_PieceHeld.Color].at(m_PieceHeld.Type);   // Stupid en virkar
        ImVec2 MousePos = ImGui::GetMousePosOnOpeningCurrentPopup();

        ImVec2 UpLeft( MousePos.x - image.Width / 2, MousePos.y - image.Height / 2 );
        ImVec2 DownRight( MousePos.x + image.Width / 2, MousePos.y + image.Height / 2 );

        DrawList->AddImage( image.Texture, UpLeft, DownRight );
    }
}

void ChessApp::DrawDebugScreen()
{
    using namespace std::chrono;

    ImGui::Begin( "Debug" );

    ImGui::SeparatorText( "Basic Info" );
    ImGui::Text( "FPS: %f", ImGui::GetIO().Framerate );
    if ( m_SelectedSq != -1 )
        ImGui::Text( "Square selected: %d", m_SelectedSq );
    else
        ImGui::Text( "Square selected: None " );

    std::string PieceString;
    switch ( m_PieceHeld.Type )
    {
    case Chess::PieceType::None:
        PieceString = "None";
        break;
    case Chess::PieceType::King:
        PieceString = "King";
        break;
    case Chess::PieceType::Pawn:
        PieceString = "Pawn";
        break;
    case Chess::PieceType::Knight:
        PieceString = "Knight";
        break;
    case Chess::PieceType::Bishop:
        PieceString = "Bishop";
        break;
    case Chess::PieceType::Rook:
        PieceString = "Rook";
        break;
    case Chess::PieceType::Queen:
        PieceString = "Queen";
        break;
    default:
        break;
    }
    ImGui::Text( "Piece held: %s", PieceString.c_str() );

    ImGui::SeparatorText( "Chessboard Variables" );

    ImGui::ColorEdit4( "Even Color", &m_EvenColor.Value.x );
    ImGui::ColorEdit4( "Even Highlight", &m_EvenColorHighlight.Value.x );
    ImGui::ColorEdit4( "Even Color Red", &m_EvenColorRed.Value.x );

    ImGui::ColorEdit4( "Odd Color", &m_OddColor.Value.x );
    ImGui::ColorEdit4( "Odd Highlight", &m_OddColorHighlight.Value.x );
    ImGui::ColorEdit4( "Odd Color Red", &m_OddColorRed.Value.x );

    ImGui::DragFloat( "Cell size", &m_CellSize, 1.f, 50.f, 150.f);

    ImGui::SeparatorText( "Testing" );

    std::string time_string = FormatMs( m_Clock.GetTimeLeft() );
    ImGui::PushFont( m_Gabarito );
    ImGui::TextUnformatted( time_string.c_str() );
    ImGui::PopFont();
    m_Clock.Update();

    ImGui::End();
}

void ChessApp::LoadFonts()
{
    namespace fs = std::filesystem;
    ImGuiIO& io = ImGui::GetIO();
    //io.Fonts->AddFontDefault();

    fs::path GabaritoPath = fs::path( ".\\Assets\\Fonts\\Gabarito\\static\\Gabarito-Regular.ttf" );
    fs::path LexendPath = fs::path( ".\\Assets\\Fonts\\Lexend\\static\\Lexend-Regular.ttf" );
    fs::path OutfitPath= fs::path( ".\\Assets\\Fonts\\Outfit\\static\\Outfit-Regular.ttf" );

    m_Gabarito = io.Fonts->AddFontFromFileTTF( GabaritoPath.string().c_str(), 24.0f );
    m_Lexend = io.Fonts->AddFontFromFileTTF( LexendPath.string().c_str(), 24.0f );
    m_Outfit = io.Fonts->AddFontFromFileTTF( OutfitPath.string().c_str(), 24.0f );
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

void ChessApp::tempLoadBoard()
{
    m_Board[0] = Square( Chess::Color::White, Chess::PieceType::King );
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

    m_Clock.SetTimeLeft( 3620000ms );

    LoadFonts();
    LoadPieceImages();
    tempLoadBoard();
}

ChessApp::~ChessApp()
{
    // Cleanup
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

        // Create a fullscreen dock space
        SetupDockspace();

        DrawDebugScreen();

        //ImGui::ShowDemoWindow();

        // Example windows
        ImGui::Begin( "Example Window 1" );
        ImGui::Text( "This window can be docked or undocked." );
        ImGui::End();

        ImGui::Begin( "Example Window 2" );
        ImGui::Text( "Drag me around, dock me, or even pop me out!" );
        ImGui::End();

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
