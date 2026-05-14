#include "assetManager/resourceIndex.h"
#include <filesystem>
#include <ranges>

#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif

bool dzemikk::ResourceIndex::initialize() {
    namespace fs = std::filesystem;

    _pathIndex.clear();

    std::optional<fs::path> root;

    fs::path start = fs::current_path();

    if (!fs::exists(start / "game")) {

        fs::path parent = start.parent_path();

        if (fs::exists(parent / "game") && fs::is_directory(parent / "game")) {

            start = parent / "game";
        }
    }

#if DZEMIKK_DEV_TOOLS
    spdlog::info("[ResourceIndex] Search start: {}", start.string());
#endif

    for (const auto& entry : fs::recursive_directory_iterator(start)) {
        if (entry.is_directory() && entry.path().filename() == "res") {
            root = fs::absolute(entry.path());
            break;
        }
    }

    if (!root) {
#if DZEMIKK_DEV_TOOLS
        spdlog::error("[ResourceIndex] Cannot find 'res' folder!");
#endif
        return false;
    }

    _rootPath = root->string();
    std::ranges::replace(_rootPath, '\\', '/');

#if DZEMIKK_DEV_TOOLS
    spdlog::info("[ResourceIndex] Root: {}", _rootPath);
#endif

    for (const auto& entry : fs::recursive_directory_iterator(_rootPath)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        std::string fullPath = entry.path().string();
        std::ranges::replace(fullPath, '\\', '/');

        std::string relative = fs::relative(entry.path(), _rootPath).string();
        std::ranges::replace(relative, '\\', '/');

        _pathIndex[relative] = fullPath;
    }

    return true;
}

std::string dzemikk::ResourceIndex::resolve(const std::string& path) const {
    auto it = _pathIndex.find(path);
    if (it != _pathIndex.end()) {
        return it->second;
    }

    return (std::filesystem::path(_rootPath) / path).generic_string();
}

void dzemikk::ResourceIndex::clear() {
    _pathIndex.clear();
}

std::string dzemikk::ResourceIndex::getRoot() {
    return _rootPath;
}

std::vector<std::string> dzemikk::ResourceIndex::getAllPaths() const {
    std::vector<std::string> out;
    out.reserve(_pathIndex.size());

    for (const auto& [rel, full] : _pathIndex) {
        out.push_back(rel);
    }

    return out;
}
