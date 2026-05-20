#ifndef EDITOR_ANIMATOR_INSPECTOR_H
#define EDITOR_ANIMATOR_INSPECTOR_H

namespace dzemikk {
class Animator;
}

namespace editor {

struct InspectorContext;

class AnimatorInspector {
  public:
    static void draw(dzemikk::Animator* animator, const InspectorContext& ctx);
};

} // namespace editor

#endif