#ifndef DZEMIKK_GAMEOBJECT_H
#define DZEMIKK_GAMEOBJECT_H

#include <memory>
#include <vector>
#include "component.h"
#include "components/transform.h"

namespace dzemikk {
    class GameObject {
        public:
            GameObject();
            GameObject(const GameObject& other) = delete;
            GameObject& operator=(const GameObject& other) = delete;
            GameObject(GameObject&& other) noexcept = delete;
            GameObject& operator=(GameObject&& other) noexcept = delete;
            ~GameObject() = default;

            // ---
            Transform* transform();

            // --- Component operations
            template<typename T>
            T* getComponent() {
                for (const auto& component : _components) {
                    T* result = dynamic_cast<T*>(component.get());
                    if (result) return result;
                }
                return nullptr;
            }

        private:
            Transform _transform;

            std::vector<std::unique_ptr<Component>> _components;
    };
}

#endif // DZEMIKK_GAMEOBJECT_H
