#ifndef DZEMIKK_GAMEOBJECT_H
#define DZEMIKK_GAMEOBJECT_H

#include "ecs/component.h"
#include "ecs/components/transform.h"

#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace dzemikk {
    class MonoBehaviour;
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
            const Transform* transform() const;

            // --- Component operations
            template <typename T> T* getComponent() {
                for (const auto& component : _components) {
                    T* result = dynamic_cast<T*>(component.get());
                    if (result) {
                        return result;
                    }
                }
                return nullptr;
            }

            template <typename T, typename... Args> T* addComponent(Args&&... args) {
                auto component = std::make_unique<T>(std::forward<Args>(args)...);
                T* result = component.get();
                if constexpr (std::is_base_of_v<MonoBehaviour, T>) {
                    // For monobehaviours, set owner to this GameObject and cache the pointer for quick access
                    _monoBehaviours.push_back(result);
                    result->setOwner(this);
                }
                _components.push_back(std::move(component));
                return result;
            }

            template <typename T> void removeComponent(T* component) {
                if (!component) { return; }

                auto iter = std::ranges::find_if(_components.begin(), _components.end(), [component](const std::unique_ptr<Component>& comp) {
                    return comp.get() == component;
                });
                if (iter != _components.end()) {
                    if constexpr (std::is_base_of_v<MonoBehaviour, T>) {
                        // Remove from monobehaviour cache if necessary
                        auto monoIter = std::ranges::find(_monoBehaviours, component);
                        if (monoIter != _monoBehaviours.end()) {
                            _monoBehaviours.erase(monoIter);
                        }
                    }
                    _components.erase(iter);
                }
            }

            // -- Getters
            [[nodiscard]] GameObject* getParent() const;
            [[nodiscard]] const std::vector<GameObject*>& getChildren() const;
            [[nodiscard]] std::string getName() const;
            [[nodiscard]] const std::vector<MonoBehaviour*>& getMonoBehaviours() const;

            // --- Setters
            void setName(const std::string& name);

            // --- Hierarchy operations
            void setParent(GameObject* parent); // TODO: Cycle detection
            void addChild(GameObject* child);
            void removeChild(GameObject* child);

        private:
            std::string _name;
            Transform _transform;
            GameObject* _parent = nullptr;
            std::vector<GameObject*> _children;

            std::vector<std::unique_ptr<Component>> _components;
            std::vector<MonoBehaviour*> _monoBehaviours;
    };
} // namespace dzemikk

#endif // DZEMIKK_GAMEOBJECT_H
