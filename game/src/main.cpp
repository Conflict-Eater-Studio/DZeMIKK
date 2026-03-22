#include <memory>
#include <core/engine.h>
#include <renderer/renderer.h>

int main() {
    auto engine = std::make_shared<dzemikk::Engine>();
    engine->update();

    auto renderer = std::make_shared<dzemikk::Renderer>();
}