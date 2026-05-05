#ifndef DZEMIKK_RESOURCE_INDEX_H
#define DZEMIKK_RESOURCE_INDEX_H

#include <string>
#include <unordered_map>

namespace dzemikk {
/**
 * @brief Resolves virtual asset paths to real filesystem paths.
 *
 * Acts as a central mapping layer between engine-level resource paths
 * and actual disk locations. Enables abstraction of file structure
 * from engine systems.
 */
class ResourceIndex {
  public:

    /**
     * @brief Initializes the resource index (loads mappings, sets root path).
     *
     * @return true if initialization succeeded, false otherwise.
     */
    bool initialize();

    /**
     * @brief Resolves a virtual path to a physical filesystem path.
     *
     * @param path Virtual engine path (e.g. "textures/wood.png").
     * @return std::string Resolved filesystem path
     */
    [[nodiscard]] std::string resolve(const std::string& path) const;

    /**
     * @brief Clears all registered path mappings.
     */
    void clear();

    /**
     * @brief Returns root resource directory.
     *
     * @return Root path used as base for resolving resources.
     */
    [[nodiscard]] std::string getRoot();

  private:
    /**
     * @brief Mapping of virtual paths to real filesystem paths.
     */
    std::unordered_map<std::string, std::string> _pathIndex;
    
    /**
     * @brief Root directory for all engine resources.
     */
    std::string _rootPath;
};
}// namespace dzemikk

#endif //DZEMIKK_RESOURCE_INDEX_H
