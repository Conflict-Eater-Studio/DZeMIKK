#include "ecs/component.h"
#include "ecs/components/monoBehaviour.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"

#include <algorithm>
#include <gtest/gtest.h>
#include <vector>

namespace {

struct CallbackCounters {
    int startCount = 0;
    int updateCount = 0;
    int lateUpdateCount = 0;
    int fixedUpdateCount = 0;
    int destroyCount = 0;
};

class TestComponent final : public dzemikk::Component {
  public:
    using Base = dzemikk::Component;

    explicit TestComponent(int value = 0) : value(value) {}

    [[nodiscard]] std::string typeName() const override {
        return "TestComponent";
    }

    int value = 0;
};

class AnotherComponent final : public dzemikk::Component {
  public:
    using Base = dzemikk::Component;

    [[nodiscard]] std::string typeName() const override {
        return "AnotherComponent";
    }
};

class BaseComponent : public dzemikk::Component {
  public:
    using Base = dzemikk::Component;

    [[nodiscard]] std::string typeName() const override {
        return "BaseComponent";
    }
};

class DerivedComponent final : public BaseComponent {
  public:
    using Base = BaseComponent;

    [[nodiscard]] std::string typeName() const override {
        return "DerivedComponent";
    }
};

class CountingMonoBehaviour final : public dzemikk::MonoBehaviour {
  public:
    explicit CountingMonoBehaviour(CallbackCounters* counters) : _counters(counters) {}

    [[nodiscard]] std::string typeName() const override {
        return "CountingMonoBehaviour";
    }

    void start() override {
        if (_counters) {
            ++_counters->startCount;
        }
    }

    void update(double /*delta_time*/) override {
        if (_counters) {
            ++_counters->updateCount;
        }
    }

    void lateUpdate() override {
        if (_counters) {
            ++_counters->lateUpdateCount;
        }
    }

    void fixedUpdate(double /*delta_time*/) override {
        if (_counters) {
            ++_counters->fixedUpdateCount;
        }
    }

    void onDestroy() override {
        if (_counters) {
            ++_counters->destroyCount;
        }
    }

