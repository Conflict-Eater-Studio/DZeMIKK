#include "assetManager/primitiveFactory.h"
#include "renderer/mesh.h"
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/constants.hpp>

std::unique_ptr<dzemikk::Mesh> dzemikk::PrimitiveFactory::createCube() {
    auto mesh = std::make_unique<dzemikk::Mesh>();

    float vertices[] = {// --- Front face
                            -0.5F, -0.5F, 0.5F, 0.0F, 0.0F, 1.0F, 0.5F, -0.5F, 0.5F, 0.0F, 0.0F, 1.0F,
                            0.5F, 0.5F, 0.5F, 0.0F, 0.0F, 1.0F,

                            0.5F, 0.5F, 0.5F, 0.0F, 0.0F, 1.0F, -0.5F, 0.5F, 0.5F, 0.0F, 0.0F, 1.0F,
                            -0.5F, -0.5F, 0.5F, 0.0F, 0.0F, 1.0F,

                            // --- Back face
                            -0.5F, -0.5F, -0.5F, 0.0F, 0.0F, -1.0F, 0.5F, 0.5F, -0.5F, 0.0F, 0.0F,
                            -1.0F, 0.5F, -0.5F, -0.5F, 0.0F, 0.0F, -1.0F,

                            0.5F, 0.5F, -0.5F, 0.0F, 0.0F, -1.0F, -0.5F, -0.5F, -0.5F, 0.0F, 0.0F,
                            -1.0F, -0.5F, 0.5F, -0.5F, 0.0F, 0.0F, -1.0F,

                            // --- Left face
                            -0.5F, -0.5F, -0.5F, -1.0F, 0.0F, 0.0F, -0.5F, -0.5F, 0.5F, -1.0F, 0.0F,
                            0.0F, -0.5F, 0.5F, 0.5F, -1.0F, 0.0F, 0.0F,

                            -0.5F, 0.5F, 0.5F, -1.0F, 0.0F, 0.0F, -0.5F, 0.5F, -0.5F, -1.0F, 0.0F, 0.0F,
                            -0.5F, -0.5F, -0.5F, -1.0F, 0.0F, 0.0F,

                            // --- Right face
                            0.5F, -0.5F, -0.5F, 1.0F, 0.0F, 0.0F, 0.5F, 0.5F, 0.5F, 1.0F, 0.0F, 0.0F,
                            0.5F, -0.5F, 0.5F, 1.0F, 0.0F, 0.0F,

                            0.5F, 0.5F, 0.5F, 1.0F, 0.0F, 0.0F, 0.5F, -0.5F, -0.5F, 1.0F, 0.0F, 0.0F,
                            0.5F, 0.5F, -0.5F, 1.0F, 0.0F, 0.0F,

                            // --- Top face
                            -0.5F, 0.5F, -0.5F, 0.0F, 1.0F, 0.0F, -0.5F, 0.5F, 0.5F, 0.0F, 1.0F, 0.0F,
                            0.5F, 0.5F, 0.5F, 0.0F, 1.0F, 0.0F,

                            0.5F, 0.5F, 0.5F, 0.0F, 1.0F, 0.0F, 0.5F, 0.5F, -0.5F, 0.0F, 1.0F, 0.0F,
                            -0.5F, 0.5F, -0.5F, 0.0F, 1.0F, 0.0F,

                            // --- Bottom face
                            -0.5F, -0.5F, -0.5F, 0.0F, -1.0F, 0.0F, 0.5F, -0.5F, 0.5F, 0.0F, -1.0F,
                            0.0F, -0.5F, -0.5F, 0.5F, 0.0F, -1.0F, 0.0F,

                            0.5F, -0.5F, 0.5F, 0.0F, -1.0F, 0.0F, -0.5F, -0.5F, -0.5F, 0.0F, -1.0F,
                            0.0F, 0.5F, -0.5F, -0.5F, 0.0F, -1.0F, 0.0F};

    mesh->create(vertices, 36, 6);
    return mesh;
}

std::unique_ptr<dzemikk::Mesh> dzemikk::PrimitiveFactory::createQuad() {
    auto mesh = std::make_unique<dzemikk::Mesh>();
    float vertices[] = {// pos        // uv
                        0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F, 1.0F, 0.0F, 1.0F, 1.0F, 1.0F, 1.0F,

                        1.0F, 1.0F, 1.0F, 1.0F, 0.0F, 1.0F, 0.0F, 1.0F, 0.0F, 0.0F, 0.0F, 0.0F};

    mesh->create2D(vertices, 6);
    return mesh;
}

