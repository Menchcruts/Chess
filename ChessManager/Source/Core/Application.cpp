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
    case Chess::CastlingRights::BOTH:
        return "Both";
    case Chess::CastlingRights::None:
    default:
        return "None";
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
    DrawChessBoard();

    // Draw the selected piece on the mouse
    DrawPieceSelected( drawList );

    ImGui::End();
}

void ChessApp::DrawChessBoard()
{
    if ( ImGui::IsWindowFocused() && ImGui::IsMouseClicked( 0 ) )
        m_BoardSettings.HighlightedSquares.clear();

    const int BoardSize = 8;

    const Chess::Board& Board = m_Engine.GetBoard();
    
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

            ImColor CellColor;
            if ( CurrSq == m_BoardSettings.SelectedSquare )
                CellColor = isDark ? m_Colors.EvenColorHighlight : m_Colors.OddColorHighlight;
            else if ( m_BoardSettings.HighlightedSquares.contains( CurrSq ) )
                CellColor = isDark ? m_Colors.EvenColorRed : m_Colors.OddColorRed;
            else
                CellColor = isDark ? m_Colors.EvenColor : m_Colors.OddColor;

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
                ImGui::Image( image.Texture, ImVec2( (float)image.Width, (float)image.Height ) );   // Annars teiknum við myndina
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

void ChessApp::HandleBoardClicks( Chess::Piece Piece, int CurrentSq )
{
    if ( ImGui::IsMouseClicked( 0 ) )
    {
        std::cout << "Mouse clicked on " << CurrentSq << "\n";
        if ( m_BoardSettings.SelectedSquare != -1 && CurrentSq != m_BoardSettings.SelectedSquare )
        {
            if ( true ) // Future check for valid move
            {
                MakeMove( m_BoardSettings.SelectedSquare, CurrentSq );
                m_BoardSettings.SelectedSquare = -1;
            }
            return;
        }
        
        if ( !Piece.IsNullPiece() )
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
            m_BoardSettings.PieceHeld = Chess::Piece();
        }
    }
    else if ( ImGui::IsMouseReleased( 0 ) )
    {
        std::cout << "Mouse released on " << CurrentSq << "\n";

        if ( !m_BoardSettings.PieceHeld.IsNullPiece() && CurrentSq != m_BoardSettings.SelectedSquare )
            if ( true ) // Future check for valid move
            {
                MakeMove( m_BoardSettings.SelectedSquare, CurrentSq );
                m_BoardSettings.SelectedSquare = -1;
                m_BoardSettings.SelectedPressed = false;
            }

        if ( m_BoardSettings.SelectedPressed && CurrentSq == m_BoardSettings.SelectedSquare )
        {
            m_BoardSettings.SelectedSquare = -1;
            m_BoardSettings.SelectedPressed = false;
        }
        m_BoardSettings.PieceHeld = Chess::Piece();
    }
    else if ( ImGui::IsMouseClicked( 1 ) )
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

        ImVec2 UpLeft( MousePos.x - image.Width / 2, MousePos.y - image.Height / 2 );
        ImVec2 DownRight( MousePos.x + image.Width / 2, MousePos.y + image.Height / 2 );

        DrawList->AddImage( image.Texture, UpLeft, DownRight );
    }
}

void ChessApp::MakeMove( int Start, int Target )
{
    std::cout << "Move( " << Start << ", " << Target << " )\n";
    m_Engine.MakeMove( Start, Target );
    m_BoardSettings.EngineInfo = m_Engine.GetEngineInfo();  // Update info
    m_SoundEngine->play2D( "Assets/Sounds/move-self.mp3" );
}

void ChessApp::DrawDebugScreen()
{
    using namespace std::chrono;

    ImGui::Begin( "Debug" );

    ImGui::SeparatorText( "Basic Info" );
    ImGui::Text( "FPS: %f", ImGui::GetIO().Framerate );
    if ( m_BoardSettings.SelectedSquare != -1 )
        ImGui::Text( "Square selected: %d", m_BoardSettings.SelectedSquare );
    else
        ImGui::Text( "Square selected: None " );

    std::string PieceString;
    switch ( m_BoardSettings.PieceHeld.type )
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

    ImGui::SeparatorText( "Engine" );

    ImGui::Text( "%s to play", m_BoardSettings.EngineInfo.WhiteToPlay ? "White" : "Black" );
    ImGui::Text( "En Passant square: %s", GetSquareName(m_BoardSettings.EngineInfo.EnPassantSquare).c_str() );
    ImGui::Text( "Fullmove clock: %d", m_BoardSettings.EngineInfo.FullmoveClock );
    ImGui::Text( "Halfmove clock: %d", m_BoardSettings.EngineInfo.HalfmoveClock );
    ImGui::Text( "White castling rights: %s", GetCastlingString( m_BoardSettings.EngineInfo.WhiteCastling ).c_str() );
    ImGui::Text( "Black castling rights: %s", GetCastlingString( m_BoardSettings.EngineInfo.BlackCastling ).c_str() );

    if ( ImGui::InputText( "Load new FEN position", m_BoardSettings.NewFen, IM_ARRAYSIZE( m_BoardSettings.NewFen ), ImGuiInputTextFlags_EnterReturnsTrue ) )
    {
        std::cout << "Loading FEN: " << m_BoardSettings.NewFen << "\n";
        m_Engine.LoadFEN( std::string( m_BoardSettings.NewFen ) );
        m_BoardSettings.EngineInfo = m_Engine.GetEngineInfo();
        memset( m_BoardSettings.NewFen, 0, sizeof( m_BoardSettings.NewFen ) );
    }

    ImGui::SeparatorText( "Chessboard Variables" );

    ImGui::ColorEdit4( "Even Color", &m_Colors.EvenColor.Value.x );
    ImGui::ColorEdit4( "Even Highlight", &m_Colors.EvenColorHighlight.Value.x );
    ImGui::ColorEdit4( "Even Color Red", &m_Colors.EvenColorRed.Value.x );

    ImGui::ColorEdit4( "Odd Color", &m_Colors.OddColor.Value.x );
    ImGui::ColorEdit4( "Odd Highlight", &m_Colors.OddColorHighlight.Value.x );
    ImGui::ColorEdit4( "Odd Color Red", &m_Colors.OddColorRed.Value.x );

    ImGui::DragFloat( "Cell size", &m_BoardSettings.CellSize, 1.f, 50.f, 150.f);

    ImGui::SeparatorText( "Testing" );

    std::string time_string = FormatMs( m_Clock.GetTimeLeft() );
    ImGui::PushFont( m_Fonts.Gabarito );
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

    m_Fonts.Gabarito = io.Fonts->AddFontFromFileTTF( GabaritoPath.string().c_str(), 24.0f );
    m_Fonts.Lexend = io.Fonts->AddFontFromFileTTF( LexendPath.string().c_str(), 24.0f );
    m_Fonts.Outfit = io.Fonts->AddFontFromFileTTF( OutfitPath.string().c_str(), 24.0f );
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

    m_Clock.SetTimeLeft( 3620000ms );

    LoadFonts();
    LoadPieceImages();

    m_Engine.LoadFEN( "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" );
    m_BoardSettings.EngineInfo = m_Engine.GetEngineInfo();
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
