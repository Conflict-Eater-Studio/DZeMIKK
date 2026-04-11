#ifndef DZEMIKK_PROPERTY_H
#define DZEMIKK_PROPERTY_H
#include <functional>

template<typename T>
class Property {
public:
    using Getter = std::function<T()>;
    using Setter = std::function<void(const T&)>;

    Property() = default;

    Property(Getter g, Setter s)
        : _get(std::move(g)), _set(std::move(s)) {}

    void set(const T& value) {
        if (_set) _set(value);
    }

    T get() const {
        return _get ? _get() : T{};
    }

    explicit operator bool() const {
        return _get && _set;
    }

private:
    Getter _get;
    Setter _set;
};
#endif
