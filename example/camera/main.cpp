#include <DirectXMath.h>

#include <GL/glew.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <filesystem>
#include <grapho/camera/camera.h>
#include <grapho/gl3/error_check.h>
#include <grapho/gl3/fbo.h>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/vao.h>
#include <grapho/imgui/dockspace.h>
#include <grapho/imgui/widgets.h>
#include <grapho/mesh.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
#include <iostream>

#include "glfw_platform.h"

// settings
auto SCR_WIDTH = 800;
auto SCR_HEIGHT = 600;
auto DOCK_SPACE = "DOCK_SPACE";

static const auto VS = u8R"(#version 400
uniform mat4 model; 
uniform mat4 view; 
uniform mat4 projection; 
in vec3 vPos;
void main()
{
    gl_Position = projection * view * model * vec4(vPos, 1.0);
};
)";

static const auto FS = u8R"(#version 400
out vec4 FragColor;
void main()
{
    FragColor = vec4(1,1,1,1);
};
)";

class Gui
{
  std::vector<grapho::imgui::Dock> m_docks;

  grapho::gl3::FboHolder m_fbo;
  grapho::camera::Camera m_camera;
  DirectX::XMFLOAT4 m_clearColor{ 0.1f, 0.1f, 0.1f, 1 };

  std::shared_ptr<grapho::gl3::ShaderProgram> m_shader;
  struct Drawable
  {
    std::shared_ptr<grapho::gl3::Vao> Vao;
    DirectX::XMFLOAT4X4 Matrix;
  };
  std::vector<std::shared_ptr<Drawable>> m_drawables;

public:
  Gui(GLFWwindow* window)
  {
    m_camera.Transform.Translation = { 0, 5, 20 };

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad;              // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport
                                                        // / Platform Windows
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
  }

  ~Gui()
  {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }

  void ShowCamera()
  {
    ImGui::InputFloat3("T", &m_camera.Transform.Translation.x);
    ImGui::InputFloat4("R", &m_camera.Transform.Rotation.x);
    ImGui::InputFloat("Gaze", &m_camera.GazeDistance);
    ImGui::InputFloat("Yaw", &m_camera.TmpYaw);
    ImGui::InputFloat("Pitch", &m_camera.TmpPitch);
  }

  bool InitializeScene()
  {
    m_docks.push_back({ "view", [=]() { ShowGui(); } });
    m_docks.push_back({ "camera", [=]() { ShowCamera(); } });

    if (auto shader = grapho::gl3::ShaderProgram::Create(VS, FS)) {
      m_shader = *shader;
    } else {
      std::cerr << shader.error() << std::endl;
      return false;
    }

    auto cube = grapho::mesh::Cube(0.5f);

    auto d = 5.0f;
    {
      // 0
      auto drawable = std::make_shared<Drawable>();
      drawable->Vao = grapho::gl3::Vao::Create(cube);
      DirectX::XMStoreFloat4x4(&drawable->Matrix,
                               DirectX::XMMatrixTranslation(-d, 0, -d));
      m_drawables.push_back(drawable);
    }
    {
      // 1
      auto drawable = std::make_shared<Drawable>();
      drawable->Vao = grapho::gl3::Vao::Create(cube);
      DirectX::XMStoreFloat4x4(&drawable->Matrix,
                               DirectX::XMMatrixTranslation(d, 0, -d));
      m_drawables.push_back(drawable);
    }
    {
      // 2
      auto drawable = std::make_shared<Drawable>();
      drawable->Vao = grapho::gl3::Vao::Create(cube);
      DirectX::XMStoreFloat4x4(&drawable->Matrix,
                               DirectX::XMMatrixTranslation(d, 0, d));
      m_drawables.push_back(drawable);
    }
    {
      // 3
      auto drawable = std::make_shared<Drawable>();
      drawable->Vao = grapho::gl3::Vao::Create(cube);
      DirectX::XMStoreFloat4x4(&drawable->Matrix,
                               DirectX::XMMatrixTranslation(-d, 0, d));
      m_drawables.push_back(drawable);
    }

    return true;
  }

  void ShowGui()
  {
    assert(!grapho::gl3::TryGetError());

    // get and update fbo size
    auto size = ImGui::GetContentRegionAvail();
    auto texture = m_fbo.Bind(
      static_cast<int>(size.x), static_cast<int>(size.y), m_clearColor);
    auto [isActive, isHovered] =
      grapho::imgui::DraggableImage((ImTextureID)(uint64_t)texture, size);

    // update camera from mouse
    ImGuiIO& io = ImGui::GetIO();
    m_camera.Projection.SetSize(size.x, size.y);
    if (isActive) {
      if (io.MouseDown[ImGuiMouseButton_Right]) {
        m_camera.YawPitch(static_cast<int>(io.MouseDelta.x),
                          static_cast<int>(io.MouseDelta.y));
      }
      if (io.MouseDown[ImGuiMouseButton_Middle]) {
        m_camera.Shift(static_cast<int>(io.MouseDelta.x),
                       static_cast<int>(io.MouseDelta.y));
      }
    }
    if (isHovered) {
      m_camera.Dolly(static_cast<int>(io.MouseWheel));
    }

    // render to fbo
    m_camera.Update();
    m_shader->Use();
    m_shader->Uniform("view")->Set(m_camera.ViewMatrix);
    assert(!grapho::gl3::TryGetError());
    m_shader->Uniform("projection")->Set(m_camera.ProjectionMatrix);
    assert(!grapho::gl3::TryGetError());
    for (auto& drawable : m_drawables) {
      m_shader->Uniform("model")->Set(drawable->Matrix);
      assert(!grapho::gl3::TryGetError());
      drawable->Vao->Draw(GL_TRIANGLES, 36);
      assert(!grapho::gl3::TryGetError());
    }

    m_fbo.Unbind();
  }

  void Begin()
  {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    assert(!grapho::gl3::TryGetError());

    // update imgui
    grapho::imgui::BeginDockSpace("dock_space");
    ImGui::End();
    for (auto& d : m_docks) {
      d.Show();
    }
  }

  void End()
  {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }
};

int
main(int argc, char** argv)
{
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
  assert(!grapho::gl3::TryGetError());

  Gui gui(window);
  if (!gui.InitializeScene()) {
    return 4;
  }
  assert(!grapho::gl3::TryGetError());

  // render loop
  // -----------
  while (platform.BeginFrame()) {
    assert(!grapho::gl3::TryGetError());

    gui.Begin();
    {
      assert(!grapho::gl3::TryGetError());
      ImGuiIO& io = ImGui::GetIO();

      // render
      glViewport(0,
                 0,
                 static_cast<int>(io.DisplaySize.x),
                 static_cast<int>(io.DisplaySize.y));
      glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      assert(!grapho::gl3::TryGetError());
    }
    gui.End();
    assert(!grapho::gl3::TryGetError());

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

  return 0;
}
