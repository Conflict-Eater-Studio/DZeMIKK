#include "assetManagerPanel.h"

#include "assetManager/assetmanager.h"

#include <cstring>
#include <imgui.h>
#include <fstream>

#include <ecs/gameobject.h>
#include <ecs/serialize/prefabSerializer.h>


static std::string normalizeAssetPath(std::string path) {
    constexpr std::string_view prefix = "Assets/";
    if (path.starts_with(prefix)) {
        path = path.substr(prefix.size());
    }

    auto removeSuffix = [&](std::string_view suffix) {
        if (path.size() >= suffix.size() &&
            path.compare(path.size() - suffix.size(), suffix.size(), suffix) == 0) {
            path.erase(path.size() - suffix.size());
        }
    };

    removeSuffix(".vert");
    removeSuffix(".frag");

    return path;
}

static const char* getAssetDragType(const std::string& path) {
    if (path.ends_with(".ttf") || path.ends_with(".otf"))
        return "ASSET_FONT";

    if (path.ends_with(".png") || path.ends_with(".jpg") || path.ends_with(".jpng"))
        return "ASSET_TEXTURE";

    if (path.ends_with(".obj") || path.ends_with(".fbx"))
        return "ASSET_MODEL";

    if (path.ends_with(".vert") || path.ends_with(".frag") || path.ends_with(".glsl"))
        return "ASSET_SHADER";

    if (path.ends_with(".wav"))
        return "ASSET_AUDIO";

    return "ASSET_PATH";
}

void editor::AssetManagerPanel::draw(dzemikk::AssetManager* assetManager) {
    ImGui::Begin("Asset Manager");

    char buffer[256];
    std::memset(buffer, 0, sizeof(buffer));
    std::strncpy(buffer, _search.c_str(), sizeof(buffer) - 1);

    if (ImGui::InputText("Search", buffer, sizeof(buffer))) {
        _search = buffer;
    }

    ImGui::Separator();

    if (!assetManager) {
        ImGui::Text("No AssetManager");
        ImGui::End();
        return;
    }

    auto assets = assetManager->getAllAssets();

    std::vector<std::string> filtered;
    filtered.reserve(assets.size());

    for (const auto& a : assets) {
        if (_search.empty() || a.find(_search) != std::string::npos) {
            filtered.push_back(a);
        }
    }

    Node root;
    for (const auto& a : filtered) {
        insert(root, a);
    }

    ImGui::Columns(2, "AssetColumns");

    ImGui::BeginChild("FolderPanel", ImVec2(0, 0), true);

    if (ImGui::TreeNode("Assets")) {
        drawNode(root, "Assets");
        ImGui::TreePop();
    }

    ImGui::EndChild();

    if (ImGui::BeginDragDropTarget()) {

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_GAMEOBJECT")) {

            auto* gameObject = *static_cast<dzemikk::GameObject**>(payload->Data);

            if (gameObject) {

                auto json = dzemikk::PrefabSerializer::serialize(*gameObject);

                std::filesystem::create_directories("Assets/Prefabs");

                std::string path = "Assets/Prefabs/" + gameObject->getName() + ".prefab";

                std::ofstream file(path);

                if (file.is_open()) {
                    file << json.dump(4);
                    file.close();
                }
            }
        }

        ImGui::EndDragDropTarget();
    }

    ImGui::NextColumn();

    ImGui::BeginChild("FilePanel", ImVec2(0, 0), true);

    const Node* currentFolder = findNode(root, _selectedPath);

    if (currentFolder) {

        const float cellSize = 80.0f;
        const float iconSize = 48.0f;
        int columnCount = std::max(1, (int)(ImGui::GetContentRegionAvail().x / cellSize));

        ImGui::Columns(columnCount, nullptr, false);

        for (const auto& file : currentFolder->files) {
            if (!_search.empty() && file.find(_search) == std::string::npos)
                continue;

            ImGui::PushID(file.c_str());

            ImGui::BeginGroup();

            ImGui::Button("[FILE]", ImVec2(iconSize, iconSize));

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {

                std::string fullPath;

                if (_selectedPath.empty()) {
                    fullPath = file;
                } else {
                    fullPath = _selectedPath + "/" + file;
                }

                const char* type = getAssetDragType(fullPath);
                
                fullPath = normalizeAssetPath(fullPath);

                ImGui::SetDragDropPayload(type, fullPath.c_str(), fullPath.size() + 1);

                ImGui::TextUnformatted(fullPath.c_str());

                ImGui::EndDragDropSource();
            }

            float textWidth = ImGui::CalcTextSize(file.c_str()).x;
            float offset = (iconSize - textWidth) * 0.5f;
            if (offset > 0.0f)
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

            ImGui::TextWrapped("%s", file.c_str());

            ImGui::EndGroup();

            ImGui::NextColumn();
            ImGui::PopID();
        }

        ImGui::Columns(1);
    }

    ImGui::EndChild();

    ImGui::Columns(1);

    ImGui::End();
}

void editor::AssetManagerPanel::insert(Node& root, const std::string& path) {
    Node* current = &root;

    size_t start = 0;

    while (true) {
        size_t slash = path.find('/', start);

        if (slash == std::string::npos) {
            current->files.push_back(path.substr(start));
            break;
        }

        std::string folder = path.substr(start, slash - start);
        current = &current->children[folder];

        start = slash + 1;
    }
}

void editor::AssetManagerPanel::drawNode(const Node& node, const std::string& path) {
    for (const auto& [folder, child] : node.children) {

        std::string fullPath = path + "/" + folder;

        if (ImGui::TreeNode(folder.c_str())) {

            if (ImGui::IsItemClicked()) {
                _selectedPath = fullPath;
            }

            drawNode(child, fullPath);

            ImGui::TreePop();
        }
    }
}

const editor::AssetManagerPanel::Node*
editor::AssetManagerPanel::findNode(const editor::AssetManagerPanel::Node& node,
                                    const std::string& path) {
    if (path == "Assets") {
        return &node;
    }

    const editor::AssetManagerPanel::Node* current = &node;

    size_t start = 7;

    while (true) {
        if (start >= path.size()) {
            return current;
        }

        if (path[start] == '/') {
            start++;
            continue;
        }

        size_t slash = path.find('/', start);

        std::string folder =
            path.substr(start, (slash == std::string::npos) ? std::string::npos : slash - start);

        auto it = current->children.find(folder);
        if (it == current->children.end()) {
            return nullptr;
        }

        current = &it->second;

        if (slash == std::string::npos) {
            break;
        }

        start = slash + 1;
    }

    return current;
}