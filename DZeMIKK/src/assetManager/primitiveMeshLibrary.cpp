#include "assetManager/primitiveMeshLibrary.h"
#include "assetManager/primitiveFactory.h"
#include "renderer/mesh.h"
#include <iostream>

dzemikk::PrimitiveMeshLibrary::PrimitiveMeshLibrary() {
    init();
}

void dzemikk::PrimitiveMeshLibrary::init() {
    _meshes[PrimitiveMesh::Cube] = PrimitiveFactory::createCube();
    _meshes[PrimitiveMesh::Quad] = PrimitiveFactory::createQuad();
    _meshes[PrimitiveMesh::Sphere] = PrimitiveFactory::createSphere();
    _meshes[PrimitiveMesh::Capsule] = PrimitiveFactory::createCapsule();
}

dzemikk::Mesh* dzemikk::PrimitiveMeshLibrary::get(PrimitiveMesh type) {
    auto it = _meshes.find(type);
    if (it != _meshes.end()) {
        return it->second.get();
    }

    return nullptr;
}

void dzemikk::PrimitiveMeshLibrary::clear() {
    _meshes.clear();
}
