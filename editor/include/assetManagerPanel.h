#ifndef EDITOR_ASSET_MANAGER_PANEL_INSPECTOR_H
#define EDITOR_ASSET_MANAGER_PANEL_INSPECTOR_H

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace dzemikk {
class AssetManager;
}

namespace editor {

/**
 * @brief Editor panel responsible for browsing and managing project assets.
 *
 * Displays a hierarchical view of the asset database and provides
 * utilities such as search, selection, and prefab saving.
 */
class AssetManagerPanel {
  public:
#pragma region Drawing

    /**
     * @brief Renders the asset manager UI.
     *
     * @param assetManager Pointer to the engine AssetManager instance.
     */
    void draw(dzemikk::AssetManager* assetManager);

#pragma endregion

  private:
#pragma region Data Structures

    /**
     * @brief Internal tree node representing asset folder hierarchy.
     */
    struct Node {
        std::map<std::string, Node> children; ///< Subdirectories
        std::vector<std::string> files;       ///< Files in this node
    };

#pragma endregion

#pragma region State

    std::string _search;                  ///< Current search query
    std::string _selectedPath = "Assets"; ///< Currently selected directory

#pragma endregion

#pragma region Helpers

    /**
     * @brief Inserts a filesystem path into the asset tree.
     *
     * @param root Root node of the tree.
     * @param path Full asset path to insert.
     */
    static void insert(Node& root, const std::string& path);

    /**
     * @brief Draws a single node of the asset tree.
     *
     * @param node Node to render.
     * @param path Current hierarchical path.
     */
    void drawNode(const Node& node, const std::string& path);

    /**
     * @brief Finds a node by path inside the tree.
     *
     * @param node Root node to search from.
     * @param path Target path.
     * @return Pointer to node if found, otherwise nullptr.
     */
    const Node* findNode(const Node& node, const std::string& path);

    /**
     * @brief Opens a native dialog to save a prefab asset.
     *
     * @return Path of the created prefab.
     */
    std::string openSavePrefabDialog();

#pragma endregion
};

} // namespace editor

#endif