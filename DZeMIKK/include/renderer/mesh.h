#include <glad/glad.h>
#include <cstdint>

namespace dzemikk {
    class Mesh {
      public:
        GLuint vao;
        GLuint vbo;
        uint32_t vertexCount;

        void draw() const;
    };
} 