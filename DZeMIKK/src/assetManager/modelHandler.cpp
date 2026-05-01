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

bool isAssimpHelperNode(const std::string& name) {
    return name.find("_$AssimpFbx$") != std::string::npos;
}

bool isBoneNode(const std::string& name, const dzemikk::Skeleton& skeleton) {
    return skeleton.getBoneIndex(name) != -1;
}

void printNodeHierarchyForMesh(aiNode* node, const aiScene* scene,
                               const dzemikk::Skeleton& skeleton, int depth = 0) {
    for (int i = 0; i < depth; i++)
        std::cout << "  ";

    std::string name = node->mName.C_Str();

    bool isHelper = isAssimpHelperNode(name);
    bool isBone = isBoneNode(name, skeleton);
    bool hasMeshes = node->mNumMeshes > 0;

    std::cout << name;

    if (isHelper)
        std::cout << " [ASSIMP_HELPER]";
    if (isBone)
        std::cout << " [BONE]";
    if (hasMeshes)
        std::cout << " [MESHES: " << node->mNumMeshes << "]";

    std::cout << "\n";

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        printNodeHierarchyForMesh(node->mChildren[i], scene, skeleton, depth + 1);
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

    const bool hasAnimations = scene->mNumAnimations > 0;

    std::cout << "Model: " << path << "\n";
    std::cout << "Meshes: " << scene->mNumMeshes << "\n";
    std::cout << "Has bones: " << (scene->HasMeshes() ? "YES" : "NO") << "\n";
    std::cout << "Has animations: " << (hasAnimations ? "YES" : "NO") << "\n";
    std::cout << "Animations count: " << scene->mNumAnimations << "\n";

    for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {
        const aiAnimation* anim = scene->mAnimations[i];

        std::cout << "\n=== Animation " << i << " ===\n";
        std::cout << "Name: " << (anim->mName.length > 0 ? anim->mName.C_Str() : "Unnamed") << "\n";
        std::cout << "Duration: " << anim->mDuration << "\n";
        std::cout << "Ticks per second: "
                  << (anim->mTicksPerSecond != 0 ? anim->mTicksPerSecond : 25.0) << "\n";

        std::cout << "Channels (animated nodes): " << anim->mNumChannels << "\n";

        for (unsigned int j = 0; j < anim->mNumChannels; ++j) {
            const aiNodeAnim* channel = anim->mChannels[j];

            std::cout << "  Node: " << channel->mNodeName.C_Str() << "\n";
            std::cout << "    Position keys: " << channel->mNumPositionKeys << "\n";
            std::cout << "    Rotation keys: " << channel->mNumRotationKeys << "\n";
            std::cout << "    Scaling keys: " << channel->mNumScalingKeys << "\n";
        }
    }

    auto model = std::make_shared<Model>();

    auto skeleton = std::make_shared<dzemikk::Skeleton>();
    buildSkeleton(scene->mRootNode, *skeleton, -1, glm::mat4(1.0F));

    skeleton->setGlobalInverseTransform(glm::inverse(aiToGlm(scene->mRootNode->mTransformation)));

    model->setSkeleton(skeleton);

    //printSkeleton(*skeleton);

    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        std::cout << "\n========================\n";
        std::cout << "MESH " << i << ": " << scene->mMeshes[i]->mName.C_Str() << "\n";
        std::cout << "========================\n";

        printNodeHierarchyForMesh(scene->mRootNode, scene, *skeleton);
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

    if (scene->mNumAnimations > 0) {
        loadAnimations(scene, *skeleton);
    }

    return model;
}

auto normalizeBoneName = [](const std::string& name) -> std::string {
    size_t pos = name.find("_$AssimpFbx$");
    if (pos != std::string::npos) {
        return name.substr(0, pos);
    }

    pos = name.find("$AssimpFbx$");
    if (pos != std::string::npos) {
        return name.substr(0, pos);
    }

    return name;
};

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

        std::unordered_map<std::string, const aiNodeAnim*> channelMap;

        for (unsigned int j = 0; j < anim->mNumChannels; ++j) {
            const aiNodeAnim* ch = anim->mChannels[j];
            std::string norm = normalizeBoneName(ch->mNodeName.C_Str());
            channelMap[norm] = ch;
        }

        std::unordered_set<int> usedBones;

        for (const auto& [normName, ch] : channelMap) {

            auto it = normalizedToBone.find(normName);
            if (it == normalizedToBone.end())
                continue;

            int boneIndex = it->second;
            Bone* bone = skeleton.getBone(boneIndex);
            if (!bone)
                continue;

            usedBones.insert(boneIndex);

            BoneTrack* track = clip->addBoneTrack();

            for (unsigned int i = 0; i < ch->mNumPositionKeys; ++i) {
                const auto& k = ch->mPositionKeys[i];
                track->addPositionKey({(float)k.mTime, {k.mValue.x, k.mValue.y, k.mValue.z}});
            }

            for (unsigned int i = 0; i < ch->mNumRotationKeys; ++i) {
                const auto& k = ch->mRotationKeys[i];
                track->addRotationKey(
                    {(float)k.mTime, glm::quat(k.mValue.w, k.mValue.x, k.mValue.y, k.mValue.z)});
            }

            for (unsigned int i = 0; i < ch->mNumScalingKeys; ++i) {
                const auto& k = ch->mScalingKeys[i];
                track->addScaleKey({(float)k.mTime, {k.mValue.x, k.mValue.y, k.mValue.z}});
            }

            track->bindBone(bone);
        }

        for (int i = 0; i < skeleton.getBoneCount(); ++i) {

            if (usedBones.count(i))
                continue;

            Bone* bone = skeleton.getBone(i);
            if (!bone)
                continue;

            BoneTrack* track = clip->addBoneTrack();

            glm::mat4 bind = bone->getBindLocalTransform();

            glm::vec3 pos, scale, skew;
            glm::quat rot;
            glm::vec4 persp;

            glm::decompose(bind, scale, rot, pos, skew, persp);

            track->addPositionKey({0.0f, pos});
            track->addRotationKey({0.0f, rot});
            track->addScaleKey({0.0f, scale});

            track->bindBone(bone);
        }

        std::string name =
            anim->mName.length > 0 ? anim->mName.C_Str() : "Anim_" + std::to_string(a);

        skeleton.addClip(name, clip);
    }
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

void dzemikk::ModelHandler::buildSkeleton(aiNode* node, Skeleton& skeleton, int parent,
                                          glm::mat4 accumulatedTransform) {
    std::string name = node->mName.C_Str();

    glm::mat4 local = aiToGlm(node->mTransformation);
    glm::mat4 newAccum = accumulatedTransform * local;

    int index = skeleton.addBone(name, parent);

    auto* bone = skeleton.getBone(index);
    if (bone) {
        bone->setLocalTransform(local);       
        bone->setBindLocalTransform(local); 
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        aiNode* child = node->mChildren[i];
        buildSkeleton(child, skeleton, index, newAccum);
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