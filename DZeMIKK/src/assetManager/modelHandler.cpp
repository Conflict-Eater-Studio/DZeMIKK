#include "assetManager/modelHandler.h"
#include "renderer/StaticMesh.h"
#include "renderer/model.h"

#include "animation/boneTrack.h"
#include "animation/animationclip.h"

#include <assimp/Importer.hpp>

#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <glm/gtx/matrix_decompose.hpp>

#include "assetManager/skeletonBuilder.h"

#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif

bool dzemikk::ModelHandler::isAssimpHelperNode(const std::string& name) {
    return name.find("_$AssimpFbx$") != std::string::npos;
}

bool dzemikk::ModelHandler::isBoneNode(const std::string& name, const dzemikk::Skeleton& skeleton) {
    return skeleton.getBoneIndex(name) != -1;
}

void dzemikk::ModelHandler::printNodeHierarchyForMesh(aiNode* node, const aiScene* scene,
                               const dzemikk::Skeleton& skeleton, int depth) {
#if DZEMIKK_DEV_TOOLS
    if (!node)
        return;

    std::string indent(depth * 2, ' ');
    std::string name = node->mName.C_Str();

    bool isHelper = isAssimpHelperNode(name);
    bool isBone = isBoneNode(name, skeleton);
    bool hasMeshes = node->mNumMeshes > 0;

    std::string line = indent + name;

    if (isHelper)
        line += " [ASSIMP_HELPER]";
    if (isBone)
        line += " [BONE]";
    if (hasMeshes)
        line += " [MESHES: " + std::to_string(node->mNumMeshes) + "]";

    spdlog::info("[ModelHandler] {}", line);

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        printNodeHierarchyForMesh(node->mChildren[i], scene, skeleton, depth + 1);
    }
#endif
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

    auto newModel = loadModelFromFile(path, LoadMode::MeshOnly);
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

std::shared_ptr<dzemikk::Model> dzemikk::ModelHandler::loadModelFromFile(const std::string& path, LoadMode loadMode) {

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals |
                                                       aiProcess_JoinIdenticalVertices);

    if (!scene || !scene->HasMeshes()) {
        std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << "\n";
        return nullptr;
    }

    const bool hasAnimations = scene->mNumAnimations > 0;

    auto model = std::make_shared<Model>();

    auto skeleton = SkeletonBuilder::build(scene);
    model->setSkeleton(std::shared_ptr<Skeleton>(skeleton));


    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
    #if DZEMIKK_DEV_TOOLS
        spdlog::info("[ModelHandler] MESH {}: {}", i, scene->mMeshes[i]->mName.C_Str());
        printNodeHierarchyForMesh(scene->mRootNode, scene, *skeleton);
    #endif
    }

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

    if (scene->mNumAnimations > 0 && loadMode == LoadMode::All) {
        loadAnimations(scene, *skeleton);
    }

    return model;
}

std::string dzemikk::ModelHandler::normalizeBoneName(const std::string& name) {
    const std::string tag1 = "_$AssimpFbx$";
    const std::string tag2 = "$AssimpFbx$";

    size_t pos = name.find(tag1);
    if (pos != std::string::npos) {
        return name.substr(0, pos);
    }

    pos = name.find(tag2);
    if (pos != std::string::npos) {
        return name.substr(0, pos);
    }

    return name;
}

void dzemikk::ModelHandler::loadAnimations(const aiScene* scene, Skeleton& skeleton) {
    std::unordered_map<std::string, int> normalizedToBone;

    for (int i = 0; i < skeleton.getBoneCount(); ++i) {
        Bone* b = skeleton.getBone(i);
        if (!b)
            continue;

        std::string norm = normalizeBoneName(b->getName());
        normalizedToBone[norm] = i;
    }

    for (unsigned int a = 0; a < scene->mNumAnimations; ++a) {

        const aiAnimation* anim = scene->mAnimations[a];

        float duration = static_cast<float>(anim->mDuration);
        float tps = anim->mTicksPerSecond != 0 ? anim->mTicksPerSecond : 25.0f;

        auto* clip = new AnimationClip(duration, tps);

        struct ChannelBundle {
            const aiNodeAnim* translation = nullptr;
            const aiNodeAnim* rotation = nullptr;
            const aiNodeAnim* scaling = nullptr;
        };

        std::unordered_map<std::string, ChannelBundle> channelMap;

        for (unsigned int j = 0; j < anim->mNumChannels; ++j) {
            const aiNodeAnim* ch = anim->mChannels[j];

            std::string rawName = ch->mNodeName.C_Str();
            std::string norm = normalizeBoneName(rawName);

            if (rawName.find("Translation") != std::string::npos) {
                channelMap[norm].translation = ch;
            } else if (rawName.find("Rotation") != std::string::npos) {
                channelMap[norm].rotation = ch;
            } else if (rawName.find("Scaling") != std::string::npos) {
                channelMap[norm].scaling = ch;
            } else {
                channelMap[norm].translation = ch;
                channelMap[norm].rotation = ch;
                channelMap[norm].scaling = ch;
            }
        }

        for (const auto& [normName, bundle] : channelMap) {

            auto it = normalizedToBone.find(normName);
            if (it == normalizedToBone.end())
                continue;

            int boneIndex = it->second;
            Bone* bone = skeleton.getBone(boneIndex);
            if (!bone)
                continue;

            BoneTrack* track = clip->addBoneTrack();
            track->bindBone(&skeleton, boneIndex);

            if (bundle.translation) {
                for (unsigned int i = 0; i < bundle.translation->mNumPositionKeys; ++i) {
                    const auto& k = bundle.translation->mPositionKeys[i];
                    track->addPositionKey({(float)k.mTime, {k.mValue.x, k.mValue.y, k.mValue.z}});
                }
            }

            if (bundle.rotation) {
                for (unsigned int i = 0; i < bundle.rotation->mNumRotationKeys; ++i) {
                    const auto& k = bundle.rotation->mRotationKeys[i];
                    track->addRotationKey({(float)k.mTime, glm::quat(k.mValue.w, k.mValue.x,
                                                                     k.mValue.y, k.mValue.z)});
                }
            }

            if (bundle.scaling) {
                for (unsigned int i = 0; i < bundle.scaling->mNumScalingKeys; ++i) {
                    const auto& k = bundle.scaling->mScalingKeys[i];
                    track->addScaleKey({(float)k.mTime, {k.mValue.x, k.mValue.y, k.mValue.z}});
                }
            }
        }

        std::string name =
            anim->mName.length > 0 ? anim->mName.C_Str() : "Anim_" + std::to_string(a);

        skeleton.addClip(name, clip);
    }
}

void dzemikk::ModelHandler::extractBoneWeights(const aiMesh* mesh, std::vector<dzemikk::SkinnedVertex>& vertices,
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