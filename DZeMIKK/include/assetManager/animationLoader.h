#ifndef DZEMIKK_ANIMATION_BUILDER_H
#define DZEMIKK_ANIMATION_BUILDER_H

#include <assimp/scene.h>
#include "animation/skeleton.h"

namespace dzemikk {
/**
 * @brief Loads animation data from an Assimp scene into a Skeleton.
 *
 * Responsible for extracting animation channels from Assimp and mapping them
 * onto the internal Skeleton representation. Handles bone name normalization
 * and ensures consistency between mesh bones and animation channels.
 */
class AnimationLoader {
  public:
    /**
     * @brief Loads animation data from the given Assimp scene.
     *
     * Extracts animation tracks and applies them to the provided Skeleton.
     * Assumes that skeleton hierarchy and bone mapping are already constructed.
     *
     * @param scene Pointer to a loaded Assimp scene containing animations.
     * @param skeleton Target skeleton to apply animation data to.
     */
    static void load(const aiScene* scene, Skeleton& skeleton);

    /**
     * @brief Normalizes bone names to ensure consistent mapping.
     *
     * Assimp bone names may contain prefixes or formatting differences
     * depending on exporter (Blender, Maya, etc.). This function standardizes
     * names for reliable skeleton lookup.
     *
     * @param name Raw bone name from Assimp.
     * @return std::string Normalized bone name used internally.
     */
    static std::string normalizeBoneName(const std::string& name);
};

}// namespace dzemikk
#endif // DZEMIKK_ANIMATION_BUILDER_H
