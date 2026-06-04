#ifndef DZEMIKK_PRIMITIVE_MESH_LIBRARY_H
#define DZEMIKK_PRIMITIVE_MESH_LIBRARY_H

#include <cstdint>
#include <memory>
#include <unordered_map>

namespace dzemikk {
class Mesh;
/**
 * @brief Built-in primitive mesh factory/cache.
 *
 * Provides access to pre-generated engine meshes such as cubes,
 * quads, spheres, etc. Each mesh is created once and cached for reuse.
 */
class PrimitiveMeshLibrary {
  public:
    PrimitiveMeshLibrary();
    ~PrimitiveMeshLibrary() = default;

    PrimitiveMeshLibrary(const PrimitiveMeshLibrary&) = delete;
    PrimitiveMeshLibrary& operator=(const PrimitiveMeshLibrary&) = delete;
    PrimitiveMeshLibrary(PrimitiveMeshLibrary&&) noexcept = delete;
    PrimitiveMeshLibrary& operator=(PrimitiveMeshLibrary&&) noexcept = delete;

    /**
     * @brief Types of built-in primitive meshes.
     */
    enum class PrimitiveMesh : std::uint8_t { Cube, Quad, Sphere, Capsule };

    /**
     * @brief Initializes all primitive meshes.
     */
    void init();

    /**
     * @brief Retrieves a cached primitive mesh.
     *
     * @param type Primitive mesh type.
     * @return Mesh* Pointer to mesh or nullptr if not initialized.
     */
    [[nodiscard]] std::shared_ptr<Mesh> get(PrimitiveMesh type);

    /**
     * @brief Clears all cached meshes.
     */
    void clear();

  private:
    /**
     * @brief Internal storage of generated primitive meshes.
     */
    std::unordered_map<PrimitiveMesh, std::shared_ptr<Mesh>> _meshes;
};
} // namespace dzemikk

#endif // DZEMIKK_PRIMITIVE_MESH_LIBRARY_H