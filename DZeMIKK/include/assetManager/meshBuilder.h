#ifndef DZEMIKK_MESH_BUILDER_H
#define DZEMIKK_MESH_BUILDER_H

#include <memory>
#include <vector>
#include <assimp/SceneCombiner.h>

#include "renderer/staticMesh.h"
#include "renderer/skinnedMesh.h"
#include "animation/skeleton.h"

namespace dzemikk {
class MeshBuilder {
  public:

    struct RawStaticMesh {
        std::vector<StaticVertex> vertices;
        std::vector<unsigned int> indices;
        int materialIndex;
    };

    struct RawSkinnedMesh {
        std::vector<SkinnedVertex> vertices;
        std::vector<unsigned int> indices;
        int materialIndex;
    };

    /**
     * @brief Builds a static (non-animated) mesh from Assimp mesh data.
     *
     * Converts vertex positions, normals, and index data from an aiMesh into a
     * GPU-ready StaticMesh. This function is used for geometry that does not
     * participate in skeletal animation.
     *
     * @param mesh Input Assimp mesh containing static geometry data.
     * @return std::shared_ptr<StaticMesh> Fully constructed and GPU-uploaded static mesh.
     */
    static RawStaticMesh buildStaticMeshRaw(const aiMesh* mesh);

    /**
     * @brief Builds a skinned (skeletally animated) mesh from Assimp mesh data.
     *
     * Converts vertex attributes (position, normal, bone IDs, weights) and index
     * data into a GPU-ready SkinnedMesh. Also binds vertex bone influences to the
     * provided skeleton structure.
     *
     * @param mesh Input Assimp mesh containing bone and skinning data.
     * @param skeleton Skeleton used to resolve bone IDs and bind pose transforms.
     * @return std::shared_ptr<SkinnedMesh> Fully constructed skinned mesh ready for animation.
     */
    static RawSkinnedMesh buildSkinnedMeshRaw(const aiMesh* aiMesh, Skeleton& skeleton);

    /**
     * @brief Builds a static (non-animated) mesh from Assimp mesh data.
     *
     * Converts vertex positions, normals, and index data from an aiMesh into a
     * GPU-ready StaticMesh. This function is used for geometry that does not
     * participate in skeletal animation.
     *
     * @param mesh Input Assimp mesh containing static geometry data.
     * @return std::shared_ptr<StaticMesh> Fully constructed and GPU-uploaded static mesh.
     */
    static std::shared_ptr<StaticMesh> buildStaticMesh(const aiMesh* mesh);

    /**
     * @brief Builds a skinned (skeletally animated) mesh from Assimp mesh data.
     *
     * Converts vertex attributes (position, normal, bone IDs, weights) and index
     * data into a GPU-ready SkinnedMesh. Also binds vertex bone influences to the
     * provided skeleton structure.
     *
     * @param mesh Input Assimp mesh containing bone and skinning data.
     * @param skeleton Skeleton used to resolve bone IDs and bind pose transforms.
     * @return std::shared_ptr<SkinnedMesh> Fully constructed skinned mesh ready for animation.
     */
    static std::shared_ptr<SkinnedMesh> buildSkinnedMesh(const aiMesh* mesh, Skeleton& skeleton);

    /**
     * @brief Extracts bone weights from an Assimp mesh and applies them to vertices.
     *
     * Parses bone influence data from the mesh and assigns up to 4 bone IDs and
     * weights per vertex. Also updates skeleton mapping with bone indices.
     *
     * @param mesh Input Assimp mesh containing bone data.
     * @param vertices Output vertex array to be filled with skinning data.
     * @param skeleton Skeleton structure used to map bone names to indices.
     */
    static void extractBoneWeights(const aiMesh* mesh, std::vector<SkinnedVertex>& vertices,
                                   dzemikk::Skeleton& skeleton);
};
} // namespace dzemikk
#endif // DZEMIKK_MESH_BUILDER_H