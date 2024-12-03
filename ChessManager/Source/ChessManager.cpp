#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>


int main()
{
    // Initialize GLFW
    if ( !glfwInit() )
        return -1;

    GLFWwindow* window = glfwCreateWindow( 800, 600, "Chess Manager", NULL, NULL );
    if ( !window )
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent( window );

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL( window, true );
    ImGui_ImplOpenGL3_Init( "#version 130" );

    // Main loop
    while ( !glfwWindowShouldClose( window ) )
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui UI code
        ImGui::Begin( "Hello, ImGui!" );
        ImGui::Text( "This is a Dear ImGui example." );
        ImGui::End();

        ImGui::Render();
        glClear( GL_COLOR_BUFFER_BIT );
        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

        glfwSwapBuffers( window );
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow( window );
    glfwTerminate();
}
