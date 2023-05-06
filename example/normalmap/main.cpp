#include "normal_mapping_fs.h"
#include "normal_mapping_vs.h"
#include "normalmap.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/shadergenerator.h>
#include <grapho/orbitview.h>
#include <grapho/shadersnippet.h>
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

  // render loop
  // -----------
  while (!glfwWindowShouldClose(window)) {
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

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved
    // etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
