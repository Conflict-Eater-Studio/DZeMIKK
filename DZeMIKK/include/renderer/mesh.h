#include <glad/glad.h>
#include <cstdint>

namespace dzemikk {
    class Mesh {
      public:
        GLuint vao;
        GLuint vbo;
        GLuint ebo = 0;

        uint32_t vertexCount = 0;
        uint32_t indexCount = 0;

        bool useIndices = false;

        void draw() const;
    };
} 