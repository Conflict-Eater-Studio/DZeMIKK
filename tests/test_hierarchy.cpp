#include <gtest/gtest.h>

#include "ecs/gameobject.h"
#include "ecs/components/transform.h"

#include <glm/glm/glm.hpp>
#include <cmath>

namespace {

constexpr float kEpsilon = 1e-5F;

bool vec3Near(const glm::vec3& vec1, const glm::vec3& vec2, float eps = kEpsilon) {
    return glm::length(vec1 - vec2) < eps;
}

} // namespace

// ---------------------------------------------------------------------------
// Hierarchy
// ---------------------------------------------------------------------------

TEST(Hierarchy, DefaultParentIsNull) {
    dzemikk::GameObject obj;
    EXPECT_EQ(obj.getParent(), nullptr);
}

TEST(Hierarchy, SetParentLinksBothWays) {
    dzemikk::GameObject parent;
    dzemikk::GameObject child;

    child.setParent(&parent);

    EXPECT_EQ(child.getParent(), &parent);
    const auto& children = parent.getChildren();
    ASSERT_EQ(children.size(), 1U);
    EXPECT_EQ(children[0], &child);
}

TEST(Hierarchy, ReParentRemovesFromOldParent) {
    dzemikk::GameObject parentA;
    dzemikk::GameObject parentB;
    dzemikk::GameObject child;

    child.setParent(&parentA);
    ASSERT_EQ(parentA.getChildren().size(), 1U);

    child.setParent(&parentB);

    EXPECT_EQ(child.getParent(), &parentB);
    EXPECT_TRUE(parentA.getChildren().empty());
    ASSERT_EQ(parentB.getChildren().size(), 1U);
    EXPECT_EQ(parentB.getChildren()[0], &child);
}

TEST(Hierarchy, SetParentNullUnparents) {
    dzemikk::GameObject parent;
    dzemikk::GameObject child;

    child.setParent(&parent);
    child.setParent(nullptr);

    EXPECT_EQ(child.getParent(), nullptr);
    EXPECT_TRUE(parent.getChildren().empty());
}

TEST(Hierarchy, AddChildSetsParent) {
    dzemikk::GameObject parent;
    dzemikk::GameObject child;

    parent.addChild(&child);

    EXPECT_EQ(child.getParent(), &parent);
    ASSERT_EQ(parent.getChildren().size(), 1U);
    EXPECT_EQ(parent.getChildren()[0], &child);
}

TEST(Hierarchy, AddChildSelfIgnored) {
    dzemikk::GameObject obj;
    obj.addChild(&obj);
    EXPECT_TRUE(obj.getChildren().empty());
}

TEST(Hierarchy, MultipleChildren) {
    dzemikk::GameObject parent;
    dzemikk::GameObject child1;
    dzemikk::GameObject child2;
    dzemikk::GameObject child3;

    parent.addChild(&child1);
    parent.addChild(&child2);
    parent.addChild(&child3);

    ASSERT_EQ(parent.getChildren().size(), 3U);
    EXPECT_EQ(child1.getParent(), &parent);
    EXPECT_EQ(child2.getParent(), &parent);
    EXPECT_EQ(child3.getParent(), &parent);
}

// ---------------------------------------------------------------------------
// Transform - local
// ---------------------------------------------------------------------------

TEST(TransformLocal, DefaultPositionIsZero) {
    dzemikk::GameObject obj;
    auto pos = obj.transform()->getPosition();
    EXPECT_TRUE(vec3Near(pos, glm::vec3(0.0F)));
}

TEST(TransformLocal, SetPosition) {
    dzemikk::GameObject obj;
    obj.transform()->setPosition(glm::vec3(1.0F, 2.0F, 3.0F));
    EXPECT_TRUE(vec3Near(obj.transform()->getPosition(), glm::vec3(1.0F, 2.0F, 3.0F)));
}

TEST(TransformLocal, Translate) {
    dzemikk::GameObject obj;
    obj.transform()->setPosition(glm::vec3(1.0F, 0.0F, 0.0F));
    obj.transform()->translate(glm::vec3(0.0F, 5.0F, -2.0F));
    EXPECT_TRUE(vec3Near(obj.transform()->getPosition(), glm::vec3(1.0F, 5.0F, -2.0F)));
}

TEST(TransformLocal, DefaultScaleIsOne) {
    dzemikk::GameObject obj;
    EXPECT_TRUE(vec3Near(obj.transform()->getScale(), glm::vec3(1.0F)));
}

TEST(TransformLocal, LocalMatrixTranslation) {
    dzemikk::GameObject obj;
    obj.transform()->setPosition(glm::vec3(10.0F, 0.0F, 0.0F));
    glm::mat4 mat = obj.transform()->getLocalMatrix();
    // Translation column is m[3]
    EXPECT_TRUE(vec3Near(glm::vec3(mat[3]), glm::vec3(10.0F, 0.0F, 0.0F)));
}

// ---------------------------------------------------------------------------
// Transform - world
// ---------------------------------------------------------------------------

TEST(TransformWorld, NoParentWorldEqualsLocal) {
    dzemikk::GameObject obj;
    obj.transform()->setPosition(glm::vec3(5.0F, 0.0F, 0.0F));
    glm::mat4 world = obj.transform()->getWorldMatrix();
    glm::mat4 local = obj.transform()->getLocalMatrix();
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_NEAR(world[i][j], local[i][j], kEpsilon);
        }
    }
}

TEST(TransformWorld, ChildPositionIsParentPlusLocal) {
    dzemikk::GameObject parent;
    dzemikk::GameObject child;

    parent.transform()->setPosition(glm::vec3(10.0F, 0.0F, 0.0F));
    child.transform()->setPosition(glm::vec3(1.0F, 0.0F, 0.0F));
    child.setParent(&parent);

    glm::mat4 world = child.transform()->getWorldMatrix();
    auto worldPos = glm::vec3(world[3]);
    EXPECT_TRUE(vec3Near(worldPos, glm::vec3(11.0F, 0.0F, 0.0F)));
}

TEST(TransformWorld, DeepNesting) {
    dzemikk::GameObject root;
    dzemikk::GameObject mid;
    dzemikk::GameObject leaf;

    root.transform()->setPosition(glm::vec3(1.0F, 0.0F, 0.0F));
    mid.transform()->setPosition(glm::vec3(2.0F, 0.0F, 0.0F));
    leaf.transform()->setPosition(glm::vec3(3.0F, 0.0F, 0.0F));

    mid.setParent(&root);
    leaf.setParent(&mid);

    glm::mat4 world = leaf.transform()->getWorldMatrix();
    auto worldPos = glm::vec3(world[3]);
    EXPECT_TRUE(vec3Near(worldPos, glm::vec3(6.0F, 0.0F, 0.0F)));
}
