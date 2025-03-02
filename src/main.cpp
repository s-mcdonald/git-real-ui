#include <stdio.h>
#include <ctime> // Include the ctime library
#include <cstdlib> // Include the cstdlib library for the system function
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <iostream>
#include <git2.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <poll.h>
#include <iterator> // Include the iterator library for streambuf_iterator

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define GL_SILENCE_DEPRECATION

#include <GLFW/glfw3.h> // Will drag system OpenGL headers



// move to gloabls
// @todo: refactor away inotify hooks
int setup_inotify(const std::string& path) {
    int inotify_fd = inotify_init();
    if (inotify_fd < 0) {
        std::cerr << "Failed to initialize inotify: " << strerror(errno) << std::endl;
        return -1;
    }

    int watch_fd = inotify_add_watch(inotify_fd, path.c_str(), IN_MODIFY | IN_CREATE | IN_DELETE);
    if (watch_fd < 0) {
        std::cerr << "Failed to add inotify watch: " << strerror(errno) << std::endl;
        close(inotify_fd);
        return -1;
    }

    return inotify_fd;
}

bool check_inotify_events(int inotify_fd) {
    struct pollfd pfd = { inotify_fd, POLLIN, 0 };
    int poll_result = poll(&pfd, 1, 0); // Non-blocking poll

    if (poll_result > 0) {
        char buffer[1024];
        ssize_t length = read(inotify_fd, buffer, sizeof(buffer));
        if (length < 0) {
            std::cerr << "Failed to read inotify events: " << strerror(errno) << std::endl;
            return false;
        }

        return true;
    }

    return false;
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Git // Real", nullptr, nullptr);
    //GLFWwindow* window = glfwCreateWindow(1280, 720, Gr::App::GIT_REAL_APP_NAME, nullptr, nullptr);
    if (window == nullptr)
        return 1;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ///
    /// StateControl variables below
    ///

    // Load from the config for initial alpha dev
    // then we can create a File->open dialog later
    std::string config_file = "./config.ini";

    std::string current_branch;
    std::vector<std::string> branches;
    std::vector<std::string> remotes;


    // Timer for periodic updates
    auto last_update_time = std::chrono::steady_clock::now();


    // exit parm
    bool should_exit = false;

    //
    //
    // Main loop
    //
    //
    while (!glfwWindowShouldClose(window) && false == should_exit)
    {
        // Set tab rounding to 0 to make tabs squared
        ImGui::PushStyleVar(ImGuiStyleVar_TabRounding, 0.0f);

        // Set the background color
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.012f, 0.173f, 0.259f, 0.90f));

        // ImGui Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

   
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

            // Create a docking space
            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
            ImGui::DockSpace(ImGui::GetID("MyDockspace"));

            // Create a menu bar
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("GitReal")) {
                    if (ImGui::MenuItem("Exit")) {
                        should_exit = true;
                    }
                    
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("View")) {
                    if (ImGui::MenuItem("Reflog")) 
                    {
                        //
                    }
                    if (ImGui::BeginMenu("Changes")) {
                        ImGui::EndMenu();
                    }     
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Repository")) {
                    if (ImGui::MenuItem("Open path in terminal")) {
                        //
                    }
                    if (ImGui::BeginMenu("Actions")) {
                        ImGui::MenuItem("Open");     
                        ImGui::MenuItem("Initialize");      
                        ImGui::EndMenu();
                    }     
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Remotes")) {
                    if (ImGui::BeginMenu("Actions")) {
                        ImGui::MenuItem("Add / Remove");      
                        ImGui::EndMenu();
                    }        
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Branch")) {
                    
                    if (ImGui::BeginMenu("Actions")) {
                        ImGui::MenuItem("Fetch");
                        ImGui::MenuItem("Pull");
                        ImGui::MenuItem("Fetch + Pull");
                        ImGui::EndMenu();
                    }

                    ImGui::EndMenu();
                }
            }
            ImGui::EndMainMenuBar();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
