#include "ecs/gameobject.h"
#include "ecs/scene.h"

#include <gtest/gtest.h>
#include <vector>

namespace {

// =============================================================================
// Scene findGameObject(s)ByName
// =============================================================================

TEST(SceneFindByName, FindReturnsCorrectObject) {
    dzemikk::Scene scene;
    dzemikk::GameObject* obj = scene.createGameObject("Player");
    ASSERT_NE(obj, nullptr);

    EXPECT_EQ(scene.findGameObjectByName("Player"), obj);
}

TEST(SceneFindByName, FindReturnsNullWhenNotFound) {
    dzemikk::Scene scene;

    EXPECT_EQ(scene.findGameObjectByName("Nonexistent"), nullptr);
}

TEST(SceneFindByName, FindReturnsFirstWhenMultipleSameName) {
    dzemikk::Scene scene;
    dzemikk::GameObject* first = scene.createGameObject("Enemy");
    dzemikk::GameObject* second = scene.createGameObject("Enemy");
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_EQ(scene.findGameObjectByName("Enemy"), first);
}

TEST(SceneFindByName, FindAllReturnsAllWithSameName) {
    dzemikk::Scene scene;
    dzemikk::GameObject* a = scene.createGameObject("Pickup");
    dzemikk::GameObject* b = scene.createGameObject("Pickup");
    dzemikk::GameObject* c = scene.createGameObject("Pickup");
    scene.createGameObject("Other");

    auto results = scene.findGameObjectsByName("Pickup");
    ASSERT_EQ(results.size(), 3U);
    EXPECT_EQ(results[0], a);
    EXPECT_EQ(results[1], b);
    EXPECT_EQ(results[2], c);
}

TEST(SceneFindByName, FindAllReturnsEmptyWhenNoneMatch) {
    dzemikk::Scene scene;
    scene.createGameObject("Alpha");

    auto results = scene.findGameObjectsByName("Beta");
    EXPECT_TRUE(results.empty());
}

TEST(SceneFindByName, NameChangeUpdatesIndex) {
    dzemikk::Scene scene;
    dzemikk::GameObject* obj = scene.createGameObject("OldName");

    obj->setName("NewName");

    EXPECT_EQ(scene.findGameObjectByName("OldName"), nullptr);
    EXPECT_EQ(scene.findGameObjectByName("NewName"), obj);
}

// =============================================================================
// Scene findGameObject(s)ByTag
// =============================================================================

TEST(SceneFindByTag, FindReturnsCorrectObject) {
    dzemikk::Scene scene;
    dzemikk::GameObject* obj = scene.createGameObject("Tagger");
    obj->addTag("Enemy");

    EXPECT_EQ(scene.findGameObjectByTag("Enemy"), obj);
}

TEST(SceneFindByTag, FindReturnsNullWhenNotFound) {
    dzemikk::Scene scene;

    EXPECT_EQ(scene.findGameObjectByTag("Nonexistent"), nullptr);
}

TEST(SceneFindByTag, FindReturnsFirstWhenMultipleSameTag) {
    dzemikk::Scene scene;
    dzemikk::GameObject* first = scene.createGameObject("A");
    dzemikk::GameObject* second = scene.createGameObject("B");
    first->addTag("Shared");
    second->addTag("Shared");

    EXPECT_EQ(scene.findGameObjectByTag("Shared"), first);
}

TEST(SceneFindByTag, FindAllReturnsAllWithSameTag) {
    dzemikk::Scene scene;
    dzemikk::GameObject* a = scene.createGameObject("A");
    dzemikk::GameObject* b = scene.createGameObject("B");
    dzemikk::GameObject* c = scene.createGameObject("C");
    scene.createGameObject("D");
    a->addTag("Collectible");
    b->addTag("Collectible");
    c->addTag("Collectible");

    auto results = scene.findGameObjectsByTag("Collectible");
    ASSERT_EQ(results.size(), 3U);
    EXPECT_EQ(results[0], a);
    EXPECT_EQ(results[1], b);
    EXPECT_EQ(results[2], c);
}

TEST(SceneFindByTag, FindAllReturnsEmptyWhenNoneMatch) {
    dzemikk::Scene scene;
    dzemikk::GameObject* obj = scene.createGameObject("A");
    obj->addTag("Foo");

    auto results = scene.findGameObjectsByTag("Bar");
    EXPECT_TRUE(results.empty());
}

TEST(SceneFindByTag, TagRemovalUpdatesIndex) {
    dzemikk::Scene scene;
    dzemikk::GameObject* obj = scene.createGameObject("A");
    obj->addTag("Temp");

    ASSERT_EQ(scene.findGameObjectByTag("Temp"), obj);
    obj->removeTag("Temp");

    EXPECT_EQ(scene.findGameObjectByTag("Temp"), nullptr);
    EXPECT_TRUE(scene.findGameObjectsByTag("Temp").empty());
}

TEST(SceneFindByTag, MultipleTagAddition) {
    dzemikk::Scene scene;
    dzemikk::GameObject* obj = scene.createGameObject("MultiTag");
    obj->addTag("TagA");
    obj->addTag("TagB");

    EXPECT_EQ(scene.findGameObjectByTag("TagA"), obj);
    EXPECT_EQ(scene.findGameObjectByTag("TagB"), obj);
}

TEST(SceneFindByTag, DuplicateTagHasNoEffect) {
    dzemikk::Scene scene;
    dzemikk::GameObject* obj = scene.createGameObject("A");
    obj->addTag("Enemy");
    obj->addTag("Enemy");

    auto results = scene.findGameObjectsByTag("Enemy");
    ASSERT_EQ(results.size(), 1U);
    EXPECT_EQ(results[0], obj);
}

// =============================================================================
// Scene findGameObject(s)ById
// =============================================================================

TEST(SceneFindById, FindReturnsCorrectObject) {
    dzemikk::Scene scene;
    dzemikk::GameObject* obj = scene.createGameObject("FindMe");
    ASSERT_NE(obj, nullptr);

    EXPECT_EQ(scene.findGameObjectById(obj->getId()), obj);
}

TEST(SceneFindById, FindReturnsNullWhenNotFound) {
    dzemikk::Scene scene;
    dzemikk::GameObject* obj = scene.createGameObject("Exists");
    ASSERT_NE(obj, nullptr);

    dzemikk::GameObject other;
    EXPECT_EQ(scene.findGameObjectById(other.getId()), nullptr);
}

TEST(SceneFindById, FindAllReturnsCorrectObject) {
    dzemikk::Scene scene;
    dzemikk::GameObject* obj = scene.createGameObject("FindMe");
    ASSERT_NE(obj, nullptr);

    auto results = scene.findGameObjectsById(obj->getId());
    ASSERT_EQ(results.size(), 1U);
    EXPECT_EQ(results[0], obj);
}

TEST(SceneFindById, FindAllReturnsEmptyWhenNotFound) {
    dzemikk::Scene scene;

    dzemikk::GameObject other;
    auto results = scene.findGameObjectsById(other.getId());
    EXPECT_TRUE(results.empty());
}

// =============================================================================
// GameObject findChild(ren)ByName
// =============================================================================

TEST(GameObjectFindChildByName, FindReturnsFirstMatchingChild) {
    dzemikk::Scene scene;
    dzemikk::GameObject* parent = scene.createGameObject("Parent");
    dzemikk::GameObject* childA = scene.createGameObject("Child");
    dzemikk::GameObject* childB = scene.createGameObject("Other");
    parent->addChild(childA);
    parent->addChild(childB);

    EXPECT_EQ(parent->findChildByName("Child"), childA);
}

TEST(GameObjectFindChildByName, FindReturnsNullWhenNotFound) {
    dzemikk::Scene scene;
    dzemikk::GameObject* parent = scene.createGameObject("Parent");
    dzemikk::GameObject* child = scene.createGameObject("Child");
    parent->addChild(child);

    EXPECT_EQ(parent->findChildByName("Nonexistent"), nullptr);
}

TEST(GameObjectFindChildByName, FindOnParentWithNoChildren) {
    dzemikk::Scene scene;
    dzemikk::GameObject* parent = scene.createGameObject("Parent");

    EXPECT_EQ(parent->findChildByName("Anything"), nullptr);
}

TEST(GameObjectFindChildByName, FindChildrenReturnsAllMatchingNames) {
    dzemikk::Scene scene;
    dzemikk::GameObject* parent = scene.createGameObject("Parent");
    dzemikk::GameObject* childA = scene.createGameObject("Enemy");
    dzemikk::GameObject* childB = scene.createGameObject("Friend");
    dzemikk::GameObject* childC = scene.createGameObject("Enemy");
    parent->addChild(childA);
    parent->addChild(childB);
    parent->addChild(childC);

    auto results = parent->findChildrenByName("Enemy");
    ASSERT_EQ(results.size(), 2U);
    EXPECT_EQ(results[0], childA);
    EXPECT_EQ(results[1], childC);
}

TEST(GameObjectFindChildByName, FindChildrenReturnsEmptyWhenNoneMatch) {
    dzemikk::Scene scene;
    dzemikk::GameObject* parent = scene.createGameObject("Parent");
    dzemikk::GameObject* child = scene.createGameObject("Child");
    parent->addChild(child);

    auto results = parent->findChildrenByName("NotFound");
    EXPECT_TRUE(results.empty());
}

TEST(GameObjectFindChildByName, FindChildrenOnParentWithNoChildren) {
    dzemikk::Scene scene;
    dzemikk::GameObject* parent = scene.createGameObject("Parent");

    auto results = parent->findChildrenByName("Anything");
    EXPECT_TRUE(results.empty());
}

// =============================================================================
// GameObject findChild(ren)ByTag
// =============================================================================

TEST(GameObjectFindChildByTag, FindReturnsFirstMatchingChild) {
    dzemikk::Scene scene;
    dzemikk::GameObject* parent = scene.createGameObject("Parent");
    dzemikk::GameObject* childA = scene.createGameObject("A");
    dzemikk::GameObject* childB = scene.createGameObject("B");
    childA->addTag("UIElement");
    childB->addTag("WorldElement");
    parent->addChild(childA);
    parent->addChild(childB);

    EXPECT_EQ(parent->findChildByTag("UIElement"), childA);
}

TEST(GameObjectFindChildByTag, FindReturnsNullWhenNotFound) {
    dzemikk::Scene scene;
    dzemikk::GameObject* parent = scene.createGameObject("Parent");
    dzemikk::GameObject* child = scene.createGameObject("Child");
    child->addTag("Foo");
    parent->addChild(child);

    EXPECT_EQ(parent->findChildByTag("Bar"), nullptr);
}

TEST(GameObjectFindChildByTag, FindOnParentWithNoChildren) {
    dzemikk::Scene scene;
    dzemikk::GameObject* parent = scene.createGameObject("Parent");

    EXPECT_EQ(parent->findChildByTag("AnyTag"), nullptr);
}

TEST(GameObjectFindChildByTag, FindChildrenReturnsAllMatchingTags) {
    dzemikk::Scene scene;
    dzemikk::GameObject* parent = scene.createGameObject("Parent");
    dzemikk::GameObject* childA = scene.createGameObject("A");
    dzemikk::GameObject* childB = scene.createGameObject("B");
    dzemikk::GameObject* childC = scene.createGameObject("C");
    childA->addTag("Collectible");
    childB->addTag("Other");
    childC->addTag("Collectible");
    parent->addChild(childA);
    parent->addChild(childB);
    parent->addChild(childC);

    auto results = parent->findChildrenByTag("Collectible");
    ASSERT_EQ(results.size(), 2U);
    EXPECT_EQ(results[0], childA);
    EXPECT_EQ(results[1], childC);
}

TEST(GameObjectFindChildByTag, FindChildrenReturnsEmptyWhenNoneMatch) {
    dzemikk::Scene scene;
    dzemikk::GameObject* parent = scene.createGameObject("Parent");
    dzemikk::GameObject* child = scene.createGameObject("Child");
    child->addTag("Foo");
    parent->addChild(child);

    auto results = parent->findChildrenByTag("Bar");
    EXPECT_TRUE(results.empty());
}

TEST(GameObjectFindChildByTag, FindChildrenOnParentWithNoChildren) {
    dzemikk::Scene scene;
    dzemikk::GameObject* parent = scene.createGameObject("Parent");

    auto results = parent->findChildrenByTag("AnyTag");
    EXPECT_TRUE(results.empty());
}

// =============================================================================
// GameObject findDescendant(s)ByName
// =============================================================================

TEST(GameObjectFindDescendantByName, FindDirectChild) {
    dzemikk::Scene scene;
    dzemikk::GameObject* root = scene.createGameObject("Root");
    dzemikk::GameObject* child = scene.createGameObject("Target");
    root->addChild(child);

    EXPECT_EQ(root->findDescendantByName("Target"), child);
}

TEST(GameObjectFindDescendantByName, FindDeepNestedDescendant) {
    dzemikk::Scene scene;
    dzemikk::GameObject* grandparent = scene.createGameObject("GrandParent");
    dzemikk::GameObject* parent = scene.createGameObject("Parent");
    dzemikk::GameObject* child = scene.createGameObject("DeepTarget");
    grandparent->addChild(parent);
    parent->addChild(child);

    EXPECT_EQ(grandparent->findDescendantByName("DeepTarget"), child);
}

TEST(GameObjectFindDescendantByName, FindReturnsNullWhenNotFound) {
    dzemikk::Scene scene;
    dzemikk::GameObject* root = scene.createGameObject("Root");
    dzemikk::GameObject* child = scene.createGameObject("Child");
    root->addChild(child);

    EXPECT_EQ(root->findDescendantByName("Nonexistent"), nullptr);
}

TEST(GameObjectFindDescendantByName, FindOnLeafNode) {
    dzemikk::Scene scene;
    dzemikk::GameObject* leaf = scene.createGameObject("Leaf");

    EXPECT_EQ(leaf->findDescendantByName("Anything"), nullptr);
}

TEST(GameObjectFindDescendantByName, FindDescendantsReturnsAllMatchingInSubtree) {
    dzemikk::Scene scene;
    dzemikk::GameObject* root = scene.createGameObject("Root");
    dzemikk::GameObject* childA = scene.createGameObject("Enemy");
    dzemikk::GameObject* childB = scene.createGameObject("Friend");
    dzemikk::GameObject* grandchildA1 = scene.createGameObject("Enemy");
    dzemikk::GameObject* grandchildB1 = scene.createGameObject("Friend");
    dzemikk::GameObject* grandchildA2 = scene.createGameObject("Enemy");
    root->addChild(childA);
    root->addChild(childB);
    childA->addChild(grandchildA1);
    childA->addChild(grandchildB1);
    childB->addChild(grandchildA2);

    auto results = root->findDescendantsByName("Enemy");
    ASSERT_EQ(results.size(), 3U);
}

TEST(GameObjectFindDescendantByName, FindDescendantsReturnsEmptyWhenNoneMatch) {
    dzemikk::Scene scene;
    dzemikk::GameObject* root = scene.createGameObject("Root");
    dzemikk::GameObject* child = scene.createGameObject("Child");
    root->addChild(child);

    auto results = root->findDescendantsByName("NotFound");
    EXPECT_TRUE(results.empty());
}

TEST(GameObjectFindDescendantByName, FindDescendantsOnLeafNode) {
    dzemikk::Scene scene;
    dzemikk::GameObject* leaf = scene.createGameObject("Leaf");

    auto results = leaf->findDescendantsByName("Anything");
    EXPECT_TRUE(results.empty());
}

// =============================================================================
// GameObject findDescendant(s)ByTag
// =============================================================================

TEST(GameObjectFindDescendantByTag, FindDirectChild) {
    dzemikk::Scene scene;
    dzemikk::GameObject* root = scene.createGameObject("Root");
    dzemikk::GameObject* child = scene.createGameObject("Child");
    child->addTag("Target");
    root->addChild(child);

    EXPECT_EQ(root->findDescendantByTag("Target"), child);
}

TEST(GameObjectFindDescendantByTag, FindDeepNestedDescendant) {
    dzemikk::Scene scene;
    dzemikk::GameObject* grandparent = scene.createGameObject("GrandParent");
    dzemikk::GameObject* parent = scene.createGameObject("Parent");
    dzemikk::GameObject* child = scene.createGameObject("Child");
    child->addTag("DeepTag");
    grandparent->addChild(parent);
    parent->addChild(child);

    EXPECT_EQ(grandparent->findDescendantByTag("DeepTag"), child);
}

TEST(GameObjectFindDescendantByTag, FindReturnsNullWhenNotFound) {
    dzemikk::Scene scene;
    dzemikk::GameObject* root = scene.createGameObject("Root");
    dzemikk::GameObject* child = scene.createGameObject("Child");
    child->addTag("Tag");
    root->addChild(child);

    EXPECT_EQ(root->findDescendantByTag("MissingTag"), nullptr);
}

TEST(GameObjectFindDescendantByTag, FindOnLeafNode) {
    dzemikk::Scene scene;
    dzemikk::GameObject* leaf = scene.createGameObject("Leaf");

    EXPECT_EQ(leaf->findDescendantByTag("AnyTag"), nullptr);
}

TEST(GameObjectFindDescendantByTag, FindDescendantsReturnsAllMatchingInSubtree) {
    dzemikk::Scene scene;
    dzemikk::GameObject* root = scene.createGameObject("Root");
    dzemikk::GameObject* childA = scene.createGameObject("A");
    dzemikk::GameObject* childB = scene.createGameObject("B");
    dzemikk::GameObject* grandchildA1 = scene.createGameObject("A1");
    dzemikk::GameObject* grandchildB1 = scene.createGameObject("B1");
    dzemikk::GameObject* grandchildA2 = scene.createGameObject("A2");
    childA->addTag("Item");
    grandchildA1->addTag("Item");
    grandchildA2->addTag("Item");
    root->addChild(childA);
    root->addChild(childB);
    childA->addChild(grandchildA1);
    childA->addChild(grandchildB1);
    childB->addChild(grandchildA2);

    auto results = root->findDescendantsByTag("Item");
    ASSERT_EQ(results.size(), 3U);
}

TEST(GameObjectFindDescendantByTag, FindDescendantsReturnsEmptyWhenNoneMatch) {
    dzemikk::Scene scene;
    dzemikk::GameObject* root = scene.createGameObject("Root");
    dzemikk::GameObject* child = scene.createGameObject("Child");
    child->addTag("Foo");
    root->addChild(child);

    auto results = root->findDescendantsByTag("Bar");
    EXPECT_TRUE(results.empty());
}

TEST(GameObjectFindDescendantByTag, FindDescendantsOnLeafNode) {
    dzemikk::Scene scene;
    dzemikk::GameObject* leaf = scene.createGameObject("Leaf");

    auto results = leaf->findDescendantsByTag("AnyTag");
    EXPECT_TRUE(results.empty());
}

} // namespace
