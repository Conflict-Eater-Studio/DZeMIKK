#ifndef DZEMIKK_PRIMITIVE_FACTORY_HANDLE_H
#define DZEMIKK_PRIMITIVE_FACTORY_HANDLE_H

#include <memory>

namespace dzemikk {
    class Mesh;

    /**
     * @brief Factory for creating basic primitive meshes.
     *
     * Provides static methods for generating common 3D shapes such as cube,
     * quad, sphere, and capsule. Mesh data is generated procedurally.
     *
     * @note Returned meshes are heap-allocated and must be deleted manually.
     */
    class PrimitiveFactory {
      public:
        /**
         * @brief Creates a unit cube mesh with normals.
         */
        static std::unique_ptr<dzemikk::Mesh> createCube();
        
        /**
         * @brief Creates a 2D quad (with UV coordinates).
         */
        static std::unique_ptr<dzemikk::Mesh> createQuad();

        /**
         * @brief Creates a sphere using spherical coordinates.
         */
        static std::unique_ptr<dzemikk::Mesh> createSphere();

        /**
         * @brief Creates a capsule (cylinder + two hemispheres).
         */
        static std::unique_ptr<dzemikk::Mesh> createCapsule();
    };
} // namespace dzemikk

#endif // DZEMIKK_PRIMITIVE_FACTORY_HANDLE_H
