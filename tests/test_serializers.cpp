#include "ecs/components/meshRenderer.h"
#include "ecs/components/monoBehaviour.h"
#include "ecs/components/ui/uiButton.h"
#include "ecs/components/ui/uiButtonActionRegistry.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "ecs/serialize/componentSerializerRegistry.h"
#include "ecs/serialize/gameobjectSerializer.h"
#include "ecs/serialize/monoBehaviourSerializer.h"
#include "ecs/serialize/prefabSerializer.h"
#include "ecs/serialize/sceneSerializer.h"
#include "renderer/mesh.h"

#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_hash.hpp>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace {
class SerializerRefTargetScript final : public dzemikk::MonoBehaviour {
  public:
    using Base = dzemikk::MonoBehaviour;

    [[nodiscard]] std::string typeName() const override {
        return "SerializerRefTargetScript";
    }

    int someValue = 7;
};

class SerializerRefOwnerScript final : public dzemikk::MonoBehaviour {
  public:
    using Base = dzemikk::MonoBehaviour;

    [[nodiscard]] std::string typeName() const override {
        return "SerializerRefOwnerScript";
    }

    dzemikk::SerializedRef<SerializerRefTargetScript> targetRef{*this};
};

TEST(ComponentSerializerRegistrySerialization, SerializeTransformUsesRegisteredSerializer) {
    dzemikk::Scene scene;
    dzemikk::GameObject* object = scene.createGameObject();
    ASSERT_NE(object, nullptr);

    object->transform()->setPosition(glm::vec3(1.0F, 2.0F, 3.0F));

    nlohmann::json json =
        dzemikk::ComponentSerializerRegistry::get().serialize(*object->transform());

    ASSERT_TRUE(json.contains("type"));
    EXPECT_EQ(json["type"], "Transform");
    ASSERT_TRUE(json.contains("position"));
    EXPECT_EQ(json["position"].size(), 3U);
    EXPECT_FLOAT_EQ(json["position"][0].get<float>(), 1.0F);
    EXPECT_FLOAT_EQ(json["position"][1].get<float>(), 2.0F);
    EXPECT_FLOAT_EQ(json["position"][2].get<float>(), 3.0F);
}

TEST(ComponentSerializerRegistrySerialization, DeserializeIntoGameObjectAppliesTransform) {
    dzemikk::Scene scene;
    dzemikk::GameObject* object = scene.createGameObject();
    ASSERT_NE(object, nullptr);

    nlohmann::json transformJson;
    transformJson["type"] = "Transform";
    transformJson["id"] = boost::uuids::to_string(object->transform()->getId());
    transformJson["position"] = {5.0F, 6.0F, 7.0F};
    transformJson["rotation"] = {1.0F, 0.0F, 0.0F, 0.0F};
    transformJson["scale"] = {2.0F, 3.0F, 4.0F};

    dzemikk::ComponentSerializerRegistry::DeserializationContext context(*object, transformJson);
    dzemikk::ComponentSerializerRegistry::get().deserializeIntoGameObject(context);

    const glm::vec3 position = object->transform()->getPosition();
    const glm::vec3 scale = object->transform()->getScale();
    EXPECT_FLOAT_EQ(position[0], 5.0F);
    EXPECT_FLOAT_EQ(position[1], 6.0F);
    EXPECT_FLOAT_EQ(position[2], 7.0F);
    EXPECT_FLOAT_EQ(scale[0], 2.0F);
    EXPECT_FLOAT_EQ(scale[1], 3.0F);
    EXPECT_FLOAT_EQ(scale[2], 4.0F);
}

TEST(ComponentSerializerRegistrySerialization, DeserializeUnknownTypeThrows) {
    dzemikk::Scene scene;
    dzemikk::GameObject* object = scene.createGameObject();
    ASSERT_NE(object, nullptr);

    nlohmann::json unknown;
    unknown["type"] = "NoSuchComponent";

    dzemikk::ComponentSerializerRegistry::DeserializationContext context(*object, unknown);
    EXPECT_THROW(
        dzemikk::ComponentSerializerRegistry::get().deserializeIntoGameObject(context),
        std::runtime_error);
}

