#include "renderer/model.h"

void dzemikk::Model::draw() const {
    for (const auto& subMesh : _subMeshes) {
        if (subMesh.mesh) {
            subMesh.mesh->draw();
        }
    }
}

void dzemikk::Model::addMesh(const std::shared_ptr<Mesh>& mesh, std::uint32_t materialIndex) {
    if (!mesh) {
        return;
    }

    _subMeshes.emplace_back(mesh, materialIndex);
}

void dzemikk::Model::clear() {
    _subMeshes.clear();
}

const std::vector<dzemikk::Model::SubMesh>& dzemikk::Model::getSubMeshes() const {
    return _subMeshes;
}

std::size_t dzemikk::Model::getMeshCount() const {
    return _subMeshes.size();
}

const dzemikk::Model::SubMesh* dzemikk::Model::getSubMesh(std::size_t index) const {
    if (index >= _subMeshes.size()) {
        return nullptr;
    }
        
    return &_subMeshes[index];
}