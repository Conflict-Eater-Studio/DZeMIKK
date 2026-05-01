#ifndef DZEMIKK_INPUT_H
#define DZEMIKK_INPUT_H

#include "core/iEngineModule.h"
#include "events/event.h"
#include "events/mouse_event.h"
#include "events/key_event.h"
#include <glm/glm.hpp>
#include <vector>
#include <functional>
#include <algorithm>

struct GLFWwindow;

namespace dzemikk {

    using ListenerID = uint32_t;

    template <typename T>
    class Action {
    public:
        ListenerID addListener(std::function<void(T&)> listener) {
            ListenerID id = ++m_nextId;
            m_listeners.push_back({id, listener});
            return id;
        }

        void removeListener(ListenerID id) {
            m_listeners.erase(
                std::remove_if(m_listeners.begin(), m_listeners.end(), 
                    [id](const auto& pair) { return pair.first == id; }),
                m_listeners.end()
            );
        }

        void Invoke(T& e) {
            for (auto& l : m_listeners) l.second(e);
        }
    private:
        ListenerID m_nextId = 0;
        std::vector<std::pair<ListenerID, std::function<void(T&)>>> m_listeners;
    };

    class Input : public IEngineModule {
    public:
        Input() = default;
        ~Input() override = default;

        void initialize() override;
        void uninitialize() override;

        void setInputWindow(GLFWwindow* window);
        
        void OnEvent(Event& e);

        Action<dzemikk::MouseMovedEvent> OnMouseMoved;
        Action<dzemikk::MouseScrolledEvent> OnMouseScrolled;
        Action<dzemikk::MouseButtonPressedEvent> OnMouseButtonPressed;
        Action<dzemikk::MouseButtonReleasedEvent> OnMouseButtonReleased;
        Action<dzemikk::KeyPressedEvent> OnKeyPressed;
        Action<dzemikk::KeyReleasedEvent> OnKeyReleased;

        bool IsKeyPressed(int keycode) const;
        bool IsMouseButtonPressed(int button) const;
        glm::vec2 GetMousePosition() const;
        float GetMouseX() const;
        float GetMouseY() const;

    private:
        GLFWwindow* _windowContext = nullptr;
    };

}

#endif // DZEMIKK_INPUT_H
