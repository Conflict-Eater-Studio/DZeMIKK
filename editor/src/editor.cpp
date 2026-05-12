#include "editor.h"

#if DZEMIKK_DEV_TOOLS

#include "ecs/components/camera.h"
#include "ecs/components/meshRenderer.h"
#include "ecs/components/transform.h"
#include "ecs/scenemanager.h"

#include <imgui.h>
#include <imgui_internal.h>

#endif

editor::Editor::Editor(dzemikk::Engine* engine) {
    _engine = engine;
}

void editor::Editor::start() {
   _engine->start();
}