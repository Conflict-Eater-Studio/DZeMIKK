#ifndef EDITOR_IMAGERENDERERINSPECTOR_H
#define EDITOR_IMAGERENDERERINSPECTOR_H

namespace dzemikk {
class ImageRenderer;
}

namespace editor {

struct InspectorContext;

class ImageRendererInspector {
  public:
    static void draw(dzemikk::ImageRenderer* renderer, const InspectorContext& ctx);
};

} // namespace editor

#endif