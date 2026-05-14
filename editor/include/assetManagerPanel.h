#pragma once

#include <memory>
#include <string>
#include <map>
#include <vector>

namespace dzemikk {
class AssetManager;
}

namespace editor {
class AssetManagerPanel {
  public:
    void draw(dzemikk::AssetManager* assetManager);

  private:
    struct Node {
        std::map<std::string, Node> children;
        std::vector<std::string> files;
    };

    std::string _search;
    std::string _selectedPath = "Assets";

    static void insert(Node& root, const std::string& path);
    void drawNode(const Node& node, const std::string& path);
    const Node* findNode(const Node& node, const std::string& path);
};

} // namespace editor