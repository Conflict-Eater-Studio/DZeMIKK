#include "assetManager/modelHandler.h"
#include "renderer/StaticMesh.h"
#include "renderer/model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

void printNode(aiNode* node, int depth = 0) {
    for (int i = 0; i < depth; i++) {
        std::cout << "  ";
    }

    std::cout << node->mName.C_Str() << " (Meshes: " << node->mNumMeshes << ")\n";

    aiNode** child = node->mChildren;

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        printNode(*(child + i), depth + 1);
    }
}

void printSkeleton(const dzemikk::Skeleton& skeleton, int boneIndex = 0, int depth = 0) {
    const dzemikk::Bone* bone = skeleton.getBone(boneIndex);
    if (!bone) {
        return;
    }

    for (int i = 0; i < depth; i++) {
        std::cout << "  ";
    }

    std::cout << bone->getName() << " (ID: " << boneIndex << ")\n";

    for (int childIndex : bone->getChildren()) {
        printSkeleton(skeleton, childIndex, depth + 1);
    }
}

dzemikk::ModelHandler::Result dzemikk::ModelHandler::load(const std::string& path) {
    auto model = loadModelFromFile(path);

    if (!model) {
        std::cerr << "Failed to load mesh: " << path << "\n";
        return {nullptr, AssetError::LoadFailed};
    }

    return {model, AssetError::None};
}

bool dzemikk::ModelHandler::reload(Handle& asset, const std::string& path) {
    if (!asset) {
        return false;
    }

    auto newModel = loadModelFromFile(path);
    if (!newModel) {
        return false;
    }

    auto* model = asset.get();

    model->clear();

    for (const auto& subMesh : newModel->getSubMeshes()) {
        model->addMesh(subMesh.mesh, subMesh.materialIndex);
    }

    return true;
}

void dzemikk::ModelHandler::unload(Handle& asset) {
    asset = Handle{};
}

std::shared_ptr<dzemikk::Model> dzemikk::ModelHandler::loadModelFromFile(const std::string& path) {

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals |
                                                       aiProcess_JoinIdenticalVertices);

    if (!scene || !scene->HasMeshes()) {
        std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << "\n";
        return nullptr;
    }

    auto model = std::make_shared<Model>();

    auto skeleton = std::make_shared<dzemikk::Skeleton>();
    buildSkeleton(scene->mRootNode, *skeleton, -1, glm::mat4(1.0F));

    skeleton->setGlobalInverseTransform(glm::inverse(aiToGlm(scene->mRootNode->mTransformation)));

    model->setSkeleton(skeleton);

    //printSkeleton(*skeleton);

    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {

        const aiMesh* mesh = scene->mMeshes[i];
        const bool isSkinned = mesh->HasBones();

        if (!isSkinned) {
            auto staticMesh = buildStaticMesh(mesh);
            model->addMesh(staticMesh, mesh->mMaterialIndex);
        } else {
            auto skinnedMesh = buildSkinnedMesh(mesh, *skeleton);
            model->addMesh(skinnedMesh, mesh->mMaterialIndex);
        }
    }

    return model;
}

glm::mat4 dzemikk::ModelHandler::aiToGlm(const aiMatrix4x4& m) {
    glm::mat4 result;
    result[0][0] = m.a1;
    result[1][0] = m.a2;
    result[2][0] = m.a3;
    result[3][0] = m.a4;
    result[0][1] = m.b1;
    result[1][1] = m.b2;
    result[2][1] = m.b3;
    result[3][1] = m.b4;
    result[0][2] = m.c1;
    result[1][2] = m.c2;
    result[2][2] = m.c3;
    result[3][2] = m.c4;
    result[0][3] = m.d1;
    result[1][3] = m.d2;
    result[2][3] = m.d3;
    result[3][3] = m.d4;
    return result;
}

bool isAssimpHelperNode(const std::string& name) {
    return name.find("_$AssimpFbx$") != std::string::npos;
}

void dzemikk::ModelHandler::buildSkeleton(aiNode* node, Skeleton& skeleton, int parent,
                                 glm::mat4 accumulatedTransform) {
    std::string name = node->mName.C_Str();

    glm::mat4 local = aiToGlm(node->mTransformation);
    glm::mat4 newAccum = accumulatedTransform * local;

    if (isAssimpHelperNode(name)) {
        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            aiNode* child = node->mChildren[i];
            buildSkeleton(child, skeleton, parent, newAccum);
        }
    } else {
        int index = skeleton.addBone(name, parent);

        auto* bone = skeleton.getBone(index);
        if (bone) {
            bone->setLocalTransform(newAccum);
            bone->setBindLocalTransform(newAccum);
        }

        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            aiNode* child = node->mChildren[i];
            buildSkeleton(child, skeleton, index, glm::mat4(1.0F));
        }
    }
}

void dzemikk::ModelHandler::extractBoneWeights(const aiMesh* mesh, std::vector<dzemikk::SkinnedVertex>& vertices,
                               dzemikk::Skeleton& skeleton) {
    for (unsigned int i = 0; i < mesh->mNumBones; i++) {
        aiBone* bone = mesh->mBones[i];

        int boneID = skeleton.getBoneIndex(bone->mName.C_Str());

        //std::cout << "Bone: " << i << " name: " << bone->mName.C_Str() << "\n";

        if (boneID == -1) {
            continue;
        }
        
        auto* skelBone = skeleton.getBone(boneID);
        if (skelBone) {
            skelBone->setOffsetMatrix(aiToGlm(bone->mOffsetMatrix));
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

    //std::cout << "\n\n";
}

std::shared_ptr<dzemikk::StaticMesh> dzemikk::ModelHandler::buildStaticMesh(const aiMesh* aiMesh) {

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

    return mesh;
}

std::shared_ptr<dzemikk::SkinnedMesh> dzemikk::ModelHandler::buildSkinnedMesh(const aiMesh* aiMesh,
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

    return mesh;
}