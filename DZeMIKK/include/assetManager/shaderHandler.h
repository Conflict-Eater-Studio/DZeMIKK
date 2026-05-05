#ifndef DZEMIKK_SHADER_HANDLE_H
#define DZEMIKK_SHADER_HANDLE_H

#include "IAssetHandler.h"

#include <string>
#include <memory>

namespace dzemikk {
    class Shader;

    /**
     * @brief Handles loading, reloading, and unloading of Shader assets.
     *
     * ShaderHandler loads shader programs from file pairs (e.g. vertex + fragment)
     * and creates runtime Shader objects used by the rendering system.
     *
     * Supports hot-reloading, allowing shaders to be recompiled at runtime.
     */
    class ShaderHandler : public IAssetHandler<Shader> {
      public:
        using Handle = AssetHandle<Shader>;
        using Result = AssetResult<Shader>;

        /**
         * @brief Loads a shader from base path.
         *
         * The handler expects shader files to be split into stages
         * (e.g. .vert and .frag).
         *
         * @param path Base path to shader (without extension or with custom convention).
         * @return AssetResult containing a valid Shader handle or error.
         */
        Result load(const std::string& path) override;

        /**
         * @brief Reloads an existing shader.
         *
         * Recompiles shader sources and updates the GPU program.
         *
         * @param asset Reference to shader handle.
         * @param path Base path to shader.
         * @return True if reload succeeded, false otherwise.
         */
        bool reload(Handle& asset, const std::string& path) override;

        /**
         * @brief Unloads a shader from memory.
         *
         * Releases ownership of the shader resource.
         *
         * @param asset Shader handle to unload.
         */
        void unload(Handle& asset) override;

      private:
        /**
         * @brief Vertex shader file extension.
         */
        static const std::string VERT;

        /**
         * @brief Fragment shader file extension.
         */
        static const std::string FRAG;

        /**
         * @brief Loads and compiles a shader from disk.
         *
         * @param path Base path to shader.
         * @return Shared pointer to Shader or nullptr.
         */
        static std::shared_ptr<Shader> loadShaderFromFile(const std::string& path);

        /**
         * @brief Reloads shader data into existing instance.
         *
         * @param path Base path to shader.
         * @param shader Reference to shader instance.
         * @return True if reload succeeded.
         */
        static bool reloadShader(const std::string& path, Shader& shader);

        /**
         * @brief Builds full file paths for shader stages.
         *
         * @param path Base path to shader.
         * @return Pair of (vertex path, fragment path).
         */
        static std::pair<std::string, std::string> buildShaderPaths(const std::string& path);

    };

} // namespace dzemikk

#endif // DZEMIKK_SHADER_HANDLE_H