  private:
    CallbackCounters* _counters = nullptr;
};

TEST(GameObjectCore, TransformOwnerIsSelfOnConstruction) {
    dzemikk::GameObject object;

    ASSERT_NE(object.transform(), nullptr);
    EXPECT_EQ(object.transform()->getOwner(), &object);

    const dzemikk::GameObject& constObject = object;
    ASSERT_NE(constObject.transform(), nullptr);
    EXPECT_EQ(constObject.transform()->getOwner(), &object);
}

TEST(GameObjectCore, NameAndStartedFlagWork) {
    dzemikk::GameObject object;

    EXPECT_EQ(object.getName(), "");
    EXPECT_FALSE(object.hasStarted());

    object.setName("Player");
    object.markStarted();

    EXPECT_EQ(object.getName(), "Player");
    EXPECT_TRUE(object.hasStarted());
}

TEST(GameObjectComponents, AddAndGetPlainComponent) {
    dzemikk::GameObject object;

    auto* component = object.addComponent<TestComponent>(42);
    ASSERT_NE(component, nullptr);

    EXPECT_EQ(component->value, 42);
    EXPECT_EQ(component->getOwner(), &object);
    EXPECT_EQ(object.getComponent<TestComponent>(), component);
    EXPECT_EQ(object.getComponent<AnotherComponent>(), nullptr);
}

TEST(GameObjectComponents, GetComponentsReturnsAllMatchingTypes) {
    dzemikk::GameObject object;

    auto* baseComponent = object.addComponent<BaseComponent>();
    auto* derivedComponent = object.addComponent<DerivedComponent>();

    auto baseComponents = object.getComponents<BaseComponent>();
    ASSERT_EQ(baseComponents.size(), 2U);
    EXPECT_NE(std::ranges::find(baseComponents, baseComponent), baseComponents.end());
    EXPECT_NE(std::ranges::find(baseComponents, derivedComponent), baseComponents.end());

    auto derivedComponents = object.getComponents<DerivedComponent>();
    ASSERT_EQ(derivedComponents.size(), 1U);
    EXPECT_EQ(derivedComponents.front(), derivedComponent);
}

TEST(GameObjectComponents, RemoveComponentClearsQueries) {
    dzemikk::GameObject object;

    auto* component = object.addComponent<TestComponent>(7);
    ASSERT_NE(component, nullptr);

    object.removeComponent(component);

    EXPECT_EQ(object.getComponent<TestComponent>(), nullptr);
    EXPECT_TRUE(object.getComponents<TestComponent>().empty());
}

TEST(GameObjectMonoBehaviour, AddMonoBehaviourCachesAndSetsOwner) {
    dzemikk::GameObject object;
    CallbackCounters counters;

    auto* mono = object.addComponent<CountingMonoBehaviour>(&counters);
    ASSERT_NE(mono, nullptr);

    const auto& monoBehaviours = object.getMonoBehaviours();
    ASSERT_EQ(monoBehaviours.size(), 1U);
    EXPECT_EQ(monoBehaviours.front(), mono);
    EXPECT_EQ(mono->getOwner(), &object);
}

TEST(GameObjectSceneInteraction, SceneCreatedObjectStartsAndUpdatesMonoBehaviour) {
    dzemikk::Scene scene;
    dzemikk::GameObject* object = scene.createGameObject();
    ASSERT_NE(object, nullptr);

    CallbackCounters counters;
    object->addComponent<CountingMonoBehaviour>(&counters);

    scene.update(1.0 / 60.0);

    EXPECT_EQ(counters.startCount, 1);
    EXPECT_EQ(counters.updateCount, 1);
    EXPECT_EQ(counters.lateUpdateCount, 1);
    EXPECT_EQ(counters.fixedUpdateCount, 0);

    scene.update(1.0 / 60.0);
    EXPECT_EQ(counters.startCount, 1);
    EXPECT_EQ(counters.updateCount, 2);
    EXPECT_EQ(counters.lateUpdateCount, 2);

    scene.fixedUpdate(1.0 / 60.0);
    EXPECT_EQ(counters.startCount, 1);
    EXPECT_EQ(counters.fixedUpdateCount, 1);
}

TEST(GameObjectSceneInteraction, SetSceneQueuesExistingMonoBehaviours) {
    dzemikk::GameObject object;
    CallbackCounters counters;

    object.addComponent<CountingMonoBehaviour>(&counters);

    dzemikk::Scene scene;
    object.setScene(&scene);
    scene.update(1.0 / 60.0);

    EXPECT_EQ(counters.startCount, 1);
    EXPECT_EQ(counters.updateCount, 1);
    EXPECT_EQ(counters.lateUpdateCount, 1);
}

TEST(GameObjectSceneInteraction, RemoveMonoBehaviourUnregistersFromSceneLoops) {
    dzemikk::Scene scene;
    dzemikk::GameObject* object = scene.createGameObject();
    ASSERT_NE(object, nullptr);

    CallbackCounters counters;
    auto* mono = object->addComponent<CountingMonoBehaviour>(&counters);
    ASSERT_NE(mono, nullptr);

    scene.update(1.0 / 60.0);
    ASSERT_EQ(counters.startCount, 1);
    ASSERT_EQ(counters.updateCount, 1);
    ASSERT_EQ(counters.lateUpdateCount, 1);

    object->removeComponent(mono);
    EXPECT_TRUE(object->getMonoBehaviours().empty());

    scene.update(1.0 / 60.0);
    scene.fixedUpdate(1.0 / 60.0);

    EXPECT_EQ(counters.startCount, 1);
    EXPECT_EQ(counters.updateCount, 1);
    EXPECT_EQ(counters.lateUpdateCount, 1);
    EXPECT_EQ(counters.fixedUpdateCount, 0);
}

TEST(GameObjectHierarchy, DetachChildAndDetachChildrenUnparentWithoutDestroy) {
    dzemikk::GameObject parent;
    dzemikk::GameObject childA;
    dzemikk::GameObject childB;

    parent.addChild(&childA);
    parent.addChild(&childB);
    ASSERT_EQ(parent.getChildren().size(), 2U);

    parent.detachChild(&childA);
    EXPECT_EQ(childA.getParent(), nullptr);
    ASSERT_EQ(parent.getChildren().size(), 1U);
    EXPECT_EQ(parent.getChildren().front(), &childB);

    parent.detachChildren();
    EXPECT_EQ(childB.getParent(), nullptr);
    EXPECT_TRUE(parent.getChildren().empty());
}

TEST(GameObjectHierarchy, DestroyChildIsDeferredAndCallsOnDestroyOnce) {
    dzemikk::Scene scene;
    dzemikk::GameObject* parent = scene.createGameObject();
    dzemikk::GameObject* child = scene.createGameObject();
    ASSERT_NE(parent, nullptr);
    ASSERT_NE(child, nullptr);

    parent->addChild(child);

    CallbackCounters counters;
    child->addComponent<CountingMonoBehaviour>(&counters);

    scene.update(1.0 / 60.0);
    ASSERT_EQ(counters.destroyCount, 0);

    parent->destroyChild(child);
    EXPECT_EQ(child->getParent(), nullptr);
    EXPECT_EQ(counters.destroyCount, 0);

    scene.update(1.0 / 60.0);
    EXPECT_EQ(counters.destroyCount, 1);
}

TEST(GameObjectHierarchy, DestroyChildrenDeduplicatesQueuedDestruction) {
    dzemikk::Scene scene;
    dzemikk::GameObject* parent = scene.createGameObject();
    dzemikk::GameObject* childA = scene.createGameObject();
    dzemikk::GameObject* childB = scene.createGameObject();
    ASSERT_NE(parent, nullptr);
    ASSERT_NE(childA, nullptr);
    ASSERT_NE(childB, nullptr);

    parent->addChild(childA);
    parent->addChild(childB);

    CallbackCounters countersA;
    CallbackCounters countersB;
    childA->addComponent<CountingMonoBehaviour>(&countersA);
    childB->addComponent<CountingMonoBehaviour>(&countersB);

    parent->destroyChildren();
    scene.destroyGameObject(childA); // duplicate request should be ignored

    scene.update(1.0 / 60.0);

    EXPECT_EQ(countersA.destroyCount, 1);
    EXPECT_EQ(countersB.destroyCount, 1);
}

TEST(GameObjectHierarchy, CyclicParentingIsRejected) {
    dzemikk::GameObject grandParent;
    dzemikk::GameObject parent;
    dzemikk::GameObject child;

    parent.setParent(&grandParent);
    child.setParent(&parent);

    grandParent.setParent(&child); // would create cycle, should be rejected

    EXPECT_EQ(grandParent.getParent(), nullptr);
    EXPECT_EQ(parent.getParent(), &grandParent);
    EXPECT_EQ(child.getParent(), &parent);
}

TEST(GameObjectSceneInteraction, DestroyIgnoresObjectsOutsideScene) {
    dzemikk::Scene scene;
    dzemikk::GameObject external;
    CallbackCounters counters;

    external.addComponent<CountingMonoBehaviour>(&counters);
    scene.destroyGameObject(&external);
    scene.update(1.0 / 60.0);

    EXPECT_EQ(counters.destroyCount, 0);
}

} // namespace