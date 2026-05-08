#include "core/engine.h"

namespace editor {
class Editor {
  public:
    explicit Editor(dzemikk::Engine* engine);
    ~Editor() = default;

    void start();

  private:
    dzemikk::Engine* _engine;
};

}
