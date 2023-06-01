#include <GL/glew.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <functional>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include "drawable.h"
#include "glfw_platform.h"
#include "imageloader.h"
#include <grapho/gl3/pbr.h>
#include <grapho/gl3/shader_type_name.h>
#include <grapho/imgui/dockspace.h>
#include <grapho/imgui/widgets.h>
#include <grapho/orbitview.h>
#include <iostream>
#include <vector>

// settings
const auto SCR_WIDTH = 1600;
const auto SCR_HEIGHT = 1200;

class Gui
{
  std::vector<grapho::imgui::Dock> docks;
  bool resetLayout = false;

public:
  Gui(GLFWwindow* window)
  {
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

    docks.push_back(
      grapho::imgui::Dock("demo", [](const char* title, bool* popen) {
        ImGui::ShowDemoWindow(popen);
      }));
    docks.back().IsOpen = false;
  }

  ~Gui()
  {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }

  grapho::gl3::FboHolder m_fbo;
  grapho::OrbitView m_camera;
  DirectX::XMFLOAT4 m_clearColor{ 0.1f, 0.1f, 0.1f, 1 };
  Scene m_scene;
  std::shared_ptr<grapho::gl3::PbrEnv> m_pbrEnv;
  std::shared_ptr<grapho::gl3::Ubo> m_worldUbo;
  grapho::gl3::Material::WorldVars m_world;

  bool InitializeScene(const std::filesystem::path& dir)
  {
    m_camera.shift_[2] = -10;

    //
    // PbrEnv
    //
    ImageLoader hdr;
    if (!hdr.LoadHdr(dir / ("resources/textures/hdr/newport_loft.hdr"))) {
      std::cout << "Failed to load HDR image." << std::endl;
      return false;
    }

    auto hdrTexture = grapho::gl3::Texture::Create(hdr.Image, true);
    if (!hdrTexture) {
      return false;
    }
    m_pbrEnv = std::make_shared<grapho::gl3::PbrEnv>(hdrTexture);

    m_world = 
    {
      .lightPositions = {
        { -10.0f, 10.0f, 10.0f, 0 },
        { 10.0f, 10.0f, 10.0f, 0 },
        { -10.0f, -10.0f, 10.0f, 0 },
        { 10.0f, -10.0f, 10.0f, 0 },
      },
      .lightColors = {
        { 300.0f, 300.0f, 300.0f, 0 },
        { 300.0f, 300.0f, 300.0f, 0 },
        { 300.0f, 300.0f, 300.0f, 0 },
        { 300.0f, 300.0f, 300.0f, 0 },
      }
    };
    m_worldUbo = grapho::gl3::Ubo::Create(sizeof(m_world), nullptr);

    //
    // PbrMaterial objects
    //
    m_scene.Load(dir);

    // if (!m_scene.Initialize(dir)) {
    //   return false;
    // }
    docks.push_back({ "pbr", std::bind(&Gui::ShowGui, this), true });

    docks.push_back(
      { "objects", std::bind(&Gui::ShowObjectList, this), false });

    docks.push_back(
      { "uniforms", std::bind(&Gui::ShowUniformList, this), false });

    return true;
  }

  void ShowGui()
  {
    // get and update fbo size
    auto size = ImGui::GetContentRegionAvail();
    auto texture = m_fbo.Bind(
      static_cast<int>(size.x), static_cast<int>(size.y), m_clearColor);
    auto [isActive, isHovered] =
      grapho::imgui::DraggableImage((ImTextureID)(uint64_t)texture, size);

    // update camera from mouse
    ImGuiIO& io = ImGui::GetIO();
    m_camera.SetSize(static_cast<int>(size.x), static_cast<int>(size.y));
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
    m_camera.Update(&m_world.projection._11, &m_world.view._11);
    m_world.camPos = {
      m_camera.Position.x, m_camera.Position.y, m_camera.Position.z, 1
    };

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    // set depth function to less than AND equal for skybox depth trick.
    glDepthFunc(GL_LEQUAL);
    // enable seamless cubemap sampling for lower mip levels in the pre-filter
    // map.
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    m_worldUbo->Upload(m_world);
    m_worldUbo->Bind();
    m_worldUbo->SetBindingPoint(0);

    // glViewport(0,
    //            0,
    //            static_cast<int>(io.DisplaySize.x),
    //            static_cast<int>(io.DisplaySize.y));
    // glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ENV
    m_pbrEnv->Activate();
    // OBJECTS
    for (auto& drawable : m_scene.Drawables) {
      drawable->Draw(0);
    }
    m_pbrEnv->DrawSkybox(m_world.projection, m_world.view);

    m_fbo.Unbind();
  }

  int m_selected = 0;

  void ShowObjectList()
  {
    int i = 0;
    for (auto d : m_scene.Drawables) {
      char buf[64];
      snprintf(buf, sizeof(buf), "%d", i);
      if (ImGui::Selectable(buf, i == m_selected)) {
        m_selected = i;
      }
      ++i;
    }
  }

  void ShowUniformList()
  {
    ImGui::Text("%d", m_selected);
    if (m_selected < 0 || m_selected >= m_scene.Drawables.size()) {
      return;
    }
    auto d = m_scene.Drawables[m_selected];

    for (auto& var : d->Material->Shader->Uniforms) {
      ImGui::Text("#%d: %s %s",
                  var.Location,
                  grapho::gl3::ShaderTypeName(var.Type),
                  var.Name.c_str());
    }
  }

  void Update()
  {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // update imgui
    grapho::imgui::DockSpace("dock_space", docks, &resetLayout);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }
};

int
main(int argc, char** argv)
{
  if (argc < 2) {
    std::cout << "usage: " << argv[0] << " {path to LearnOpenGL dir}"
              << std::endl;
    return 1;
  }
  std::filesystem::path dir(argv[1]);

  GlfwPlatform platform;
  auto window = platform.CreateWindow("LearnOpenGL", SCR_WIDTH, SCR_HEIGHT);
  if (!window) {
    return 2;
  }

  if (glewInit() != GLEW_OK) {
    std::cout << "Failed to initialize GLEW" << std::endl;
    return 3;
  }

  Gui gui(window);
  if (!gui.InitializeScene(dir)) {
    return 4;
  }

  // render loop
  // -----------
  while (platform.BeginFrame()) {
    ImGuiIO& io = ImGui::GetIO();

    // render
    glViewport(0,
               0,
               static_cast<int>(io.DisplaySize.x),
               static_cast<int>(io.DisplaySize.y));
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gui.Update();

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
