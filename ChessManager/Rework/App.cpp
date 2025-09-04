#include "App.h"
#include "asset_manager.hpp"
#include "Windows/ChessWindow.h"
#include "Windows/PerftWindow.h"
#include "../Assets/Fonts/Icons/IconsFontAwesome5Pro.h"
//#include "../Assets/Fonts/Icons/IconsForkAwesome.h"

#include "Chess/Bitboards.h"
#include <iostream>

App::App() :
    m_Logger("AppRework.log")
{    
    if (!glfwInit())
    {
        m_Logger.error("Failed to initialize GLFW.");
        throw std::exception("Failed to initialize GLFW.");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_Window = glfwCreateWindow(1920, 1080, "App Rework", NULL, NULL);
    if (m_Window == nullptr)
    {
        m_Logger.error("Failed to create GLFW window.");
        throw std::exception("Failed to create GLFW window.");
    }
    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK)
    {
        m_Logger.error("Failed to initalize GLEW.");
        glfwTerminate();
        throw std::exception();
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    m_Logger.info("OpenGL version: {}", (const char*)glGetString(GL_VERSION));

    m_Images = std::make_unique<assets::ImageManager>();
    LoadImages();
    LoadFonts();
    
    Chess::Bitboards::init();
    m_MoveHistory.reserve(128);

    m_Board = std::make_unique<Chess::Chessboard>();
    m_Board->ResetBoard();
    std::cout << m_Board->ExportFEN() << "\n";

    LoadWindows();
}

App::~App()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void App::SetupDockspace()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | 
                                    ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_NoCollapse | 
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove | 
                                    ImGuiWindowFlags_NoBringToFrontOnFocus |
                                    ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("MainDockSpaceWindow", NULL, window_flags);
    ImGui::PopStyleVar(2);

    // Set the docking space
    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();
}

void App::LoadWindows()
{
    auto MakeMove = [this](Chess::Move move)
        {
            this->m_Board->MakeMove(move);
            this->m_MoveHistory.emplace_back(move);
        };
    auto UnMakeMove = [this]()
        {
            if (this->m_MoveHistory.empty())
                return;

            Chess::Move last_move = this->m_MoveHistory.back();
            this->m_MoveHistory.pop_back();
            this->m_Board->UnMakeMove(last_move);
        };
    
    auto LoadFEN = [this](std::string_view FEN)
        {
            this->m_Board->LoadFEN(FEN);
        };

    (void)AddWindow<ChessWindow>(
        "Chessboard", 
        *m_Board,
        MakeMove,
        UnMakeMove,
        LoadFEN
    );

    (void)AddWindow<PerftWindow>(
        "Perft Tests",
        *m_Board
    );
}

void App::LoadImages()
{
    m_Images->addAlias("Pieces", "Assets/Pieces");
    m_Images->preloadDir("Assets/Pieces", { ".png" });
}

void App::LoadFonts()
{
    ImGuiIO& io = ImGui::GetIO();
    ImFontAtlas* atlas = io.Fonts;
    
    const std::string assets_fonts_text = "Assets/Fonts/Text/";

    atlas->AddFontFromFileTTF((assets_fonts_text + "Roboto-Medium.ttf").c_str(), 17.f);

    ImFontConfig config;
    config.MergeMode = true;
    config.GlyphMinAdvanceX = 20.f;
    atlas->AddFontFromFileTTF("Assets/Fonts/Icons/FontAwesome5_Pro_900.ttf", 13.f, &config);

    atlas->AddFontDefault();
}

void App::PreFrame()
{
    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    SetupDockspace();
}

void App::PostFrame()
{
    ImGuiIO& io = ImGui::GetIO();

    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(m_Window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }

    glfwSwapBuffers(m_Window);
}

void App::Run()
{
    while (!glfwWindowShouldClose(m_Window))
    {
        PreFrame();

        Draw();

        PostFrame();
    }
}

void App::Draw()
{
    for (auto& window : m_Windows)
    {
        window->Draw();
    }

    if (ImGui::Begin("Image Manager"))
        assets::DrawImageManagerPanel(*m_Images);
    ImGui::End();

    //ImGui::ShowStyleEditor();

    ImGui::ShowDemoWindow();
}