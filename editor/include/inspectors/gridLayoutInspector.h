#ifndef EDITOR_GRIDLAYOUTINSPECTOR_H
#define EDITOR_GRIDLAYOUTINSPECTOR_H

namespace dzemikk {
class GridLayout;
}

namespace editor {

struct InspectorContext;

class GridLayoutInspector {
  public:
    static void draw(dzemikk::GridLayout* layout, const InspectorContext& ctx);
};

} // namespace editor

#endif