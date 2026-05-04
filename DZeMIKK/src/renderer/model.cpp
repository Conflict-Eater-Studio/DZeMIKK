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

void dzemikk::Model::setSkeleton(std::shared_ptr<Skeleton> skeleton) {
    _skeleton = skeleton;
}

std::shared_ptr<dzemikk::Skeleton> dzemikk::Model::getSkeleton() const {
    return _skeleton;
}

void dzemikk::Model::addPending(MeshBuilder::RawStaticMesh& mesh) {
    _pendingMeshes.push_back(PendingMesh{mesh});
}

void dzemikk::Model::addPending(MeshBuilder::RawSkinnedMesh& mesh) {
    _pendingMeshes.push_back(PendingMesh{mesh});
}

void dzemikk::Model::uploadToGPU() {
    if (_gpuReady)
        return;

    for (auto& pending : _pendingMeshes) {
        if (std::holds_alternative<MeshBuilder::RawStaticMesh>(pending.data)) {
            auto& raw = std::get<MeshBuilder::RawStaticMesh>(pending.data);

            auto mesh = std::make_shared<StaticMesh>();
            mesh->create(raw.vertices, raw.indices);

            addMesh(mesh, raw.materialIndex);
        } else {
            auto& raw = std::get<MeshBuilder::RawSkinnedMesh>(pending.data);

            auto mesh = std::make_shared<SkinnedMesh>();
            mesh->create(raw.vertices, raw.indices);

            addMesh(mesh, raw.materialIndex);
        }
    }

    _pendingMeshes.clear();
    _gpuReady = true;
}