#include "normal_mapping_fs.h"
#include "normal_mapping_vs.h"
#include "normalmap.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/shadergenerator.h>
#include <grapho/imgui/dockspace.h>
#include <grapho/orbitview.h>
#include <grapho/shadersnippet.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
#include <iostream>

const std::optional<bool> NOP = std::nullopt;

// settings
auto SCR_WIDTH = 800;
auto SCR_HEIGHT = 600;

bool g_mouseRight = false;
bool g_mouseMiddle = false;
static void
mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  switch (button) {
    case GLFW_MOUSE_BUTTON_RIGHT:
      if (action == GLFW_PRESS) {
        g_mouseRight = true;
      } else if (action == GLFW_RELEASE) {
        g_mouseRight = false;
      }
      break;
    case GLFW_MOUSE_BUTTON_MIDDLE:
      if (action == GLFW_PRESS) {
        g_mouseMiddle = true;
      } else if (action == GLFW_RELEASE) {
        g_mouseMiddle = false;
      }
      break;
  }
}

grapho::OrbitView g_camera;
void
mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
  static int lastX;
  static int lastY;
  static bool firstMouse = true;

  auto xpos = static_cast<int>(xposIn);
  auto ypos = static_cast<int>(yposIn);
  if (firstMouse) {
    firstMouse = false;
  } else {
    auto xoffset = xpos - lastX;
    auto yoffset = ypos - lastY;
    if (g_mouseRight) {
      g_camera.YawPitch(xoffset, yoffset);
    }
    if (g_mouseMiddle) {
      g_camera.Shift(xoffset, yoffset);
    }
  }
  lastX = xpos;
  lastY = ypos;
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void
scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  g_camera.Dolly(static_cast<int>(yoffset));
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void
processInput(GLFWwindow* window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  // if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
  //   camera.ProcessKeyboard(FORWARD, deltaTime);
  // if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
  //   camera.ProcessKeyboard(BACKWARD, deltaTime);
  // if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
  //   camera.ProcessKeyboard(LEFT, deltaTime);
  // if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
  //   camera.ProcessKeyboard(RIGHT, deltaTime);
}

auto DOCK_SPACE = "DOCK_SPACE";