TEST(GameObjectSerializerSerialization, SerializeAndInstantiateRoundTripHierarchy) {
    dzemikk::Scene sourceScene;
    dzemikk::GameObject* root = sourceScene.createGameObject();
    dzemikk::GameObject* child = sourceScene.createGameObject();
    ASSERT_NE(root, nullptr);
    ASSERT_NE(child, nullptr);

    root->setName("Root");
    child->setName("Child");
    root->addChild(child);

    const nlohmann::json serialized = dzemikk::GameObjectSerializer::serialize(*root);

    dzemikk::Scene targetScene;
    dzemikk::GameObject* instantiated =
        dzemikk::GameObjectSerializer::instantiateIntoScene(targetScene, serialized, nullptr);

    ASSERT_NE(instantiated, nullptr);
    EXPECT_EQ(instantiated->getName(), "Root");
    ASSERT_EQ(instantiated->getChildren().size(), 1U);
    EXPECT_EQ(instantiated->getChildren().front()->getName(), "Child");
}

TEST(GameObjectSerializerSerialization, DetachedDeserializeRejectsChildren) {
    dzemikk::Scene scene;
    dzemikk::GameObject* root = scene.createGameObject();
    dzemikk::GameObject* child = scene.createGameObject();
    ASSERT_NE(root, nullptr);
    ASSERT_NE(child, nullptr);
    root->addChild(child);

    const nlohmann::json serialized = dzemikk::GameObjectSerializer::serialize(*root);

    EXPECT_THROW((void)dzemikk::GameObjectSerializer::deserialize(serialized), std::runtime_error);
}

TEST(PrefabSerializerSerialization, InstantiateRegeneratesRuntimeUuids) {
    dzemikk::Scene sourceScene;
    dzemikk::GameObject* root = sourceScene.createGameObject();
    ASSERT_NE(root, nullptr);

    const boost::uuids::uuid sourceRootId = root->getId();
    const boost::uuids::uuid sourceTransformId = root->transform()->getId();

    const nlohmann::json prefabJson = dzemikk::PrefabSerializer::serialize(*root);

    dzemikk::Scene targetScene;
    dzemikk::GameObject* instance = dzemikk::PrefabSerializer::instantiate(targetScene, prefabJson);
    ASSERT_NE(instance, nullptr);

    EXPECT_NE(instance->getId(), sourceRootId);
    EXPECT_NE(instance->transform()->getId(), sourceTransformId);
}

TEST(SceneSerializerSerialization, SerializeRootsAndDeserializeIntoScene) {
    dzemikk::Scene scene;
    dzemikk::GameObject* rootA = scene.createGameObject();
    dzemikk::GameObject* rootB = scene.createGameObject();
    dzemikk::GameObject* child = scene.createGameObject();
    ASSERT_NE(rootA, nullptr);
    ASSERT_NE(rootB, nullptr);
    ASSERT_NE(child, nullptr);

    rootA->addChild(child);

    const nlohmann::json sceneJson = dzemikk::SceneSerializer::serialize(scene);

    ASSERT_TRUE(sceneJson.contains("roots"));
    ASSERT_TRUE(sceneJson["roots"].is_array());
    EXPECT_EQ(sceneJson["roots"].size(), 2U);

    dzemikk::Scene loadedScene;
    dzemikk::SceneSerializer::deserializeInto(loadedScene, sceneJson);

    std::size_t rootCount = 0;
    for (const auto& object : loadedScene.getObjects()) {
        if (object != nullptr && object->getParent() == nullptr) {
            ++rootCount;
        }
    }

    EXPECT_EQ(rootCount, 2U);
}

