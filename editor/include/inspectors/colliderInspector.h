#ifndef EDITOR_COLLIDERINSPECTOR_H
#define EDITOR_COLLIDERINSPECTOR_H

namespace dzemikk {
class Collider;
}

namespace editor {
struct InspectorContext;
class ColliderInspector {
  public:
    static void draw(dzemikk::Collider* collider, const InspectorContext& ctx);
};

} // namespace editor

#endif // EDITOR_COLLIDERINSPECTOR_H