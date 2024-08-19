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
#include <grapho/camera/camera.h>
#include <grapho/gl3/error_check.h>
#include <grapho/gl3/glsl_type_name.h>
#include <grapho/gl3/pbr.h>
#include <grapho/gl3/ubo.h>
#include <grapho/imgui/dockspace.h>
#include <grapho/imgui/widgets.h>
#include <iostream>
#include <vector>

// settings
const auto SCR_WIDTH = 1600;
const auto SCR_HEIGHT = 1200;

class Gui
{
  std::vector<grapho::imgui::Dock> docks;

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

    // docks.push_back(
    //   grapho::imgui::Dock("demo", [](const char* title, bool* popen) {
    //     ImGui::ShowDemoWindow(popen);
    //   }));
    // docks.back().IsOpen = false;
  }

  ~Gui()
  {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }

  grapho::gl3::FboHolder m_fbo;
  grapho::camera::Camera m_camera;
  grapho::XMFLOAT4 m_clearColor{ 0.1f, 0.1f, 0.1f, 1 };
  std::vector<std::shared_ptr<Drawable>> m_drawables;
  std::shared_ptr<grapho::gl3::PbrEnv> m_pbrEnv;
  std::shared_ptr<grapho::gl3::Ubo> m_worldUbo;
  grapho::WorldVars m_world;

  bool InitializeScene(const std::filesystem::path& dir)
  {
    m_camera.Translation.z = 10;

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
    grapho::gl3::CheckAndPrintError(
      [](const char* msg) { std::cerr << "PbrEnv: " << msg << std::endl; });

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

    if (auto drawable = Drawable::Load(
          dir / "resources/textures/pbr/rusted_iron", { -4.0, 0.0, 2.0 })) {
      m_drawables.push_back(drawable);
    }
    if (auto drawable = Drawable::Load(dir / "resources/textures/pbr/gold",
                                       { -2.0, 0.0, 2.0 })) {
      m_drawables.push_back(drawable);
    }
    if (auto drawable = Drawable::Load(dir / "resources/textures/pbr/grass",
                                       { -0.0, 0.0, 2.0 })) {
      m_drawables.push_back(drawable);
    }
    if (auto drawable = Drawable::Load(dir / "resources/textures/pbr/plastic",
                                       { 2.0, 0.0, 2.0 })) {
      m_drawables.push_back(drawable);
    }
    if (auto drawable = Drawable::Load(dir / "resources/textures/pbr/wall",
                                       { 4.0, 0.0, 2.0 })) {
      m_drawables.push_back(drawable);
    }

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

    // ENV
    m_pbrEnv->Activate();
    // OBJECTS
    for (auto& drawable : m_drawables) {
      drawable->Draw(0);
    }
    m_pbrEnv->DrawSkybox(m_camera.ProjectionMatrix, m_camera.ViewMatrix);

    m_fbo.Unbind();
  }

  int m_selected = 0;

  void ShowObjectList()
  {
    int i = 0;
    for (auto d : m_drawables) {
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
    if (m_selected < 0 || m_selected >= m_drawables.size()) {
      return;
    }
    auto d = m_drawables[m_selected];

    std::array<const char*, 4> cols = {
      "index",
      "location",
      "type",
      "name",
    };
    if (grapho::imgui::BeginTableColumns("uniforms", cols)) {
      for (uint32_t i = 0; i < d->Shader->Uniforms.size(); ++i) {
        auto& u = d->Shader->Uniforms[i];
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("%d", i);
        ImGui::TableNextColumn();
        ImGui::Text("%d", u.Location);
        ImGui::TableNextColumn();
        ImGui::Text("%s", grapho::gl3::ShaderTypeName(u.Type));
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(u.Name.c_str());
      }
      ImGui::EndTable();
    }
  }

  void Begin()
  {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // update imgui
    grapho::imgui::BeginDockSpace("dock_space");
    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("docks")) {
        for (auto& d : docks) {
          ImGui::MenuItem(d.Name.c_str(), "", &d.IsOpen);
        }
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }
    ImGui::End();
    for (auto& d : docks) {
      d.Show();
    }
  }

  void End()
  {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }
};

static void
print(const char* msg)
{
  std::cerr << msg << std::endl;
}

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
  grapho::gl3::CheckAndPrintError(&print);

  Gui gui(window);
  if (!gui.InitializeScene(dir)) {
    return 4;
  }
  grapho::gl3::CheckAndPrintError(&print);

  // render loop
  // -----------
  while (platform.BeginFrame()) {
    grapho::gl3::CheckAndPrintError(&print);

    gui.Begin();

    ImGuiIO& io = ImGui::GetIO();
    glViewport(0,
               0,
               static_cast<int>(io.DisplaySize.x),
               static_cast<int>(io.DisplaySize.y));
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    grapho::gl3::CheckAndPrintError(&print);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    grapho::gl3::CheckAndPrintError(&print);

    gui.End();

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
