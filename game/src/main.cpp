#include <memory>
#include <core/engine.h>

int main() {
    auto engine = std::make_shared<dzemikk::Engine>();
    engine->update();
}