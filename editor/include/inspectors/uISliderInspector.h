#ifndef EDITOR_UISLIDERINSPECTOR_H
#define EDITOR_UISLIDERINSPECTOR_H

#include "inspectorRegistry.h"

namespace dzemikk {
class UISlider;
}

namespace editor {

class UISliderInspector {
  public:
    static void draw(dzemikk::UISlider* slider, const InspectorContext& ctx);
};

} // namespace editor

#endif