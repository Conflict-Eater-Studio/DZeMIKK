#include "scene/octree.h"
#include <algorithm>

namespace dzemikk {

OctreeNode::OctreeNode(const glm::vec3& center, float halfSize, int depth)
    : _center(center), _halfSize(halfSize), _depth(depth), _isLeaf(true) {}

void OctreeNode::insert(GameObject* go, const glm::vec3& minBounds, const glm::vec3& maxBounds) {
    if (_isLeaf) {
        _items.push_back({go, minBounds, maxBounds});
        
        if (_items.size() > MAX_OBJECTS && _depth < MAX_DEPTH) {
            split();
        }
        return;
    }

    bool insertedIntoChild = false;
    for (int i = 0; i < 8; ++i) {
        glm::vec3 childMin = _children[i]->_center - glm::vec3(_children[i]->_halfSize);
        glm::vec3 childMax = _children[i]->_center + glm::vec3(_children[i]->_halfSize);

        if (minBounds.x <= childMax.x && maxBounds.x >= childMin.x &&
            minBounds.y <= childMax.y && maxBounds.y >= childMin.y &&
            minBounds.z <= childMax.z && maxBounds.z >= childMin.z) {
            _children[i]->insert(go, minBounds, maxBounds);
            insertedIntoChild = true;
        }
    }

    if (!insertedIntoChild) {
        _items.push_back({go, minBounds, maxBounds});
    }
}

void OctreeNode::split() {
    float quarter = _halfSize / 2.0f;
    for (int i = 0; i < 8; ++i) {
        glm::vec3 offset(
            (i & 1) ? quarter : -quarter,
            (i & 2) ? quarter : -quarter,
            (i & 4) ? quarter : -quarter
        );
        _children[i] = std::make_unique<OctreeNode>(_center + offset, quarter, _depth + 1);
    }
    _isLeaf = false;

    // Redistribute items
    std::vector<OctreeItem> oldItems = std::move(_items);
    _items.clear();

    for (const auto& item : oldItems) {
        bool insertedIntoChild = false;
        for (int i = 0; i < 8; ++i) {
            glm::vec3 childMin = _children[i]->_center - glm::vec3(_children[i]->_halfSize);
            glm::vec3 childMax = _children[i]->_center + glm::vec3(_children[i]->_halfSize);

            if (item.minBox.x <= childMax.x && item.maxBox.x >= childMin.x &&
                item.minBox.y <= childMax.y && item.maxBox.y >= childMin.y &&
                item.minBox.z <= childMax.z && item.maxBox.z >= childMin.z) {
                _children[i]->insert(item.go, item.minBox, item.maxBox);
                insertedIntoChild = true;
            }
        }
        if (!insertedIntoChild) {
            _items.push_back(item);
        }
    }
}

void OctreeNode::queryFrustum(const Frustum& frustum, std::vector<GameObject*>& result) const {
    // We are skipping queryFrustum implementation as it's not needed for the current optimization plan
}

void OctreeNode::queryRay(const glm::vec3& rayOrigin, const glm::vec3& rayDir, std::vector<GameObject*>& result) const {
    if (!intersectRayAABB(rayOrigin, rayDir)) {
        return;
    }

    for (const auto& item : _items) {
        result.push_back(item.go);
    }

    if (!_isLeaf) {
        for (int i = 0; i < 8; ++i) {
            _children[i]->queryRay(rayOrigin, rayDir, result);
        }
    }
}

bool OctreeNode::intersectRayAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDir) const {
    glm::vec3 dirFrac;
    dirFrac.x = rayDir.x != 0.0f ? 1.0f / rayDir.x : 1e-6f;
    dirFrac.y = rayDir.y != 0.0f ? 1.0f / rayDir.y : 1e-6f;
    dirFrac.z = rayDir.z != 0.0f ? 1.0f / rayDir.z : 1e-6f;

    glm::vec3 minBox = _center - glm::vec3(_halfSize);
    glm::vec3 maxBox = _center + glm::vec3(_halfSize);

    float t1 = (minBox.x - rayOrigin.x) * dirFrac.x;
    float t2 = (maxBox.x - rayOrigin.x) * dirFrac.x;
    float t3 = (minBox.y - rayOrigin.y) * dirFrac.y;
    float t4 = (maxBox.y - rayOrigin.y) * dirFrac.y;
    float t5 = (minBox.z - rayOrigin.z) * dirFrac.z;
    float t6 = (maxBox.z - rayOrigin.z) * dirFrac.z;

    float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
    float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

    if (tmax < 0) return false;
    if (tmin > tmax) return false;
    return true;
}

Octree::Octree(const glm::vec3& center, float halfSize) 
    : _center(center), _halfSize(halfSize) {
    _root = std::make_unique<OctreeNode>(center, halfSize, 0);
}

void Octree::insert(GameObject* go, const glm::vec3& minBounds, const glm::vec3& maxBounds) {
    _root->insert(go, minBounds, maxBounds);
}

void Octree::clear() {
    _root = std::make_unique<OctreeNode>(_center, _halfSize, 0);
}

void Octree::queryFrustum(const Frustum& frustum, std::vector<GameObject*>& result) const {
    _root->queryFrustum(frustum, result);
}

void Octree::queryRay(const glm::vec3& rayOrigin, const glm::vec3& rayDir, std::vector<GameObject*>& result) const {
    _root->queryRay(rayOrigin, rayDir, result);
}

}
