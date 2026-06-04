#include "ecs/components/transform.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <glm/glm.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <vector>

namespace {

constexpr float kEpsilon = 1e-5F;

bool vec3Near(const glm::vec3& vec1, const glm::vec3& vec2, float eps = kEpsilon) {
    return glm::length(vec1 - vec2) < eps;
}

struct BenchmarkScene {
    dzemikk::Scene scene;
    dzemikk::GameObject* root = nullptr;
    std::vector<dzemikk::GameObject*> ownedObjects;
    std::vector<dzemikk::GameObject*> allObjects;
};

bool makeHierarchyScene(BenchmarkScene& scene, const std::vector<std::size_t>& branchingPerDepth,
                        std::mt19937& rng) {
    std::cerr << "Creating hierarchy\n";
    scene.root = scene.scene.createGameObject();
    scene.ownedObjects.clear();
    scene.allObjects.clear();

    std::uniform_real_distribution<float> pos(-200.0F, 200.0F);

    scene.allObjects.push_back(scene.root);

    std::vector<dzemikk::GameObject*> currentLevel;
    currentLevel.push_back(scene.root);

    for (const std::size_t childrenPerNode : branchingPerDepth) {
        std::vector<dzemikk::GameObject*> nextLevel;
        nextLevel.reserve(currentLevel.size() * childrenPerNode);

        for (dzemikk::GameObject* parent : currentLevel) {
            for (std::size_t i = 0; i < childrenPerNode; ++i) {
                auto* node = scene.scene.createGameObject(parent);
                node->transform()->setPosition(glm::vec3(pos(rng), pos(rng), pos(rng)));

                scene.allObjects.push_back(node);
                nextLevel.push_back(node);
                scene.ownedObjects.push_back(node);
            }
        }

        currentLevel = std::move(nextLevel);
    }

    std::cerr << "Created hierarchy\n";
    return true;
}

double benchmarkGetWorldMatrix(const std::vector<dzemikk::GameObject*>& objects, int iterations) {
    std::cerr << "beanch start\n";
    if (objects.empty() || iterations <= 0) {
        ADD_FAILURE() << "Invalid benchmark input";
        return 0.0;
    }

    volatile float sink = 0.0F;

    for (dzemikk::GameObject* object : objects) {
        const glm::mat4 warmup = object->transform()->getWorldMatrix();
        sink += warmup[3].x;
    }

    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < iterations; ++i) {
        for (dzemikk::GameObject* object : objects) {
            const glm::mat4 world = object->transform()->getWorldMatrix();
            sink += world[3].x;
        }
    }
    const auto end = std::chrono::steady_clock::now();

    EXPECT_FALSE(std::isnan(sink));

    const auto elapsed = std::chrono::duration<double, std::micro>(end - start).count();
    std::cerr << "bench end\n";
    return elapsed / static_cast<double>(objects.size() * iterations);
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
    dzemikk::Scene scene;

    dzemikk::GameObject* parent = scene.createGameObject();
    dzemikk::GameObject* child = scene.createGameObject();

    child->setParent(parent);

    EXPECT_EQ(child->getParent(), parent);
    const auto& children = parent->getChildren();
    ASSERT_EQ(children.size(), 1U);
    EXPECT_EQ(children[0], child);
}

TEST(Hierarchy, ReParentRemovesFromOldParent) {
    dzemikk::Scene scene;

    dzemikk::GameObject* parentA = scene.createGameObject();
    dzemikk::GameObject* parentB = scene.createGameObject();
    dzemikk::GameObject* child = scene.createGameObject();

    child->setParent(parentA);
    ASSERT_EQ(parentA->getChildren().size(), 1U);

    child->setParent(parentB);

    EXPECT_EQ(child->getParent(), parentB);
    EXPECT_TRUE(parentA->getChildren().empty());
    ASSERT_EQ(parentB->getChildren().size(), 1U);
    EXPECT_EQ(parentB->getChildren()[0], child);
}

TEST(Hierarchy, SetParentNullUnparents) {
    dzemikk::Scene scene;

    dzemikk::GameObject* parent = scene.createGameObject();
    dzemikk::GameObject* child = scene.createGameObject();

    child->setParent(parent);
    child->setParent(nullptr);

    EXPECT_EQ(child->getParent(), nullptr);
    EXPECT_TRUE(parent->getChildren().empty());
}

TEST(Hierarchy, AddChildSetsParent) {
    dzemikk::Scene scene;

    dzemikk::GameObject* parent = scene.createGameObject();
    dzemikk::GameObject* child = scene.createGameObject();

    parent->addChild(child);

    EXPECT_EQ(child->getParent(), parent);
    ASSERT_EQ(parent->getChildren().size(), 1U);
    EXPECT_EQ(parent->getChildren()[0], child);
}

TEST(Hierarchy, AddChildSelfIgnored) {
    dzemikk::Scene scene;

    dzemikk::GameObject* obj = scene.createGameObject();
    obj->addChild(obj);
    EXPECT_TRUE(obj->getChildren().empty());
}

TEST(Hierarchy, MultipleChildren) {
    dzemikk::Scene scene;

    dzemikk::GameObject* parent = scene.createGameObject();
    dzemikk::GameObject* child1 = scene.createGameObject();
    dzemikk::GameObject* child2 = scene.createGameObject();
    dzemikk::GameObject* child3 = scene.createGameObject();

    parent->addChild(child1);
    parent->addChild(child2);
    parent->addChild(child3);

    ASSERT_EQ(parent->getChildren().size(), 3U);
    EXPECT_EQ(child1->getParent(), parent);
    EXPECT_EQ(child2->getParent(), parent);
    EXPECT_EQ(child3->getParent(), parent);
}

