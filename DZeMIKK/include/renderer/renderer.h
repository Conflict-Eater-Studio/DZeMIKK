#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace dzemikk {
    class Renderer {
      private:
        unsigned int cubeVAO;
        unsigned int cubeVBO;
        unsigned int cubeShader;

        unsigned int rectVAO;
        unsigned int rectVBO;
        unsigned int rectShader;

        void initCube();
        void initCubeShader();

        void initRectangle();
        void initRectShader();

      public:
        Renderer();
        ~Renderer();

        void DrawCube();
        void DrawRectangle();
    };
} 