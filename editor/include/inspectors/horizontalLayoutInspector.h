#ifndef EDITOR_HORIZONTALLAYOUTINSPECTOR_H
#define EDITOR_HORIZONTALLAYOUTINSPECTOR_H

namespace dzemikk {
class HorizontalLayout;
}

namespace editor {

struct InspectorContext;

class HorizontalLayoutInspector {
  public:
    static void draw(dzemikk::HorizontalLayout* layout, const InspectorContext& ctx);
};

} // namespace editor

#endif