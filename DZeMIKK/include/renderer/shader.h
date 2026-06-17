#ifndef DZEMIKK_SHADER_H
#define DZEMIKK_SHADER_H

#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

#include "assetManager/iGpuUploadable.h"

namespace dzemikk {
    /**
     * @brief Encapsulates an OpenGL shader program.
     *
     * Provides utilities to compile, link, and set uniforms (scalars, vectors, matrices, samplers).
     */
    class Shader : public IGpuUploadable {
    public:
        #pragma region Construction / Destruction

        /**
         * @brief Constructs a shader from vertex and fragment source code.
         *
         * @param vertexSrc Vertex shader GLSL source.
         * @param fragmentSrc Fragment shader GLSL source.
         */
        explicit Shader(const char* vertexSrc, const char* fragmentSrc);

        
        /**
         * @brief Deletes the shader program on destruction.
         */
        virtual ~Shader();

        #pragma endregion

        #pragma region Binding

        /**
         * @brief Binds the shader program for use.
         */
        void bind() const;

        
        /**
         * @brief Unbinds any shader program.
         */
        void unbind() const;

        #pragma endregion
        
        /**
         * @brief Uploads resource data to the GPU.
         *
         * Transfers CPU-side asset data into GPU memory so it can be used
         * for rendering. Called after the asset has been fully loaded.
         */
        void uploadToGPU() override;

        #pragma region Copy / Move semantics

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
        Shader(Shader&&) noexcept;
        Shader& operator=(Shader&&) noexcept;

        #pragma endregion

        #pragma region Uniform setters

        // --- Scalars
        void setFloat(const char* name, float value);
        void setInt(const char* name, int value);
        void setBool(const char* name, bool value);

        // --- Vectors
        void setVec2(const char* name, const glm::vec2& vec);
        void setVec3(const char* name, const glm::vec3& vec);
        void setVec4(const char* name, const glm::vec4& vec);
        void setIVec2(const char* name, const glm::ivec2& vec);
        void setIVec3(const char* name, const glm::ivec3& vec);
        void setIVec4(const char* name, const glm::ivec4& vec);
        void setBVec2(const char* name, const glm::bvec2& vec);
        void setBVec3(const char* name, const glm::bvec3& vec);
        void setBVec4(const char* name, const glm::bvec4& vec);

        // --- Matrices
        void setMat2(const char* name, const glm::mat2& mat);
        void setMat3(const char* name, const glm::mat3& mat);
        void setMat4(const char* name, const glm::mat4& mat);
        void setMat4Array(const char* name, const std::vector<glm::mat4>& matrices) const;

        // --- Texture samplers
        void setSampler(const char* name, int textureUnit);

        #pragma endregion

        #pragma region Getters

        /**
         * @brief Returns the OpenGL shader program ID.
         */
        [[nodiscard]] const GLuint getProgramID() const;

        #pragma endregion

        void recompile(const char* vertSrc, const char* fragSrc);

        bool operator==(const Shader& other) const {
            return _program == other._program;
        }

        bool operator!=(const Shader& other) const {
            return !(*this == other);
        }

    private:
        #pragma region Internal state

        GLuint _program = 0;
        std::string _vertSrc;
        std::string _fragSrc;

        #pragma endregion

        #pragma region Internal helpers

        /**
         * @brief Checks compilation/linking errors and prints messages.
         *
         * @param shader Shader or program ID.
         * @param type Type string: "VERTEX", "FRAGMENT", "PROGRAM".
         */
        void checkCompileErrors(GLuint shader, const char* type) const;

        #pragma endregion
    };
} // namespace dzemikk
#endif // DZEMIKK_SHADER_H