#ifndef DZEMIKK_MESH_HANDLE_H
#define DZEMIKK_MESH_HANDLE_H

#include "IAssetHandler.h"

#include <string>
#include <memory>

namespace dzemikk {
    class Mesh;

    /**
     * @brief Asset handler responsible for loading and managing Mesh resources.
     *
     * Provides loading, reloading, and unloading of mesh assets from files.
     * Acts as a bridge between file data and Mesh runtime objects.
     *
     * @note Loaded meshes are heap-allocated and must be released via unload().
     * @warning Uses void* interface — requires casting to Mesh*.
     */
    class MeshHandler : public IAssetHandler {
      public:
        /**
         * @brief Loads a mesh from file.
         *
         * @param path Path to the mesh file.
         * @return void* Pointer to Mesh.
         */
        void* load(const std::string& path) override;

        /**
         * @brief Reloads an existing mesh from file.
         *
         * @param asset Pointer to Mesh (as void*).
         * @param path Path to the mesh file.
         */
        void reload(void* asset, const std::string& path) override;

        /**
         * @brief Unloads a mesh from memory.
         *
         * @param asset Pointer to Mesh (as void*).
         */
        void unload(void* asset) override;

      private:
        /**
         * @brief Loads a mesh from file and creates a Mesh object.
         */
        static std::unique_ptr<dzemikk::Mesh> loadMeshFromFile(const std::string& path);

        /**
         * @brief Reloads mesh data into an existing Mesh object.
         */
        void reloadMesh(const std::string& path, Mesh* mesh);
    };

} // namespace dzemikk

#endif // DZEMIKK_MESH_HANDLE_H