#include "inspectorRegistry.h"
#ifndef EDITOR_UICHECKBOXINSPECTOR_H
#define EDITOR_UICHECKBOXINSPECTOR_H

namespace dzemikk {
class UICheckbox;
}

namespace editor {

class UICheckboxInspector {
  public:
    static void draw(dzemikk::UICheckbox* checkbox, const InspectorContext& ctx);
};

} // namespace editor

#endif