std::unique_ptr<dzemikk::Mesh> dzemikk::PrimitiveFactory::createSphere() {
    const int stacks = 16;
    const int slices = 16;
    const float radius = 0.5F;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    for (int i = 0; i <= stacks; ++i) {
        float v = (float)i / stacks;
        float phi = v * glm::pi<float>();

        for (int j = 0; j <= slices; ++j) {
            float u = (float)j / slices;
            float theta = u * glm::two_pi<float>();

            float x = radius * sinf(phi) * cosf(theta);
            float y = radius * cosf(phi);
            float z = radius * sinf(phi) * sinf(theta);

            glm::vec3 pos(x, y, z);
            glm::vec3 normal = glm::normalize(pos);

            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            vertices.insert(vertices.end(), {pos.x, pos.y, pos.z, normal.x, normal.y, normal.z});
        }
    }

    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int first = (i * (slices + 1)) + j;
            int second = first + slices + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    auto mesh = std::make_unique<dzemikk::Mesh>();
    mesh->createIndexed(vertices.data(), vertices.size() / 6, indices.data(), indices.size(), 6);

    return mesh;
}

std::unique_ptr<dzemikk::Mesh> dzemikk::PrimitiveFactory::createCapsule() {
    auto mesh = std::make_unique<dzemikk::Mesh>();

    const int segments = 24;
    const int rings = 12;

    const float radius = 0.5F;
    const float height = 1.0F;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    auto addVertex = [&](const glm::vec3& pos) {
        glm::vec3 normal = glm::normalize(pos);

        vertices.push_back(pos.x);
        vertices.push_back(pos.y);
        vertices.push_back(pos.z);

        vertices.push_back(normal.x);
        vertices.push_back(normal.y);
        vertices.push_back(normal.z);
    };

    auto indexOf = [&](int ring, int seg) { return (ring * (segments + 1)) + seg; };

    float halfH = height * 0.5F;

    for (int i = 0; i <= 1; i++) {
        float y = (-halfH) + ((float)i * height);

        for (int j = 0; j <= segments; j++) {
            float a = (float)j / segments * glm::two_pi<float>();

            glm::vec3 pos(cos(a) * radius, y, sin(a) * radius);
            addVertex(pos);
        }
    }

    for (int j = 0; j < segments; j++) {
        int i0 = j;
        int i1 = j + 1;
        int i2 = j + segments + 1;
        int i3 = j + segments + 2;

        indices.insert(indices.end(), {(unsigned)i0, (unsigned)i2, (unsigned)i1, (unsigned)i1,
                                       (unsigned)i2, (unsigned)i3});
    }

    std::size_t baseTop = vertices.size() / 6;

    for (int i = 0; i <= rings; i++) {
        float v = (float)i / rings;
        float phi = v * (glm::half_pi<float>());

        for (int j = 0; j <= segments; j++) {
            float u = (float)j / segments;
            float theta = u * glm::two_pi<float>();

            glm::vec3 pos(cos(theta) * cos(phi) * radius, (sin(phi) * radius) + halfH,
                          sin(theta) * cos(phi) * radius);

            addVertex(pos);
        }
    }

    for (int i = 0; i < rings; i++) {
        for (int j = 0; j < segments; j++) {
            int a = (int)baseTop + (i * (segments + 1)) + j;
            int b = a + segments + 1;

            indices.insert(indices.end(), {(unsigned)a, (unsigned)b, (unsigned)a + 1,
                                           (unsigned)a + 1, (unsigned)b, (unsigned)b + 1});
        }
    }

    std::size_t baseBottom = vertices.size() / 6;

    for (int i = 0; i <= rings; i++) {
        float v = (float)i / rings;
        float phi = v * (glm::half_pi<float>());

        for (int j = 0; j <= segments; j++) {
            float u = (float)j / segments;
            float theta = u * glm::two_pi<float>();

            glm::vec3 pos(cos(theta) * cos(phi) * radius, ((-sin(phi)) * radius) - halfH,
                          sin(theta) * cos(phi) * radius);

            addVertex(pos);
        }
    }

    for (int i = 0; i < rings; i++) {
        for (int j = 0; j < segments; j++) {
            int a = (int)baseBottom + ((int)i * (segments + 1)) + j;
            int b = a + segments + 1;

            indices.insert(indices.end(), {(unsigned)a, (unsigned)a + 1, (unsigned)b,
                                           (unsigned)a + 1, (unsigned)b + 1, (unsigned)b});
        }
    }

    mesh->createIndexed(vertices.data(), vertices.size() / 6, indices.data(), indices.size(), 6);

    return mesh;
}
