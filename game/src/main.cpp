#include "core/engine.h"
#include "ecs/components/meshRenderer.h"
#include "ecs/components/spriteRenderer.h"
#include "ecs/components/transform.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "ecs/serialize/sceneSerializer.h"
#include "ecs/serialize/transformSerializer.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/renderer.h"
#include "renderer/shader.h"
#include "scripts/counter.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>

dzemikk::Mesh* createCubeMesh();
dzemikk::Mesh* createQuadMesh();

int main() {
    auto engine = std::make_shared<dzemikk::Engine>();

    dzemikk::ComponentSerializerRegistry::get().registerType<Counter>("Counter");
    dzemikk::ComponentSerializerRegistry::get().registerType<SubCounter>("SubCounter");

    // auto scene = std::make_unique<dzemikk::Scene>();
    // auto* obj = scene->createGameObject();
    // auto* counter = obj->addComponent<Counter>();
    // auto* subCounter = obj->addComponent<SubCounter>();
    // counter->subCounterRef.set(subCounter);

    // auto data = dzemikk::SceneSerializer::serialize(*scene);
    // std::ofstream outFile("scene.json");
    // outFile << data.dump(4);
    // outFile.close();

    std::ifstream inFile("scene.json");
    nlohmann::json jsonData;
    inFile >> jsonData;
    inFile.close();

    auto scene = std::make_unique<dzemikk::Scene>();
    dzemikk::SceneSerializer::deserializeInto(*scene, jsonData);

    for (int i = 0; i < 120; ++i) {
        scene->update(1.0 / 60.0);
    }

    return 0;
}