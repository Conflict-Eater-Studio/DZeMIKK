#include "inspectors/inspectorRegistry.h"

#include "ecs/component.h"

using namespace editor;
using namespace dzemikk;

void InspectorRegistry::registerInspector(const std::string& type, DrawFunction function) {
    _inspectors[type] = std::move(function);
}

void InspectorRegistry::drawInspector(Component* component, const InspectorContext& context) {

    if (!component) {
        return;
    }

    auto it = _inspectors.find(component->typeName());

    if (it == _inspectors.end()) {
        return;
    }

    it->second(component, context);
}