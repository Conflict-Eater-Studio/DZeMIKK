#ifndef EDITOR_VERTICALLAYOUTINSPECTOR_H
#define EDITOR_VERTICALLAYOUTINSPECTOR_H

namespace dzemikk {
class VerticalLayout;
}

namespace editor {

struct InspectorContext;

class VerticalLayoutInspector {
  public:
    static void draw(dzemikk::VerticalLayout* layout, const InspectorContext& ctx);
};

} // namespace editor

#endif