#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <assetManager/assetmanager.h>

namespace dzemikk {
class Component;
}

namespace editor {

struct InspectorContext {
    dzemikk::AssetManager* assetManager = nullptr;
};

class InspectorRegistry {
  public:
    using DrawFunction = std::function<void(dzemikk::Component*, const InspectorContext&)>;

    void registerInspector(const std::string& type, DrawFunction function);

    void drawInspector(dzemikk::Component* component, const InspectorContext& context);

  private:
    std::unordered_map<std::string, DrawFunction> _inspectors;
};

} // namespace editor