// ---------------------------------------------------------------------------
// Transform - local
// ---------------------------------------------------------------------------

TEST(TransformLocal, DefaultPositionIsZero) {
    dzemikk::Scene scene;

    dzemikk::GameObject* obj = scene.createGameObject();
    auto pos = obj->transform()->getPosition();
    EXPECT_TRUE(vec3Near(pos, glm::vec3(0.0F)));
}

TEST(TransformLocal, SetPosition) {
    dzemikk::Scene scene;

    dzemikk::GameObject* obj = scene.createGameObject();
    obj->transform()->setPosition(glm::vec3(1.0F, 2.0F, 3.0F));
    EXPECT_TRUE(vec3Near(obj->transform()->getPosition(), glm::vec3(1.0F, 2.0F, 3.0F)));
}

TEST(TransformLocal, Translate) {
    dzemikk::Scene scene;

    dzemikk::GameObject* obj = scene.createGameObject();
    obj->transform()->setPosition(glm::vec3(1.0F, 0.0F, 0.0F));
    obj->transform()->translate(glm::vec3(0.0F, 5.0F, -2.0F));
    EXPECT_TRUE(vec3Near(obj->transform()->getPosition(), glm::vec3(1.0F, 5.0F, -2.0F)));
}

TEST(TransformLocal, DefaultScaleIsOne) {
    dzemikk::Scene scene;

    dzemikk::GameObject* obj = scene.createGameObject();
    EXPECT_TRUE(vec3Near(obj->transform()->getScale(), glm::vec3(1.0F)));
}

TEST(TransformLocal, LocalMatrixTranslation) {
    dzemikk::Scene scene;

    dzemikk::GameObject* obj = scene.createGameObject();
    obj->transform()->setPosition(glm::vec3(10.0F, 0.0F, 0.0F));
    glm::mat4 mat = obj->transform()->getLocalMatrix();
    // Translation column is m[3]
    EXPECT_TRUE(vec3Near(glm::vec3(mat[3]), glm::vec3(10.0F, 0.0F, 0.0F)));
}

// ---------------------------------------------------------------------------
// Transform - world
// ---------------------------------------------------------------------------

TEST(TransformWorld, NoParentWorldEqualsLocal) {
    dzemikk::Scene scene;

    dzemikk::GameObject* obj = scene.createGameObject();
    obj->transform()->setPosition(glm::vec3(5.0F, 0.0F, 0.0F));
    glm::mat4 world = obj->transform()->getWorldMatrix();
    glm::mat4 local = obj->transform()->getLocalMatrix();
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_NEAR(world[i][j], local[i][j], kEpsilon);
        }
    }
}

TEST(TransformWorld, ChildPositionIsParentPlusLocal) {
    dzemikk::Scene scene;
    dzemikk::GameObject* parent = scene.createGameObject();
    dzemikk::GameObject* child = scene.createGameObject();

    parent->transform()->setPosition(glm::vec3(10.0F, 0.0F, 0.0F));
    child->transform()->setPosition(glm::vec3(1.0F, 0.0F, 0.0F));
    child->setParent(parent);

    glm::mat4 world = child->transform()->getWorldMatrix();
    auto worldPos = glm::vec3(world[3]);
    EXPECT_TRUE(vec3Near(worldPos, glm::vec3(11.0F, 0.0F, 0.0F)));
}

TEST(TransformWorld, DeepNesting) {
    dzemikk::Scene scene;

    dzemikk::GameObject* root = scene.createGameObject();
    dzemikk::GameObject* mid = scene.createGameObject();
    dzemikk::GameObject* leaf = scene.createGameObject();

    root->transform()->setPosition(glm::vec3(1.0F, 0.0F, 0.0F));
    mid->transform()->setPosition(glm::vec3(2.0F, 0.0F, 0.0F));
    leaf->transform()->setPosition(glm::vec3(3.0F, 0.0F, 0.0F));

    mid->setParent(root);
    leaf->setParent(mid);

    glm::mat4 world = leaf->transform()->getWorldMatrix();
    auto worldPos = glm::vec3(world[3]);
    EXPECT_TRUE(vec3Near(worldPos, glm::vec3(6.0F, 0.0F, 0.0F)));
}

TEST(TransformWorld, Performance) {
    std::mt19937 rng(1337U);

    struct Case {
        std::vector<std::size_t> branchingPerDepth;
    };

    const std::vector<Case> cases = {
        {{1000}},
        {{1000, 100}},
        {{100, 100, 10}},
        {{100, 100, 10, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}},
    };

    constexpr int kIterations = 5;

    std::cerr << "\ngetWorldMatrix benchmark (avg microseconds per call)\n";
    for (const auto& testCase : cases) {
        std::cerr << "Running case\n";
        BenchmarkScene scene;
        if (!makeHierarchyScene(scene, testCase.branchingPerDepth, rng)) {
            std::cerr << "  skipped: fanout too large (overflow or >2,000,000 nodes)\n";
            continue;
        }

        const double avgMicros = benchmarkGetWorldMatrix(scene.allObjects, kIterations);

        const std::size_t depth = testCase.branchingPerDepth.size() + 1;

        std::cerr << "  depth=" << depth << ", objects=" << scene.allObjects.size() << ", fanout=[";

        for (std::size_t i = 0; i < testCase.branchingPerDepth.size(); ++i) {
            std::cerr << testCase.branchingPerDepth[i];
            if (i + 1 < testCase.branchingPerDepth.size()) {
                std::cerr << ",";
            }
        }

        std::cerr << "]"
                  << ", avg_us=" << avgMicros;

        std::cerr << '\n';
    }

    SUCCEED();
}