int
main(int argc, char** argv)
{
  if (argc < 2) {
    return 1;
  }
  std::filesystem::path dir = argv[1];

  // glfw: initialize and configure
  // ------------------------------
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // glfw window creation
  // --------------------
  GLFWwindow* window =
    glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return 2;
  }
  glfwMakeContextCurrent(window);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (glewInit() != GLEW_OK) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return 3;
  }

  glfwSwapInterval(1); // Enable vsync

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

  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can
  // also load multiple fonts and use ImGui::PushFont()/PopFont() to select
  // them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you
  // need to select the font among multiple.
  // - If the file cannot be loaded, the function will return a nullptr. Please
  // handle those errors in your application (e.g. use an assertion, or display
  // an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored
  // into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which
  // ImGui_ImplXXXX_NewFrame below will call.
  // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype
  // for higher quality font rendering.
  // - Read 'docs/FONTS.md' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string
  // literal you need to write a double backslash \\ !
  // - Our Emscripten build process allows embedding fonts to be accessible at
  // runtime from the "fonts/" folder. See Makefile.emscripten for details.
  // io.Fonts->AddFontDefault();
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  // ImFont* font =
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
  // nullptr, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != nullptr);

  // configure global opengl state
  // -----------------------------
  glEnable(GL_DEPTH_TEST);

  grapho::VertexAndFragment snippet;
  snippet.Attribute(grapho::ShaderTypes::vec3, "aPos");
  snippet.Attribute(grapho::ShaderTypes::vec3, "aNormal");
  snippet.Attribute(grapho::ShaderTypes::vec2, "aTexCoords");
  snippet.Attribute(grapho::ShaderTypes::vec3, "aTangent");
  snippet.Attribute(grapho::ShaderTypes::vec3, "aBitangent");
  snippet.VsToFs(grapho::ShaderTypes::vec3, "FragPos");
  snippet.VsToFs(grapho::ShaderTypes::vec2, "TexCoords");
  snippet.VsToFs(grapho::ShaderTypes::vec3, "TangentLightPos");
  snippet.VsToFs(grapho::ShaderTypes::vec3, "TangentViewPos");
  snippet.VsToFs(grapho::ShaderTypes::vec3, "TangentFragPos");
  snippet.Uniform(grapho::ShaderTypes::mat4, "projection");
  snippet.Uniform(grapho::ShaderTypes::mat4, "view");
  snippet.Uniform(grapho::ShaderTypes::mat4, "model");
  snippet.Uniform(grapho::ShaderTypes::vec3, "lightPos");
  snippet.Uniform(grapho::ShaderTypes::vec3, "viewPos");
  snippet.VsEntry(R"(
void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));   
    vs_out.TexCoords = aTexCoords;
    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    
    mat3 TBN = transpose(mat3(T, B, N));    
    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos  = TBN * viewPos;
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;
        
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)");
  auto vs = grapho::gl3::GenerateVS(snippet);
  std::cout << "####################" << std::endl
            << vs << std::endl
            << "####################" << std::endl;

  snippet.Out(grapho::ShaderTypes::vec4, "FragColor");
  snippet.Uniform(grapho::ShaderTypes::sampler2D, "diffuseMap");
  snippet.Uniform(grapho::ShaderTypes::sampler2D, "normalMap");
  snippet.FsEntry(R"(
void main()
{           
    // obtain normal from normal map in range [0,1]
    vec3 normal = texture(normalMap, fs_in.TexCoords).rgb;
    // transform normal vector to range [-1,1]
    normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space
   
    // get diffuse color
    vec3 color = texture(diffuseMap, fs_in.TexCoords).rgb;
    // ambient
    vec3 ambient = 0.1 * color;
    // diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;
    FragColor = vec4(ambient + diffuse + specular, 1.0);
    // FragColor = vec4(vec3(diff), 1.0);
    // FragColor = texture(normalMap, fs_in.TexCoords);
})");
  auto fs = grapho::gl3::GenerateFS(snippet);

  std::cout << "####################" << std::endl
            << fs << std::endl
            << "####################" << std::endl;

  // build and compile shaders
  // -------------------------
  auto result = grapho::gl3::ShaderProgram::Create(vs, fs);
  if (!result) {
    std::cout << result.error() << std::endl;
    return 4;
  }
  auto shader = *result;

  // load textures
  // -------------
  unsigned int diffuseMap =
    loadTexture((dir / "resources/textures/brickwall.jpg").string().c_str());
  unsigned int normalMap = loadTexture(
    (dir / "resources/textures/brickwall_normal.jpg").string().c_str());

  // shader configuration
  // --------------------
  shader->Use();
  shader->Uniform("diffuseMap").and_then([&](auto u) {
    u.SetInt(0);
    return NOP;
  });
  shader->Uniform("normalMap").and_then([&](auto u) {
    u.SetInt(1);
    return NOP;
  });

  // lighting info
  // -------------
  glm::vec3 lightPos(0.5f, 1.0f, 0.3f);

  std::vector<grapho::imgui::Dock> docks;

  docks.push_back(
    grapho::imgui::Dock("demo", [](const char* title, bool* popen) {
      ImGui::ShowDemoWindow(popen);
    }));

  // render loop
  // -----------
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static bool resetLayout = false;
    grapho::imgui::DockSpace("dock_space", docks, &resetLayout);

    processInput(window);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    g_camera.SetSize(width, height);
    DirectX::XMFLOAT4X4 projection;
    DirectX::XMFLOAT4X4 view;
    g_camera.Update(&projection._11, &view._11);

    // render
    // ------
    glViewport(0, 0, width, height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader->Use();
    shader->Uniform("projection")->SetMat4(projection);
    shader->Uniform("view")->SetMat4(view);
    shader->Uniform("viewPos").and_then([&](auto u) {
      u.SetFloat3(g_camera.Position);
      return NOP;
    });
    shader->Uniform("lightPos").and_then([&](auto u) {
      u.SetFloat3(lightPos);
      return NOP;
    });

    {
      // render normal-mapped quad
      glm::mat4 model = glm::mat4(1.0f);
      model =
        glm::rotate(model,
                    glm::radians((float)glfwGetTime() * -10.0f),
                    glm::normalize(glm::vec3(
                      1.0, 0.0, 1.0))); // rotate the quad to show normal
                                        // mapping from multiple directions
      shader->Uniform("model")->SetMat4(model);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, diffuseMap);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, normalMap);
      renderQuad();
    }

    {
      // render light source (simply re-renders a smaller plane at the light's
      // position for debugging/visualization)
      auto model = glm::mat4(1.0f);
      model = glm::translate(model, lightPos);
      model = glm::scale(model, glm::vec3(0.1f));
      shader->Uniform("model")->SetMat4(model);
      renderQuad();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we
    // save/restore it to make it easier to paste this code elsewhere.
    //  For this specific demo app we could also call
    //  glfwMakeContextCurrent(window) directly)
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      GLFWwindow* backup_current_context = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backup_current_context);
    }

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved
    // etc.)
    // -------------------------------------------------------------------------------
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
