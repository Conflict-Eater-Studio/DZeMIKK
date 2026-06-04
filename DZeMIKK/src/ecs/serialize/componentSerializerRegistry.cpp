#include "ecs/serialize/componentSerializerRegistry.h"

#include "ecs/gameobject.h"
#include "ecs/serialize/animation/animatorSerializer.h"
#include "ecs/serialize/cameraSerializer.h"
#include "ecs/serialize/colliderSerializer.h"
#include "ecs/serialize/meshRendererSerializer.h"
#include "ecs/serialize/rectTransformSerializer.h"
#include "ecs/serialize/skinnedMeshSerializer.h"
#include "ecs/serialize/spriteRendererSerializer.h"
#include "ecs/serialize/textRendererSerializer.h"
#include "ecs/serialize/transformSerializer.h"
#include "ecs/serialize/ui/canvasSerializer.h"
#include "ecs/serialize/ui/gridLayoutSerializer.h"
#include "ecs/serialize/ui/horizontalLayoutSerializer.h"
#include "ecs/serialize/ui/imageRendererSerializer.h"
#include "ecs/serialize/ui/uiButtonSerializer.h"
#include "ecs/serialize/ui/uiCheckboxSerializer.h"
#include "ecs/serialize/ui/uiSliderSerializer.h"
#include "ecs/serialize/ui/uiDropdownSerializer.h"
#include "ecs/serialize/directionalLightSerializer.h"
#include "ecs/serialize/pointLightSerializer.h"
#include "ecs/serialize/spotLightSerializer.h"
#include "ecs/serialize/ui/UITextRendererSerializer.h"
#include "ecs/serialize/ui/verticalLayoutSerializer.h"

#include <stdexcept>
#include <utility>

namespace dzemikk {
namespace {
ComponentSerializerRegistry buildDefaultRegistry() {
    ComponentSerializerRegistry registry;

    registerTransformSerializer(registry);
    registerCanvasSerializer(registry);
    registerRectTransformSerializer(registry);
    registerUIButtonSerializer(registry);
    registerUISliderSerializer(registry);
    registerMeshRendererSerializer(registry);
    registerTextRendererSerializer(registry);
    registerSpriteRendererSerializer(registry);
    registerSkinnedMeshRendererSerializer(registry);
    registerCameraSerializer(registry);
    registerColliderSerializer(registry);
    registerRectTransformSerializer(registry);
    registerUICheckboxSerializer(registry);
    registerAnimatorSerializer(registry);
    registerDirectionalLightSerializer(registry);
    registerPointLightSerializer(registry);
    registerSpotLightSerializer(registry);
    registerImageRendererSerializer(registry);
    registerGridLayoutSerializer(registry);
    registerHorizontalLayoutSerializer(registry);
    registerVerticalLayoutSerializer(registry);
    registerUITextRendererSerializer(registry);
    registerUIDropdownSerializer(registry);

    return registry;
}
} // namespace

ComponentSerializerRegistry& ComponentSerializerRegistry::get() {
    static ComponentSerializerRegistry kRegistry = buildDefaultRegistry();
    return kRegistry;
}

void ComponentSerializerRegistry::registerType(
    std::string typeName, SerializeFn serializeFn, DeserializeIntoGameObjectFn deserializeFn,
    PostDeserializeComponentFn postDeserializeComponentFn) {
    _entries[std::move(typeName)] =
        Entry{.serialize = std::move(serializeFn),
              .deserializeIntoGameObject = std::move(deserializeFn),
              .postDeserializeComponent = std::move(postDeserializeComponentFn)};
}

nlohmann::json ComponentSerializerRegistry::serialize(const Component& component) const {
    const auto iter = _entries.find(component.typeName());
    if (iter == _entries.end()) {
#if DZEMIKK_DEV_TOOLS
        spdlog::error("No serializer registered for component type: {}", component.typeName());
#endif
        throw std::runtime_error("No serializer registered for component type: " +
                                 component.typeName());
    }

    return iter->second.serialize(component);
}
void ComponentSerializerRegistry::deserializeIntoGameObject(
    const DeserializationContext& context) const {
    nlohmann::json json = context.json;
    if (!json.contains("type") || !json["type"].is_string()) {
#if DZEMIKK_DEV_TOOLS
        spdlog::error("Serialized component is missing string field 'type'");
#endif
        throw std::runtime_error("Serialized component is missing string field 'type'");
    }

    const std::string typeName = json["type"].get<std::string>();
    const auto iter = _entries.find(typeName);
    if (iter == _entries.end()) {
#if DZEMIKK_DEV_TOOLS
        spdlog::error("No deserializer registered for component type: {}", typeName);
#endif
        throw std::runtime_error("No deserializer registered for component type: " + typeName);
    }

    iter->second.deserializeIntoGameObject(context);
}

void ComponentSerializerRegistry::postDeserializeComponents(GameObject* obj) const {
    for (const auto& comp : obj->getAllComponents()) {
        if (comp == nullptr) {
            continue;
        }

        const auto iter = _entries.find(comp->typeName());
        if (iter != _entries.end()) {
            iter->second.postDeserializeComponent(*(comp.get()));
        }
    }
}
} // namespace dzemikk
