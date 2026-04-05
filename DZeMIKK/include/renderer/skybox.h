#ifndef DZEMIKK_SKYBOX_H
#define DZEMIKK_SKYBOX_H

#include "renderer/mesh.h"
#include "renderer/shader.h"

#include <string>
#include <vector>

namespace dzemikk {

class Skybox {
  public:
    Skybox();
    ~Skybox();

    void loadCubemap(const std::vector<std::string>& faces);

    void setColor(const glm::vec3& color);

    void render(const glm::mat4& view, const glm::mat4& projection);

  private:
    Mesh* _cubeMesh = nullptr;
    Shader* _shader = nullptr;
    GLuint _cubemapTex = 0;
    glm::vec3 _color = glm::vec3(0.5f, 0.7f, 1.0f); 
    bool _useCubemap = false;

    void initCube();
};

} // namespace dzemikk

#endif // DZEMIKK_SKYBOX_H