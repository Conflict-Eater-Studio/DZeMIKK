#include "core/engine.h"
#include "editor.h"

#include <memory>
#include <tuple>

int main() {
    auto engine = std::make_unique<dzemikk::Engine>(dzemikk::EngineMode::Editor);

    auto editor = new editor::Editor(engine.get());
    editor->start();

    // engine->start();
    delete (editor);
    return 0;
}
