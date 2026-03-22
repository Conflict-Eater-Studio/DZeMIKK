#include "ecs/components/transform.h"
#include <memory>
#include <core/engine.h>
#include <ecs/gameobject.h>
#include <spdlog/spdlog.h>

int main() {
    auto engine = std::make_shared<dzemikk::Engine>();
    dzemikk::GameObject obj;
    spdlog::info("Hello, world!");
    auto* trs = obj.transform();
    spdlog::info("Position: {}, {}, {}", trs->getPosition()[0], trs->getPosition()[1], trs->getPosition()[2]);
    engine->update();
}
