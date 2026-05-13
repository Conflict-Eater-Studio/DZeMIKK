#ifndef DZEMIKK_OCTREE_H
#define DZEMIKK_OCTREE_H

#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace dzemikk {

class GameObject;
class Frustum;

struct OctreeItem {
    GameObject* go;
    glm::vec3 minBox;
    glm::vec3 maxBox;
};

class OctreeNode {
public:
    OctreeNode(const glm::vec3& center, float halfSize, int depth);

    void insert(GameObject* go, const glm::vec3& minBounds, const glm::vec3& maxBounds);
    void queryFrustum(const Frustum& frustum, std::vector<GameObject*>& result) const;
    void queryRay(const glm::vec3& rayOrigin, const glm::vec3& rayDir, std::vector<GameObject*>& result) const;

private:
    void split();
    bool intersectRayAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDir) const;

    glm::vec3 _center;
    float _halfSize;
    int _depth;
    bool _isLeaf;

    std::vector<OctreeItem> _items;
    std::unique_ptr<OctreeNode> _children[8];

    static constexpr int MAX_OBJECTS = 20;
    static constexpr int MAX_DEPTH = 6;
};

class Octree {
public:
    Octree(const glm::vec3& center, float halfSize);

    void insert(GameObject* go, const glm::vec3& minBounds, const glm::vec3& maxBounds);
    void clear();

    void queryFrustum(const Frustum& frustum, std::vector<GameObject*>& result) const;
    void queryRay(const glm::vec3& rayOrigin, const glm::vec3& rayDir, std::vector<GameObject*>& result) const;

private:
    std::unique_ptr<OctreeNode> _root;
    glm::vec3 _center;
    float _halfSize;
};

}

#endif // DZEMIKK_OCTREE_H
