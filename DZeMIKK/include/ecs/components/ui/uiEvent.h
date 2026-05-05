#ifndef DZEMIKK_UIEVENT_H
#define DZEMIKK_UIEVENT_H

#include <cstdint>
#include <variant>

namespace dzemikk {
class Component;

enum class UIEventType : uint8_t {
    Click,
    Hover,
    Enter,
    Exit,
    ValueChanged,
};

struct UIEvent {
    UIEvent(UIEventType type, Component* sender, std::variant<std::monostate, float, bool> payload)
        : type(type), sender(sender), payload(payload) {}

    UIEventType type;
    Component* sender;
    std::variant<std::monostate, float, bool> payload;
};
} // namespace dzemikk

#endif // DZEMIKK_UIEVENT_H
