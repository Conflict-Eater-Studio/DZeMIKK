#include "assetManager/primitiveFactory.h"
#include "renderer/StaticMesh.h"
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/constants.hpp>

std::unique_ptr<dzemikk::Mesh> dzemikk::PrimitiveFactory::createCube() {
    std::vector<StaticVertex> vertices;
    vertices.reserve(36);

    auto add = [&](glm::vec3 pos, glm::vec3 normal, glm::vec2 uv, glm::vec3 tangent,
                       glm::vec3 bitangent) {
            StaticVertex v(pos, normal, uv, tangent, bitangent);
            vertices.push_back(v);
        };

    // --- Front (+Z)
    glm::vec3 T(1, 0, 0);
    glm::vec3 B(0, 1, 0);

    add({-0.5F, -0.5F, 0.5F}, {0, 0, 1}, {0, 0}, T, B);
    add({0.5F, -0.5F, 0.5F}, {0, 0, 1}, {1, 0}, T, B);
    add({0.5F, 0.5F, 0.5F}, {0, 0, 1}, {1, 1}, T, B);

    add({0.5F, 0.5F, 0.5F}, {0, 0, 1}, {1, 1}, T, B);
    add({-0.5F, 0.5F, 0.5F}, {0, 0, 1}, {0, 1}, T, B);
    add({-0.5F, -0.5F, 0.5F}, {0, 0, 1}, {0, 0}, T, B);

    // --- Back (-Z)
    T = glm::vec3(-1, 0, 0);
    B = glm::vec3(0, 1, 0);

    add({-0.5F, -0.5F, -0.5F}, {0, 0, -1}, {1, 0}, T, B);
    add({0.5F, 0.5F, -0.5F}, {0, 0, -1}, {0, 1}, T, B);
    add({0.5F, -0.5F, -0.5F}, {0, 0, -1}, {0, 0}, T, B);

    add({0.5F, 0.5F, -0.5F}, {0, 0, -1}, {0, 1}, T, B);
    add({-0.5F, -0.5F, -0.5F}, {0, 0, -1}, {1, 0}, T, B);
    add({-0.5F, 0.5F, -0.5F}, {0, 0, -1}, {1, 1}, T, B);

    // --- Left (-X)
    T = glm::vec3(0, 0, 1);
    B = glm::vec3(0, 1, 0);

    add({-0.5F, -0.5F, -0.5F}, {-1, 0, 0}, {0, 0}, T, B);
    add({-0.5F, -0.5F, 0.5F}, {-1, 0, 0}, {1, 0}, T, B);
    add({-0.5F, 0.5F, 0.5F}, {-1, 0, 0}, {1, 1}, T, B);

    add({-0.5F, 0.5F, 0.5F}, {-1, 0, 0}, {1, 1}, T, B);
    add({-0.5F, 0.5F, -0.5F}, {-1, 0, 0}, {0, 1}, T, B);
    add({-0.5F, -0.5F, -0.5F}, {-1, 0, 0}, {0, 0}, T, B);

    // --- Right (+X)
    T = glm::vec3(0, 0, -1);
    B = glm::vec3(0, 1, 0);

    add({0.5F, -0.5F, -0.5F}, {1, 0, 0}, {1, 0}, T, B);
    add({0.5F, 0.5F, 0.5F}, {1, 0, 0}, {0, 1}, T, B);
    add({0.5F, -0.5F, 0.5F}, {1, 0, 0}, {0, 0}, T, B);

    add({0.5F, 0.5F, 0.5F}, {1, 0, 0}, {0, 1}, T, B);
    add({0.5F, -0.5F, -0.5F}, {1, 0, 0}, {1, 0}, T, B);
    add({0.5F, 0.5F, -0.5F}, {1, 0, 0}, {1, 1}, T, B);

    // --- Top (+Y)
    T = glm::vec3(1, 0, 0);
    B = glm::vec3(0, 0, -1);

    add({-0.5F, 0.5F, -0.5F}, {0, 1, 0}, {0, 0}, T, B);
    add({-0.5F, 0.5F, 0.5F}, {0, 1, 0}, {0, 1}, T, B);
    add({0.5F, 0.5F, 0.5F}, {0, 1, 0}, {1, 1}, T, B);

    add({0.5F, 0.5F, 0.5F}, {0, 1, 0}, {1, 1}, T, B);
    add({0.5F, 0.5F, -0.5F}, {0, 1, 0}, {1, 0}, T, B);
    add({-0.5F, 0.5F, -0.5F}, {0, 1, 0}, {0, 0}, T, B);

    // --- Bottom (-Y)
    T = glm::vec3(1, 0, 0);
    B = glm::vec3(0, 0, 1);

    add({-0.5F, -0.5F, -0.5F}, {0, -1, 0}, {0, 1}, T, B);
    add({0.5F, -0.5F, 0.5F}, {0, -1, 0}, {1, 0}, T, B);
    add({-0.5F, -0.5F, 0.5F}, {0, -1, 0}, {0, 0}, T, B);

    add({0.5F, -0.5F, 0.5F}, {0, -1, 0}, {1, 0}, T, B);
    add({-0.5F, -0.5F, -0.5F}, {0, -1, 0}, {0, 1}, T, B);
    add({0.5F, -0.5F, -0.5F}, {0, -1, 0}, {1, 1}, T, B);

    auto mesh = std::make_unique<StaticMesh>();
    mesh->create(vertices, {});
    mesh->uploadToGPU();
    return mesh;
}

