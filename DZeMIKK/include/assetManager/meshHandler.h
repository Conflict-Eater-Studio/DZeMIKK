#ifndef DZEMIKK_MESH_HANDLE_H
#define DZEMIKK_MESH_HANDLE_H

#include "IAssetHandler.h"

#include <string>

namespace dzemikk {
    class Mesh;

    class MeshHandler : public IAssetHandler {
      public:
        void* load(const std::string& path) override;
        void reload(void* asset, const std::string& path) override;
        void unload(void* asset) override;

      private:
        Mesh* loadMeshFromFile(const std::string& path);
        void reloadMesh(const std::string& path, Mesh* mesh);
    };

} // namespace dzemikk

#endif // DZEMIKK_MESH_HANDLE_H