#ifndef DZEMIKK_SCENE_H
#define DZEMIKK_SCENE_H

#include <memory>
#include <vector>

namespace dzemikk {
    class GameObject;
    class Scene {
        public:
            Scene() = default;
            Scene(const Scene& other) = delete;
            Scene& operator=(const Scene& other) = delete;
            Scene(Scene&& other) noexcept = delete;
            Scene& operator=(Scene&& other) noexcept = delete;
            ~Scene() = default;

            GameObject* createGameObject();
            void destroyGameObject(GameObject* object);
            void update(double deltaTime);
            void fixedUpdate(double deltaTime);

        private:
            std::vector<std::unique_ptr<dzemikk::GameObject>> _objects;
    };
}

#endif // DZEMIKK_SCENE_H
