#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include "glfw_platform.h"
#include "normal_mapping_fs.h"
#include "normal_mapping_vs.h"
#include "scene.h"
#include <GL/glew.h>
#include <chrono>
#include <filesystem>
#include <grapho/gl3/fbo.h>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/shadergenerator.h>
#include <grapho/imgui/dockspace.h>
#include <grapho/imgui/widgets.h>
#include <grapho/orbitview.h>
#include <grapho/shadersnippet.h>
#include <iostream>

// settings
auto SCR_WIDTH = 800;
auto SCR_HEIGHT = 600;
auto DOCK_SPACE = "DOCK_SPACE";

int
main(int argc, char** argv)
{
  if (argc < 2) {
    std::cout << "usage: " << argv[0] << " {path to LearnOpenGL dir}"
              << std::endl;
    return 1;
  }
  std::filesystem::path dir = argv[1];

  GlfwPlatform platform;
  auto window = platform.CreateWindow("LearnOpenGL", SCR_WIDTH, SCR_HEIGHT);
  if (!window) {
    return 2;
  }

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (glewInit() != GLEW_OK) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return 3;
  }

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
    ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
    ImGuiConfigFlags_NavEnableGamepad;                // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport /
                                                      // Platform Windows
  // io.ConfigViewportsNoAutoMerge = true;
  // io.ConfigViewportsNoTaskBarIcon = true;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // When viewports are enabled we tweak WindowRounding/WindowBg so platform
  // windows can look identical to regular ones.
  ImGuiStyle& style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 130");

  bool resetLayout = false;
  std::vector<grapho::imgui::Dock> docks;
  docks.push_back(
    grapho::imgui::Dock("demo", [](const char* title, bool* popen) {
      ImGui::ShowDemoWindow(popen);
    }));
  docks.back().IsOpen = false;

  //
  // scene
  //
  Scene scene;
  if (!scene.Initialize(dir)) {
    return 4;
  }
  grapho::gl3::FboHolder fbo;
  grapho::OrbitView camera;
  DirectX::XMFLOAT4 clearColor{ 0.1f, 0.1f, 0.1f, 1 };
  {
    docks.push_back(
      { "normalmap",
        [&scene, &fbo, &camera, &clearColor](const char* title, bool* p_open) {
          ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
          if (ImGui::Begin(title,
                           p_open,
                           ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoScrollWithMouse)) {

            // get and update fbo size
            auto size = ImGui::GetContentRegionAvail();
            auto texture = fbo.Bind(size.x, size.y, clearColor);
            auto [isActive, isHovered] = grapho::imgui::DraggableImage(
              (ImTextureID)(uint64_t)texture, size);

            // update camera from mouse
            ImGuiIO& io = ImGui::GetIO();
            camera.SetSize(io.DisplaySize.x, io.DisplaySize.y);
            if (isActive) {
              if (io.MouseDown[ImGuiMouseButton_Right]) {
                camera.YawPitch(io.MouseDelta.x, io.MouseDelta.y);
              }
              if (io.MouseDown[ImGuiMouseButton_Middle]) {
                camera.Shift(io.MouseDelta.x, io.MouseDelta.y);
              }
            }
            if (isHovered) {
              camera.Dolly(io.MouseWheel);
            }

            // render to fbo
            DirectX::XMFLOAT4X4 projection;
            DirectX::XMFLOAT4X4 view;
            camera.Update(&projection._11, &view._11);
            scene.Render(io.DeltaTime, projection, view, camera.Position);

            fbo.Unbind();
          }
          ImGui::End();
          ImGui::PopStyleVar();
        } });
  }

  // render loop
  // -----------
  while (platform.BeginFrame()) {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // update imgui
    grapho::imgui::DockSpace("dock_space", docks, &resetLayout);

    // render
    glViewport(0,
               0,
               static_cast<int>(io.DisplaySize.x),
               static_cast<int>(io.DisplaySize.y));
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    platform.EndFrame([]() {
      // Update and Render additional Platform Windows
      // (Platform functions may change the current OpenGL context, so we
      // save/restore it to make it easier to paste this code elsewhere.
      //  For this specific demo app we could also call
      //  glfwMakeContextCurrent(window) directly)
      if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
      }
    });
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  return 0;
}