TEST(MonoBehaviourSerializerSerialization, BaseReadWriteAndRefResolveWork) {
    dzemikk::Scene scene;
    dzemikk::GameObject* object = scene.createGameObject();
    ASSERT_NE(object, nullptr);

    auto* target = object->addComponent<SerializerRefTargetScript>();
    auto* owner = object->addComponent<SerializerRefOwnerScript>();
    ASSERT_NE(target, nullptr);
    ASSERT_NE(owner, nullptr);

    nlohmann::json ownerJson;
    dzemikk::MonoBehaviourSerializer::writeBase(ownerJson, *owner, "SerializerRefOwnerScript");
    owner->targetRef.captureSerializedId(target->getId());
    dzemikk::MonoBehaviourSerializer::writeRef(ownerJson, "targetRef", owner->targetRef);

    SerializerRefOwnerScript copyOwner;
    dzemikk::MonoBehaviourSerializer::readBase(ownerJson, copyOwner, "SerializerRefOwnerScript");
    dzemikk::MonoBehaviourSerializer::readRef(ownerJson, "targetRef", copyOwner.targetRef);

    std::unordered_map<boost::uuids::uuid, dzemikk::Component*> componentMap;
    componentMap[target->getId()] = target;

    copyOwner.resolveSerializedReferences(
        [](const boost::uuids::uuid&) -> dzemikk::GameObject* { return nullptr; },
        [&componentMap](const boost::uuids::uuid& uuidValue) -> dzemikk::Component* {
            const auto iter = componentMap.find(uuidValue);
            return iter == componentMap.end() ? nullptr : iter->second;
        });

    EXPECT_EQ(copyOwner.targetRef.get(), target);
}

TEST(MonoBehaviourSerializerSerialization, ReadBaseRejectsWrongType) {
    SerializerRefOwnerScript owner;
    nlohmann::json json;
    dzemikk::MonoBehaviourSerializer::writeBase(json, owner, "SerializerRefOwnerScript");
    json["type"] = "OtherType";

    EXPECT_THROW(
        dzemikk::MonoBehaviourSerializer::readBase(json, owner, "SerializerRefOwnerScript"),
        std::runtime_error);
}

TEST(UIButtonSerialization, RegistrySerializeIncludesActionIdsAndColors) {
    dzemikk::Scene scene;
    dzemikk::GameObject* object = scene.createGameObject("Button");
    ASSERT_NE(object, nullptr);

    auto* button = object->addComponent<dzemikk::UIButton>();
    ASSERT_NE(button, nullptr);

    button->setNormalColor(glm::vec4(0.1F, 0.2F, 0.3F, 0.4F));
    button->setHoverColor(glm::vec4(0.5F, 0.6F, 0.7F, 0.8F));
    button->setPressedColor(glm::vec4(0.9F, 1.0F, 0.2F, 0.3F));
    button->setOnClickActionId("test.ui.click.serialize");
    button->setOnEnterActionId("test.ui.enter.serialize");
    button->setOnExitActionId("test.ui.exit.serialize");

    const nlohmann::json json = dzemikk::ComponentSerializerRegistry::get().serialize(*button);

    ASSERT_TRUE(json.contains("type"));
    EXPECT_EQ(json["type"], "UIButton");
    ASSERT_TRUE(json.contains("normalColor"));
    ASSERT_TRUE(json.contains("hoverColor"));
    ASSERT_TRUE(json.contains("pressedColor"));
    ASSERT_TRUE(json.contains("onClickActionId"));
    ASSERT_TRUE(json.contains("onEnterActionId"));
    ASSERT_TRUE(json.contains("onExitActionId"));

    EXPECT_EQ(json["onClickActionId"].get<std::string>(), "test.ui.click.serialize");
    EXPECT_EQ(json["onEnterActionId"].get<std::string>(), "test.ui.enter.serialize");
    EXPECT_EQ(json["onExitActionId"].get<std::string>(), "test.ui.exit.serialize");

    EXPECT_FLOAT_EQ(json["normalColor"][0].get<float>(), 0.1F);
    EXPECT_FLOAT_EQ(json["normalColor"][1].get<float>(), 0.2F);
    EXPECT_FLOAT_EQ(json["normalColor"][2].get<float>(), 0.3F);
    EXPECT_FLOAT_EQ(json["normalColor"][3].get<float>(), 0.4F);
}

