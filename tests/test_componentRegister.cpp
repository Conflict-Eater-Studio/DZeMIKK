#include "ecs/component.h"
#include "ecs/componentRegistry.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"

#include <gtest/gtest.h>
#include <memory>
#include <vector>

// Test component hierarchy for inheritance testing
class TestBaseComponent : public dzemikk::Component {
  public:
    using Base = dzemikk::Component;
    int value = 0;
};

class TestDerivedComponent : public TestBaseComponent {
  public:
    using Base = TestBaseComponent;
    int derivedValue = 0;
};

namespace {

TEST(ComponentRegistryTest, SingletonReturnsSameInstance) {
    auto& registry1 = dzemikk::ComponentRegistry::get();
    auto& registry2 = dzemikk::ComponentRegistry::get();

    EXPECT_EQ(&registry1, &registry2);
}

TEST(ComponentRegistryTest, RegisterGetAndUnregisterComponent) {
    auto scene = dzemikk::Scene();
    ASSERT_NE(&scene, nullptr);
    auto* obj = scene.createGameObject();
    ASSERT_NE(obj, nullptr);
    auto* component = obj->addComponent<TestBaseComponent>();
    ASSERT_NE(component, nullptr);
    component->value = 42;
    ASSERT_EQ(component->value, 42);

    // Test getComponents
    std::vector<TestBaseComponent*> components;
    dzemikk::ComponentRegistry::get().getComponents<TestBaseComponent>(components);
    EXPECT_EQ(components.size(), 1);
    EXPECT_EQ(components[0]->value, 42);

    // Test unregister
    dzemikk::ComponentRegistry::get().unregisterComponent(component);
    components.clear();

    dzemikk::ComponentRegistry::get().getComponents<TestBaseComponent>(components);
    EXPECT_TRUE(components.empty());
}

TEST(ComponentRegistryTest, InheritanceRegistration) {
    auto scene = dzemikk::Scene();
    auto* obj = scene.createGameObject();
    auto* derivedComponent = obj->addComponent<TestDerivedComponent>();
    derivedComponent->value = 10;
    derivedComponent->derivedValue = 20;

    // Test getComponents for base type
    std::vector<TestBaseComponent*> baseComponents;
    dzemikk::ComponentRegistry::get().getComponents<TestBaseComponent>(baseComponents);
    EXPECT_EQ(baseComponents.size(), 1);
    EXPECT_EQ(baseComponents[0]->value, 10);

    // Test getComponents for derived type
    std::vector<TestDerivedComponent*> derivedComponents;
    dzemikk::ComponentRegistry::get().getComponents<TestDerivedComponent>(derivedComponents);
    EXPECT_EQ(derivedComponents.size(), 1);
    EXPECT_EQ(derivedComponents[0]->value, 10);
    EXPECT_EQ(derivedComponents[0]->derivedValue, 20);

    obj->removeComponent(derivedComponent);
}

TEST(ComponentRegistryTest, NullPointerSafety) {
    auto& registry = dzemikk::ComponentRegistry::get();

    registry.registerComponent<TestBaseComponent>(nullptr);
    registry.unregisterComponent(nullptr);
}

TEST(ComponentRegistryTest, DuplicateRegistrationIdempotent) {
    auto component = std::make_unique<TestBaseComponent>();

    dzemikk::ComponentRegistry::get().registerComponent<TestBaseComponent>(component.get());
    dzemikk::ComponentRegistry::get().registerComponent<TestBaseComponent>(component.get());

    std::vector<TestBaseComponent*> components;
    dzemikk::ComponentRegistry::get().getComponents<TestBaseComponent>(components);
    ASSERT_EQ(components.size(), 1);

    dzemikk::ComponentRegistry::get().unregisterComponent(component.get());
}

TEST(ComponentRegistryTest, GetNonExistentComponentType) {
    auto baseComponent = std::make_unique<TestBaseComponent>();
    auto derivedComponent = std::make_unique<TestDerivedComponent>();

    dzemikk::ComponentRegistry::get().registerComponent<TestBaseComponent>(baseComponent.get());

    std::vector<TestDerivedComponent*> derivedComponents;
    dzemikk::ComponentRegistry::get().getComponents<TestDerivedComponent>(derivedComponents);
    EXPECT_TRUE(derivedComponents.empty());

    dzemikk::ComponentRegistry::get().unregisterComponent(baseComponent.get());
}
} // namespace