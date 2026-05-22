#include "inspectorRegistry.h"
#ifndef EDITOR_UIDROPDOWNINSPECTOR_H
#define EDITOR_UIDROPDOWNINSPECTOR_H

namespace dzemikk {
class UIDropdown;
}

namespace editor {

class UIDropdownInspector {
  public:
    static void draw(dzemikk::UIDropdown* dropdown, const InspectorContext& ctx);
};

} // namespace editor

#endif