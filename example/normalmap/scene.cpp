#include <GL/glew.h>

#include "normalmap.h"
#include "scene.h"
#include <glm/gtc/matrix_transform.hpp>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/shadergenerator.h>
#include <grapho/shadersnippet.h>
#include <iostream>

bool
Scene::Initialize(const std::filesystem::path& dir)
{
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
  Shader = grapho::gl3::ShaderProgram::Create(vs, fs);
  if (!Shader) {
    // std::cout << result.error() << std::endl;
    return false;
  }

  // load textures
  // -------------
  DiffuseMap =
    loadTexture((dir / "resources/textures/brickwall.jpg").string().c_str());
  NormalMap = loadTexture(
    (dir / "resources/textures/brickwall_normal.jpg").string().c_str());

  // shader configuration
  // --------------------
  Shader->Use();
  Shader->SetUniform("diffuseMap", 0);
  Shader->SetUniform("normalMap", 1);

  return true;
}

void
Scene::Render(float deltaTime,
              const DirectX::XMFLOAT4X4& projection,
              const DirectX::XMFLOAT4X4& view,
              const DirectX::XMFLOAT3& cameraPosition)
{
  glEnable(GL_DEPTH_TEST);
  Shader->Use();
  Shader->SetUniform("projection", projection);
  Shader->SetUniform("view", view);
  Shader->SetUniform("viewPos", cameraPosition);
  Shader->SetUniform("lightPos", LightPos);

  {
    Time += deltaTime;
    // render normal-mapped quad
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model,
                        glm::radians((float)Time * -10.0f),
                        glm::normalize(glm::vec3(
                          1.0, 0.0, 1.0))); // rotate the quad to show normal
                                            // mapping from multiple directions

    Shader->SetUniform("model", model);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, DiffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, NormalMap);
    renderQuad();
  }

  {
    // render light source (simply re-renders a smaller plane at the light's
    // position for debugging/visualization)
    auto model = glm::mat4(1.0f);
    model = glm::translate(model, LightPos);
    model = glm::scale(model, glm::vec3(0.1f));
    Shader->SetUniform("model", model);
    renderQuad();
  }
}
