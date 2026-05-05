#include "assetManager/meshBuilder.h"
#include <assimp/scene.h>

dzemikk::MeshBuilder::RawStaticMesh dzemikk::MeshBuilder::buildStaticMeshRaw(const aiMesh* mesh) {

    RawStaticMesh result;

    result.vertices.resize(mesh->mNumVertices);

    const auto* positions = mesh->mVertices;
    const auto* normals = mesh->mNormals;

    for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {

        auto& vertex = result.vertices[v];

        const auto& pos = positions[v];
        vertex.position = {pos.x, pos.y, pos.z};

        if (mesh->HasNormals()) {
            const auto& n = normals[v];
            vertex.normal = {n.x, n.y, n.z};
        } else {
            vertex.normal = glm::vec3(0.0F);
        }
    }

    result.indices.reserve(mesh->mNumFaces * 3);

    for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
        const aiFace& face = mesh->mFaces[f];

        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            result.indices.push_back(face.mIndices[j]);
        }
    }

    return result;
}

dzemikk::MeshBuilder::RawSkinnedMesh dzemikk::MeshBuilder::buildSkinnedMeshRaw(const aiMesh* aiMesh,
                                                                               dzemikk::Skeleton& skeleton) {
    RawSkinnedMesh result;

    result.vertices.resize(aiMesh->mNumVertices);

    const auto* positions = aiMesh->mVertices;
    const auto* normals = aiMesh->mNormals;

    for (unsigned int v = 0; v < aiMesh->mNumVertices; ++v) {

        auto& vertex = result.vertices[v];

        const auto& pos = positions[v];
        vertex.position = {pos.x, pos.y, pos.z};

        if (aiMesh->HasNormals()) {
            const auto& n = normals[v];
            vertex.normal = {n.x, n.y, n.z};
        } else {
            vertex.normal = glm::vec3(0.0F);
        }

        vertex.boneIDs = {0, 0, 0, 0};
        vertex.weights = {0.0F, 0.0F, 0.0F, 0.0F};
    }

    result.indices.reserve(aiMesh->mNumFaces * 3);

    for (unsigned int f = 0; f < aiMesh->mNumFaces; ++f) {
        const aiFace& face = aiMesh->mFaces[f];

        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            result.indices.push_back(face.mIndices[j]);
        }
    }

    extractBoneWeights(aiMesh, result.vertices, skeleton);

    return result;
}

std::shared_ptr<dzemikk::StaticMesh> dzemikk::MeshBuilder::buildStaticMesh(const aiMesh* aiMesh) {

    std::vector<StaticVertex> vertices;
    std::vector<unsigned int> indices;

    vertices.resize(aiMesh->mNumVertices);

    const auto* positions = aiMesh->mVertices;
    const auto* normals = aiMesh->mNormals;

    for (unsigned int v = 0; v < aiMesh->mNumVertices; ++v) {

        auto& vertex = vertices[v];

        const auto& pos = positions[v];
        vertex.position = {pos.x, pos.y, pos.z};

        if (aiMesh->HasNormals()) {
            const auto& n = normals[v];
            vertex.normal = {n.x, n.y, n.z};
        } else {
            vertex.normal = glm::vec3(0.0F);
        }
    }

    indices.reserve(aiMesh->mNumFaces * 3);

    for (unsigned int f = 0; f < aiMesh->mNumFaces; ++f) {
        const aiFace& face = aiMesh->mFaces[f];

        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    auto mesh = std::make_shared<StaticMesh>();
    mesh->create(vertices, indices);
    mesh->uploadToGPU();

    return mesh;
}

std::shared_ptr<dzemikk::SkinnedMesh> dzemikk::MeshBuilder::buildSkinnedMesh(const aiMesh* aiMesh,
                                                                             Skeleton& skeleton) {

    std::vector<SkinnedVertex> vertices;
    std::vector<unsigned int> indices;

    vertices.resize(aiMesh->mNumVertices);

    const auto* positions = aiMesh->mVertices;
    const auto* normals = aiMesh->mNormals;

    for (unsigned int v = 0; v < aiMesh->mNumVertices; ++v) {

        auto& vertex = vertices[v];

        const auto& pos = positions[v];
        vertex.position = {pos.x, pos.y, pos.z};

        if (aiMesh->HasNormals()) {
            const auto& n = normals[v];
            vertex.normal = {n.x, n.y, n.z};
        } else {
            vertex.normal = glm::vec3(0.0F);
        }
    }

    indices.reserve(aiMesh->mNumFaces * 3);

    for (unsigned int f = 0; f < aiMesh->mNumFaces; ++f) {
        const aiFace& face = aiMesh->mFaces[f];

        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    extractBoneWeights(aiMesh, vertices, skeleton);

    auto mesh = std::make_shared<SkinnedMesh>();
    mesh->create(vertices, indices);
    mesh->uploadToGPU();

    return mesh;
}

void dzemikk::MeshBuilder::extractBoneWeights(const aiMesh* mesh,
                                               std::vector<dzemikk::SkinnedVertex>& vertices,
                                               dzemikk::Skeleton& skeleton) {
    for (unsigned int i = 0; i < mesh->mNumBones; i++) {
        aiBone* bone = mesh->mBones[i];

        int boneID = skeleton.getBoneIndex(bone->mName.C_Str());

        if (boneID == -1) {
            continue;
        }

        for (unsigned int w = 0; w < bone->mNumWeights; ++w) {
            const aiVertexWeight& vw = bone->mWeights[w];
            auto& v = vertices[vw.mVertexId];

            for (size_t k = 0; k < v.weights.size(); ++k) {
                if (v.weights[k] == 0.0F) {
                    v.boneIDs[k] = boneID;
                    v.weights[k] = vw.mWeight;
                    break;
                }
            }
        }
    }
}