#include "inspectors/inspectorRegistry.h"
#include "editor.h"

#include <ecs/component.h>

void editor::InspectorRegistry::registerInspector(const std::string& type, DrawFunction function) {
    _inspectors[type] = std::move(function);
}

void editor::InspectorRegistry::drawInspector(dzemikk::Component* component, const InspectorContext& context) {
    if (!component) {
        return;
    }

    auto it = _inspectors.find(component->typeName());

    if (it == _inspectors.end()) {
        return;
    }

    it->second(component, context);
}