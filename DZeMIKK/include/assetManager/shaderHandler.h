#ifndef DZEMIKK_SHADER_HANDLE_H
#define DZEMIKK_SHADER_HANDLE_H

#include "IAssetHandler.h"

#include <string>
#include <memory>

namespace dzemikk {
    class Shader;

    class ShaderHandler : public IAssetHandler {
      public:
        void* load(const std::string& path) override;
        void reload(void* asset, const std::string& path) override;
        void unload(void* asset) override;

      private:
        static const std::string VERT;
        static const std::string FRAG;

        static std::unique_ptr<dzemikk::Shader> loadShaderFromFile(const std::string& path);
        static void reloadShader(const std::string& path, Shader* shader);

        static std::pair<std::string, std::string> buildShaderPaths(const std::string& path);

    };

} // namespace dzemikk

#endif // DZEMIKK_SHADER_HANDLE_H
