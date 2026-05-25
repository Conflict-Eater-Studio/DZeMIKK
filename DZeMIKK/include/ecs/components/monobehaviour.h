#ifndef DZEMIKK_MONOBEHAVIOUR_H
#define DZEMIKK_MONOBEHAVIOUR_H

#include "../component.h"
#include "ecs/serialize/serializedRef.h"

#include <functional>
#include <vector>

namespace dzemikk {
class GameObject;
class MonoBehaviour : public Component, public ISerializedRefRegistrar {
  public:
    using Base = Component;

    MonoBehaviour() = default;
    MonoBehaviour(const MonoBehaviour& other) = delete;
    MonoBehaviour& operator=(const MonoBehaviour& other) = delete;
    MonoBehaviour(MonoBehaviour&& other) noexcept = delete;
    MonoBehaviour& operator=(MonoBehaviour&& other) noexcept = delete;
    ~MonoBehaviour() override = default;

    virtual void start() {};
    virtual void update(double deltaTime) {};
    virtual void lateUpdate(double deltaTime) {};
    virtual void fixedUpdate(double deltaTime) {};
    virtual void onDestroy() {};

    // --- Start state management
    [[nodiscard]] bool hasStarted() const;
    void markStarted();

    // --- Serialization
    using GameObjectResolverFn = std::function<GameObject*(const boost::uuids::uuid&)>;

    // Called after prefab instantiation, before runtime UUID reshuffle.
    // Scripts resolve UUID-backed references into raw pointers here.
    virtual void resolveSerializedReferences(const GameObjectResolverFn& gameObjectResolver,
                                             const ComponentResolverFn& componentResolver) {
        (void)gameObjectResolver;
        for (SerializedRefBase* ref : _serializedRefs) {
            if (ref != nullptr) {
                ref->resolve(componentResolver);
            }
        }
    }

    void registerSerializedReference(SerializedRefBase* reference) override {
        if (reference != nullptr) {
            _serializedRefs.push_back(reference);
        }
    }

  private:
    bool _started = false;
    std::vector<SerializedRefBase*> _serializedRefs;
};
} // namespace dzemikk

#endif // DZEMIKK_MONOBEHAVIOUR_H