std::unique_ptr<dzemikk::Mesh> dzemikk::PrimitiveFactory::createQuad() {
    StaticVertex v0;
    v0.position = {0, 0, 0};
    v0.normal = {0, 0, 1};
    v0.uv = {0, 0};
    v0.tangent = {1, 0, 0};
    v0.bitangent = {0, 1, 0};

    StaticVertex v1;
    v1.position = {1, 0, 0};
    v1.normal = {0, 0, 1};
    v1.uv = {1, 0};
    v1.tangent = {1, 0, 0};
    v1.bitangent = {0, 1, 0};

    StaticVertex v2;
    v2.position = {1, 1, 0};
    v2.normal = {0, 0, 1};
    v2.uv = {1, 1};
    v2.tangent = {1, 0, 0};
    v2.bitangent = {0, 1, 0};

    StaticVertex v3 = v2;

    StaticVertex v4;
    v4.position = {0, 1, 0};
    v4.normal = {0, 0, 1};
    v4.uv = {0, 1};
    v4.tangent = {1, 0, 0};
    v4.bitangent = {0, 1, 0};

    StaticVertex v5 = v0;

    std::vector<StaticVertex> vertices = {v0, v1, v2, v3, v4, v5};

    auto mesh = std::make_unique<StaticMesh>();
    mesh->create(vertices, {});
    mesh->uploadToGPU();
    return mesh;
}

std::unique_ptr<dzemikk::Mesh> dzemikk::PrimitiveFactory::createSphere() {
    const int stacks = 16;
    const int slices = 16;
    const float radius = 0.5F;

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

            glm::vec3 tangent(-sinf(theta), 0.0f, cosf(theta));

            tangent = glm::normalize(tangent);

            glm::vec3 bitangent = glm::normalize(glm::cross(normal, tangent));

            glm::vec2 uv(u, v);

            StaticVertex vertex;
            vertex.position = pos;
            vertex.normal = normal;
            vertex.uv = uv;
            vertex.tangent = tangent;
            vertex.bitangent = bitangent;

            vertices.push_back(vertex);
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
    mesh->uploadToGPU();

    return mesh;
}

std::unique_ptr<dzemikk::Mesh> dzemikk::PrimitiveFactory::createCapsule() {
    const int segments = 24;
    const int rings = 12;

    const float radius = 0.5F;
    const float height = 1.0F;
    float halfH = height * 0.5F;

    std::vector<StaticVertex> vertices;
    std::vector<unsigned int> indices;

    auto addVertex = [&](glm::vec3 pos, glm::vec3 normal, float theta, glm::vec2 uv) {
        glm::vec3 tangent(-sinf(theta), 0.0f, cosf(theta));

        tangent = glm::normalize(tangent);

        glm::vec3 bitangent = glm::normalize(glm::cross(normal, tangent));

        StaticVertex v;
        v.position = pos;
        v.normal = normal;
        v.uv = uv;
        v.tangent = tangent;
        v.bitangent = bitangent;

        vertices.push_back(v);
    };

    for (int i = 0; i <= 1; i++) {
        float y = (-halfH) + ((float)i * height);
        float v = (float)i; // 0 bottom, 1 top

        for (int j = 0; j <= segments; j++) {
            float u = (float)j / segments;
            float a = u * glm::two_pi<float>();

            glm::vec3 pos(cos(a) * radius, y, sin(a) * radius);

            glm::vec3 normal = glm::normalize(glm::vec3(cos(a), 0.0f, sin(a)));

            addVertex(pos, normal, a, {u, v});
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

            glm::vec2 uv(u, 1.0F - v);

            glm::vec3 normal =
                glm::normalize(glm::vec3(cos(theta) * cos(phi), sin(phi), sin(theta) * cos(phi)));

            addVertex(pos, normal, theta, uv);
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

            glm::vec3 normal =
                glm::normalize(glm::vec3(cos(theta) * cos(phi), -sin(phi), sin(theta) * cos(phi)));

            addVertex(pos, normal, theta, uv);
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
    mesh->uploadToGPU();
    return mesh;
}