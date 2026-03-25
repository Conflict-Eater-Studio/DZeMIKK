#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace dzemikk {
    class Shader {
      public:
        GLuint program;

        Shader(const char* vertexSrc, const char* fragmentSrc);
        ~Shader();
        void bind() const;
        void setMat4(const char* name, const glm::mat4& mat);
    };
} 