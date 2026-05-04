#ifndef DZEMIKK_MESH_HANDLE_H
#define DZEMIKK_MESH_HANDLE_H

#include "IAssetHandler.h"

#include <string>
#include <memory>

namespace dzemikk {
    class Mesh;

    /**
     * @brief Handles loading, reloading, and unloading of Mesh assets.
     *
     * MeshHandler converts mesh files into runtime Mesh objects
     * and integrates them with the engine asset system.
     *
     * Supports safe lifetime management via AssetHandle and hot-reload capability.
     */
    class MeshHandler : public IAssetHandler<Mesh> {
      public:
        using Handle = AssetHandle<Mesh>;
        using Result = AssetResult<Mesh>;
        
        /**
         * @brief Loads a mesh from file.
         *
         * @param path Path to the mesh file.
         * @return AssetResult containing a valid Mesh handle or error.
         */
        Result load(const std::string& path,
                    LoadExecutionMode loadExecutionMode = LoadExecutionMode::Sync) override;

        /**
         * @brief Reloads an existing mesh asset.
         *
         * Used for hot-reloading during development or runtime updates.
         *
         * @param asset Reference to the existing mesh handle.
         * @param path Path to the mesh file.
         * @return True if reload succeeded, false otherwise.
         */
        bool reload(Handle& asset, const std::string& path) override;

        /**
         * @brief Unloads a mesh asset from memory.
         *
         * Releases ownership of the underlying Mesh resource.
         *
         * @param asset Reference to the mesh handle to unload.
         */
        void unload(Handle& asset) override;

      private:
        /**
         * @brief Internal helper that loads a Mesh from file.
         *
         * @param path Path to mesh file.
         * @return Shared pointer to loaded Mesh or nullptr on failure.
         */
        static std::shared_ptr<Mesh> loadMeshFromFile(const std::string& path);

        /**
         * @brief Reloads mesh data into an existing Mesh instance.
         *
         * @param path Path to mesh file.
         * @param mesh Reference to mesh instance to update.
         * @return True if reload succeeded, false otherwise.
         */
        static bool reloadMesh(const std::string& path, Mesh& mesh);
    };

} // namespace dzemikk

#endif // DZEMIKK_MESH_HANDLE_H