#include "assetManagerPanel.h"

#include "assetManager/assetmanager.h"

#include <cstring>
#include <imgui.h>
#include <fstream>
#include <windows.h>
#include <commdlg.h>

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
    if (path.ends_with(".ttf") || path.ends_with(".otf")) {
        return "ASSET_FONT";
    }

    if (path.ends_with(".png") || path.ends_with(".jpg") || path.ends_with(".jpng")) {
        return "ASSET_TEXTURE";
    }

    if (path.ends_with(".obj") || path.ends_with(".fbx")) {
        return "ASSET_MODEL";
    }

    if (path.ends_with(".vert") || path.ends_with(".frag") || path.ends_with(".glsl")) {
        return "ASSET_SHADER";
    }

    if (path.ends_with(".wav")) {
        return "ASSET_AUDIO";
    }

    if (path.ends_with(".prefab")) {
        return "ASSET_PREFAB";
    }

    return "ASSET_PATH";
}

void editor::AssetManagerPanel::draw(dzemikk::AssetManager* assetManager) {
    ImGui::Begin("Asset Manager");

    drawSearchBar();
    ImGui::Separator();

    if (!assetManager) {
        ImGui::TextUnformatted("No AssetManager");
        ImGui::End();
        return;
    }

    auto assets = assetManager->getAllAssets();
    auto root = buildAssetTree(assets);

    ImGui::Columns(2, "AssetColumns");

    drawFolderPanel(root);
    drawFilePanel(root);

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

        handlePrefabDrop();
        std::string fullPath = path;
        fullPath += "/";
        fullPath += folder;

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

std::string editor::AssetManagerPanel::openSavePrefabDialog() {
    std::array<char, MAX_PATH> fileName{};

    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = fileName.data();
    ofn.nMaxFile = MAX_PATH;

    ofn.lpstrFilter = "Prefab Files (*.prefab)\0*.prefab\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "prefab";

    if (GetSaveFileNameA(&ofn)) {
        return fileName.data();
    }

    return {};
}

void editor::AssetManagerPanel::drawSearchBar() {
    std::array<char, 256> buffer{};

    std::strncpy(buffer.data(), _search.c_str(), buffer.size() - 1);

    if (ImGui::InputText("Search", buffer.data(), buffer.size())) {
        _search = buffer.data();
    }
}

editor::AssetManagerPanel::Node
editor::AssetManagerPanel::buildAssetTree(const std::vector<std::string>& assets) {
    Node root;

    for (const auto& a : assets) {
        if (_search.empty() || a.find(_search) != std::string::npos) {
            insert(root, a);
        }
    }

    return root;
}

void editor::AssetManagerPanel::drawFolderPanel(const Node& root) {
    ImGui::BeginChild("FolderPanel", ImVec2(0, 0), true);

    if (ImGui::TreeNode("Assets")) {
        drawNode(root, "Assets");
        ImGui::TreePop();
    }

    ImGui::EndChild();
}

void editor::AssetManagerPanel::drawFilePanel(const Node& root) {
    ImGui::NextColumn();
    ImGui::BeginChild("FilePanel", ImVec2(0, 0), true);

    const Node* current = findNode(root, _selectedPath);

    if (current) {
        drawFileGrid(*current);
    }

    ImGui::EndChild();
}

void editor::AssetManagerPanel::drawFileGrid(const Node& node) {
    const float cellSize = 80.0F;
    const float iconSize = 48.0F;

    int columnCount = std::max(1, (int)(ImGui::GetContentRegionAvail().x / cellSize));

    ImGui::Columns(columnCount, nullptr, false);

    for (const auto& file : node.files) {
        const bool searchActive = !_search.empty();
        const bool notFound = file.find(_search) == std::string::npos;

        if (searchActive && notFound) {
            continue;
        }

        drawFileItem(file, iconSize);
    }

    ImGui::Columns(1);
}

void editor::AssetManagerPanel::drawFileItem(const std::string& file, float iconSize) {
    ImGui::PushID(file.c_str());
    ImGui::BeginGroup();

    ImGui::Button("[FILE]", ImVec2(iconSize, iconSize));

    handleFileDrag(file);

    float textWidth = ImGui::CalcTextSize(file.c_str()).x;
    float offset = (iconSize - textWidth) * 0.5F;

    if (offset > 0.0F) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
    }

    ImGui::TextUnformatted(file.c_str());

    ImGui::EndGroup();
    ImGui::NextColumn();
    ImGui::PopID();
}

void editor::AssetManagerPanel::handleFileDrag(const std::string& file) {
    if (!ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        return;
    }

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

void editor::AssetManagerPanel::handlePrefabDrop() {
    if (!ImGui::BeginDragDropTarget()) {
        return;
    }

    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_GAMEOBJECT")) {

        auto* go = *static_cast<dzemikk::GameObject**>(payload->Data);

        if (go) {
            auto json = dzemikk::PrefabSerializer::serialize(*go);
            std::string path = openSavePrefabDialog();

            if (!path.empty()) {
                std::ofstream file(path);
                if (file.is_open()) {
                    file << json.dump(4);
                }
            }
        }
    }

    ImGui::EndDragDropTarget();
}