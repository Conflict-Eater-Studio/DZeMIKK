#ifndef DZeMIKK_COMPONENT_H
#define DZeMIKK_COMPONENT_H

namespace dzemikk {
    class Component {
        public:
            Component() = default;
            Component(const Component& other) = delete;
            Component(Component&& other) noexcept = delete;
            Component& operator=(const Component& other) = delete;
            Component& operator=(Component&& other) noexcept = delete;
            virtual ~Component() = default;
    };
}

#endif // DZeMIKK_COMPONENT_H
