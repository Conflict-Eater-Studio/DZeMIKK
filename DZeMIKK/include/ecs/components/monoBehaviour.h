#ifndef DZEMIKK_MONOBEHAVIOUR_H
#define DZEMIKK_MONOBEHAVIOUR_H

#include "../component.h"

namespace dzemikk {
    class GameObject;
    class MonoBehaviour : public Component {
        public:
            MonoBehaviour() = default;
            MonoBehaviour(const MonoBehaviour& other) = delete;
            MonoBehaviour& operator=(const MonoBehaviour& other) = delete;
            MonoBehaviour(MonoBehaviour&& other) noexcept = delete;
            MonoBehaviour& operator=(MonoBehaviour&& other) noexcept = delete;
            ~MonoBehaviour() override = default;

            virtual void start() {};
            virtual void update(double deltaTime) {};
            virtual void lateUpdate() {};
            virtual void fixedUpdate(double deltaTime) {};
            virtual void onDestroy() {};

            // --- Start state management
            [[nodiscard]] bool hasStarted() const;
            void markStarted();
        private:
            bool _started = false;
    };
}

#endif // DZEMIKK_MONOBEHAVIOUR_H
