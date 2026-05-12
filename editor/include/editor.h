#pragma once

#include "core/engine.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"

#include <vector>

namespace editor {

class Editor {
  public:
    explicit Editor(dzemikk::Engine* engine);

    void start();

  private:
    dzemikk::Engine* _engine = nullptr;
};

} // namespace editor