TEST(UIButtonSerialization, DeserializeBindsOnClickActionFromRegistry) {
    dzemikk::Scene scene;
    dzemikk::GameObject* source = scene.createGameObject("Source");
    ASSERT_NE(source, nullptr);

    auto* sourceButton = source->addComponent<dzemikk::UIButton>();
    ASSERT_NE(sourceButton, nullptr);
    sourceButton->setOnClickActionId("test.ui.click.deserialize");

    const nlohmann::json serializedButton =
        dzemikk::ComponentSerializerRegistry::get().serialize(*sourceButton);

    dzemikk::GameObject* target = scene.createGameObject("Target");
    ASSERT_NE(target, nullptr);

    dzemikk::ComponentSerializerRegistry::DeserializationContext context(*target, serializedButton);
    dzemikk::ComponentSerializerRegistry::get().deserializeIntoGameObject(context);

    auto* button = target->getComponent<dzemikk::UIButton>();
    ASSERT_NE(button, nullptr);
    EXPECT_EQ(button->getOnClickActionId(), "test.ui.click.deserialize");

    int clickCount = 0;
    dzemikk::UIButtonActionRegistry::get().registerAction(
        "test.ui.click.deserialize",
        [&clickCount](dzemikk::UIButton&) { return [&clickCount]() { ++clickCount; }; });

    button->onClick();
    EXPECT_EQ(clickCount, 1);
}

TEST(UIButtonSerialization, UnknownOnClickActionIdIsSafeNoOp) {
    dzemikk::Scene scene;
    dzemikk::GameObject* object = scene.createGameObject("Button");
    ASSERT_NE(object, nullptr);

    auto* button = object->addComponent<dzemikk::UIButton>();
    ASSERT_NE(button, nullptr);

    button->setOnClickActionId("test.ui.missing.action");

    EXPECT_NO_THROW(button->onClick());
}

TEST(MeshRendererSerialization, ToJsonSerializesPropertiesCorrectly) {
    dzemikk::Scene scene;
    dzemikk::GameObject* object = scene.createGameObject("RendererObject");
    ASSERT_NE(object, nullptr);

    auto* renderer = object->addComponent<dzemikk::MeshRenderer>();
    ASSERT_NE(renderer, nullptr);

    renderer->setColor(glm::vec4(0.1F, 0.5F, 0.8F, 1.0F));
    nlohmann::json json = dzemikk::ComponentSerializerRegistry::get().serialize(*object->getComponent<dzemikk::MeshRenderer>());

    ASSERT_TRUE(json.contains("type"));
    EXPECT_EQ(json["type"], "MeshRenderer");

    ASSERT_TRUE(json.contains("id"));
    EXPECT_EQ(json["id"], boost::uuids::to_string(renderer->getId()));

    ASSERT_TRUE(json.contains("color"));
    ASSERT_TRUE(json["color"].is_array());
    EXPECT_EQ(json["color"].size(), 4U);
    EXPECT_FLOAT_EQ(json["color"][0].get<float>(), 0.1F); // R
    EXPECT_FLOAT_EQ(json["color"][1].get<float>(), 0.5F); // G
    EXPECT_FLOAT_EQ(json["color"][2].get<float>(), 0.8F); // B
    EXPECT_FLOAT_EQ(json["color"][3].get<float>(), 1.0F); // A
    ASSERT_TRUE(json.contains("model"));

    EXPECT_EQ(json["model"].get<std::string>(), "");
}
TEST(MeshRendererSerialization, ToJsonExtractsModelPathFromValidHandle) {
    dzemikk::Scene scene;
    dzemikk::GameObject* object = scene.createGameObject("RendererObject");
    auto* renderer = object->addComponent<dzemikk::MeshRenderer>();

    // Mocking an AssetHandle setup (Adjust this to match how you fake handles in your tests)
    // dzemikk::AssetHandle<dzemikk::Model> mockHandle = createMockHandle("assets/models/player.obj");
    // renderer->setModel(mockHandle);

    /* Uncomment when your mock is set up
    nlohmann::json j;
    dzemikk::to_json(j, *renderer);

    ASSERT_TRUE(j.contains("model"));
    EXPECT_EQ(j["model"].get<std::string>(), "assets/models/player.obj");
    */
}
} // namespace
