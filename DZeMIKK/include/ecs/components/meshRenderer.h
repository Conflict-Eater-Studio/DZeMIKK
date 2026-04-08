#ifndef DZEMIKK_MESH_RENDERER_H
#define DZEMIKK_MESH_RENDERER_H

#include "../component.h"
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/quaternion_geometric.hpp>

namespace dzemikk {
    class Transform;
    class Mesh;
    class Material;
    
    /**
     * @brief Component responsible for rendering a mesh with a given material.
     *
     * MeshRenderer links together geometry (Mesh), appearance (Material),
     * and spatial data (Transform) to be used by the rendering system.
     */
    class MeshRenderer : public Component {
    public:
        using Base = Component;

        /**
         * @brief Constructs an empty MeshRenderer component.
         */
        MeshRenderer() = default;
        ~MeshRenderer() = default;

        
        #pragma region Disable copy and move

        MeshRenderer(const MeshRenderer& other) = delete;
        MeshRenderer(MeshRenderer&& other) noexcept = delete;
        MeshRenderer& operator=(const MeshRenderer& other) = delete;
        MeshRenderer& operator=(MeshRenderer&& other) noexcept = delete;

        #pragma endregion

        #pragma region Getters

        /**
         * @brief Returns the mesh used for rendering.
         *
         * @return Mesh* Pointer to the mesh.
         */
        [[nodiscard]] Mesh* getMesh() const {
            return _mesh;
        }

        /**
         * @brief Returns the material used for rendering.
         *
         * @return Material* Pointer to the material.
         */
        [[nodiscard]] Material* getMaterial() const {
            return _material;
        }

        /**
         * @brief Returns the transform associated with this renderer.
         *
         * @return Transform* Pointer to the transform.
         */
        [[nodiscard]] Transform* getTransform() const {
            return _transform;
        }

        #pragma endregion

        #pragma region Setters

        /**
         * @brief Sets the mesh used for rendering.
         *
         * @param mesh Pointer to the mesh.
         */
        void setMesh(Mesh* mesh) {
            _mesh = mesh;
            calculateCullingRadius(mesh);
        }

        /**
         * @brief Sets the material used for rendering.
         *
         * @param material Pointer to the material.
         */
        void setMaterial(Material* material) {
            _material = material;
        }

        /**
         * @brief Sets the transform associated with this renderer.
         *
         * @param transform Pointer to the transform.
         */
        void setTransform(Transform* transform) {
            _transform = transform;
        }

        #pragma endregion

        #pragma region Validation

        /**
         * @brief Checks if the renderer has all required data to render.
         *
         * @return true If mesh, material, and transform are set.
         * @return false Otherwise.
         */
        [[nodiscard]] bool isValid() const {
            return _mesh && _material && _transform;
        }

        #pragma endregion

        void calculateCullingRadius(Mesh* mesh);

        float getCullingRadius();
        
    private:
        #pragma region References

        Mesh* _mesh = nullptr;
        Material* _material = nullptr;
        Transform* _transform = nullptr;

        float _cullingRadius = 1.0f;

        #pragma endregion
    };

}// namespace dzemikk
#endif // DZEMIKK_MESH_RENDERER_H