#include "assetManager/primitiveFactory.h"
#include "renderer/StaticMesh.h"
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/constants.hpp>

std::unique_ptr<dzemikk::Mesh> dzemikk::PrimitiveFactory::createCube() {
    std::vector<StaticVertex> vertices;
    vertices.reserve(36);

    auto add = [&](glm::vec3 pos, glm::vec3 normal, glm::vec2 uv) {
        vertices.push_back({pos, normal, uv});
    };

    // --- Front (+Z)
    add({-0.5f, -0.5f, 0.5f}, {0, 0, 1}, {0, 0});
    add({0.5f, -0.5f, 0.5f}, {0, 0, 1}, {1, 0});
    add({0.5f, 0.5f, 0.5f}, {0, 0, 1}, {1, 1});

    add({0.5f, 0.5f, 0.5f}, {0, 0, 1}, {1, 1});
    add({-0.5f, 0.5f, 0.5f}, {0, 0, 1}, {0, 1});
    add({-0.5f, -0.5f, 0.5f}, {0, 0, 1}, {0, 0});

    // --- Back (-Z)
    add({-0.5f, -0.5f, -0.5f}, {0, 0, -1}, {1, 0});
    add({0.5f, 0.5f, -0.5f}, {0, 0, -1}, {0, 1});
    add({0.5f, -0.5f, -0.5f}, {0, 0, -1}, {0, 0});

    add({0.5f, 0.5f, -0.5f}, {0, 0, -1}, {0, 1});
    add({-0.5f, -0.5f, -0.5f}, {0, 0, -1}, {1, 0});
    add({-0.5f, 0.5f, -0.5f}, {0, 0, -1}, {1, 1});

    // --- Left (-X)
    add({-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {0, 0});
    add({-0.5f, -0.5f, 0.5f}, {-1, 0, 0}, {1, 0});
    add({-0.5f, 0.5f, 0.5f}, {-1, 0, 0}, {1, 1});

    add({-0.5f, 0.5f, 0.5f}, {-1, 0, 0}, {1, 1});
    add({-0.5f, 0.5f, -0.5f}, {-1, 0, 0}, {0, 1});
    add({-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {0, 0});

    // --- Right (+X)
    add({0.5f, -0.5f, -0.5f}, {1, 0, 0}, {1, 0});
    add({0.5f, 0.5f, 0.5f}, {1, 0, 0}, {0, 1});
    add({0.5f, -0.5f, 0.5f}, {1, 0, 0}, {0, 0});

    add({0.5f, 0.5f, 0.5f}, {1, 0, 0}, {0, 1});
    add({0.5f, -0.5f, -0.5f}, {1, 0, 0}, {1, 0});
    add({0.5f, 0.5f, -0.5f}, {1, 0, 0}, {1, 1});

    // --- Top (+Y)
    add({-0.5f, 0.5f, -0.5f}, {0, 1, 0}, {0, 0});
    add({-0.5f, 0.5f, 0.5f}, {0, 1, 0}, {0, 1});
    add({0.5f, 0.5f, 0.5f}, {0, 1, 0}, {1, 1});

    add({0.5f, 0.5f, 0.5f}, {0, 1, 0}, {1, 1});
    add({0.5f, 0.5f, -0.5f}, {0, 1, 0}, {1, 0});
    add({-0.5f, 0.5f, -0.5f}, {0, 1, 0}, {0, 0});

    // --- Bottom (-Y)
    add({-0.5f, -0.5f, -0.5f}, {0, -1, 0}, {0, 1});
    add({0.5f, -0.5f, 0.5f}, {0, -1, 0}, {1, 0});
    add({-0.5f, -0.5f, 0.5f}, {0, -1, 0}, {0, 0});

    add({0.5f, -0.5f, 0.5f}, {0, -1, 0}, {1, 0});
    add({-0.5f, -0.5f, -0.5f}, {0, -1, 0}, {0, 1});
    add({0.5f, -0.5f, -0.5f}, {0, -1, 0}, {1, 1});

    auto mesh = std::make_unique<StaticMesh>();
    mesh->create(vertices, {});
    return mesh;
}

std::unique_ptr<dzemikk::Mesh> dzemikk::PrimitiveFactory::createQuad() {
    std::vector<StaticVertex> vertices = {
        {{0, 0, 0}, {0, 0, 1}, {0, 0}}, {{1, 0, 0}, {0, 0, 1}, {1, 0}},
        {{1, 1, 0}, {0, 0, 1}, {1, 1}},

        {{1, 1, 0}, {0, 0, 1}, {1, 1}}, {{0, 1, 0}, {0, 0, 1}, {0, 1}},
        {{0, 0, 0}, {0, 0, 1}, {0, 0}},
    };

    auto mesh = std::make_unique<StaticMesh>();
    mesh->create(vertices, {});
    return mesh;
}

std::unique_ptr<dzemikk::Mesh> dzemikk::PrimitiveFactory::createSphere() {
    const int stacks = 16;
    const int slices = 16;
    const float radius = 0.5f;

    std::vector<StaticVertex> vertices;
    std::vector<unsigned int> indices;

    vertices.reserve((stacks + 1) * (slices + 1));

    // --- Vertices
    for (int i = 0; i <= stacks; ++i) {
        float v = static_cast<float>(i) / stacks;
        float phi = v * glm::pi<float>();

        for (int j = 0; j <= slices; ++j) {
            float u = static_cast<float>(j) / slices;
            float theta = u * glm::two_pi<float>();

            float x = radius * sinf(phi) * cosf(theta);
            float y = radius * cosf(phi);
            float z = radius * sinf(phi) * sinf(theta);

            glm::vec3 pos(x, y, z);
            glm::vec3 normal = glm::normalize(pos);

            // UV mapping (equirectangular)
            glm::vec2 uv(u, v);

            vertices.push_back({pos, normal, uv});
        }
    }

    // --- Indices
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int first = i * (slices + 1) + j;
            int second = first + slices + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    auto mesh = std::make_unique<StaticMesh>();
    mesh->create(vertices, indices);

    return mesh;
}

std::unique_ptr<dzemikk::Mesh> dzemikk::PrimitiveFactory::createCapsule() {
    const int segments = 24;
    const int rings = 12;

    const float radius = 0.5f;
    const float height = 1.0f;
    float halfH = height * 0.5f;

    std::vector<StaticVertex> vertices;
    std::vector<unsigned int> indices;

    auto addVertex = [&](glm::vec3 pos, glm::vec2 uv) {
        glm::vec3 normal = glm::normalize(pos);
        vertices.push_back({pos, normal, uv});
    };

    for (int i = 0; i <= 1; i++) {
        float y = (-halfH) + ((float)i * height);
        float v = (float)i; // 0 bottom, 1 top

        for (int j = 0; j <= segments; j++) {
            float u = (float)j / segments;
            float a = u * glm::two_pi<float>();

            glm::vec3 pos(cos(a) * radius, y, sin(a) * radius);

            addVertex(pos, {u, v});
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

    std::size_t baseTop = vertices.size();

    for (int i = 0; i <= rings; i++) {
        float v = (float)i / rings;
        float phi = v * glm::half_pi<float>();

        for (int j = 0; j <= segments; j++) {
            float u = (float)j / segments;
            float theta = u * glm::two_pi<float>();

            glm::vec3 pos(cos(theta) * cos(phi) * radius, (sin(phi) * radius) + halfH,
                          sin(theta) * cos(phi) * radius);

            glm::vec2 uv(u, 1.0f - v);

            addVertex(pos, uv);
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

    std::size_t baseBottom = vertices.size();

    for (int i = 0; i <= rings; i++) {
        float v = (float)i / rings;
        float phi = v * glm::half_pi<float>();

        for (int j = 0; j <= segments; j++) {
            float u = (float)j / segments;
            float theta = u * glm::two_pi<float>();

            glm::vec3 pos(cos(theta) * cos(phi) * radius, ((-sin(phi)) * radius) - halfH,
                          sin(theta) * cos(phi) * radius);

            glm::vec2 uv(u, v);

            addVertex(pos, uv);
        }
    }

    for (int i = 0; i < rings; i++) {
        for (int j = 0; j < segments; j++) {
            int a = (int)baseBottom + (i * (segments + 1)) + j;
            int b = a + segments + 1;

            indices.insert(indices.end(), {(unsigned)a, (unsigned)a + 1, (unsigned)b,
                                           (unsigned)a + 1, (unsigned)b + 1, (unsigned)b});
        }
    }

    auto mesh = std::make_unique<StaticMesh>();
    mesh->create(vertices, indices);

    return mesh;
}