#ifndef DZEMIKK_SERIALIZEDREF_H
#define DZEMIKK_SERIALIZEDREF_H

#pragma once

#include "ecs/component.h"

#include <functional>

namespace dzemikk {
using ComponentResolverFn = std::function<Component*(const boost::uuids::uuid&)>;

class SerializedRefBase {
  public:
    SerializedRefBase() = default;
    SerializedRefBase(const SerializedRefBase& other) = delete;
    SerializedRefBase& operator=(const SerializedRefBase& other) = delete;
    SerializedRefBase(SerializedRefBase&& other) noexcept = delete;
    SerializedRefBase& operator=(SerializedRefBase&& other) noexcept = delete;
    virtual ~SerializedRefBase() = default;

    virtual void resolve(const ComponentResolverFn& componentResolver) = 0;
};

class ISerializedRefRegistrar {
  public:
    ISerializedRefRegistrar() = default;
    ISerializedRefRegistrar(const ISerializedRefRegistrar& other) = delete;
    ISerializedRefRegistrar& operator=(const ISerializedRefRegistrar& other) = delete;
    ISerializedRefRegistrar(ISerializedRefRegistrar&& other) noexcept = delete;
    ISerializedRefRegistrar& operator=(ISerializedRefRegistrar&& other) noexcept = delete;
    virtual ~ISerializedRefRegistrar() = default;

    virtual void registerSerializedReference(SerializedRefBase* reference) = 0;
};

template <typename T> class SerializedRef : public SerializedRefBase {
  public:
    explicit SerializedRef(ISerializedRefRegistrar& registrar) {
        registrar.registerSerializedReference(this);
    }

    SerializedRef(const SerializedRef& other) = delete;
    SerializedRef& operator=(const SerializedRef& other) = delete;
    SerializedRef(SerializedRef&& other) noexcept = delete;
    SerializedRef& operator=(SerializedRef&& other) noexcept = delete;
    ~SerializedRef() override = default;

    [[nodiscard]] T* get() const {
        return _value;
    }

    T* operator->() const {
        return _value;
    }

    operator T*() const {
        return _value;
    }

    void set(T* value) {
        _value = value;
        _pendingId = {};
    }

    // Called by deserialization to store unresolved component ID.
    void captureSerializedId(const boost::uuids::uuid& uuidValue) {
        _pendingId = uuidValue;
        if (uuidValue.is_nil()) {
            _value = nullptr;
        }
    }

    // Called by serialization. If unresolved, returns pending ID.
    [[nodiscard]] boost::uuids::uuid toSerializedId() const {
        if (_value != nullptr) {
            return _value->getId();
        }
        return _pendingId;
    }

    void clear() {
        _value = nullptr;
        _pendingId = {};
    }

    void resolve(const ComponentResolverFn& componentResolver) override {
        if (_pendingId.is_nil()) {
            return;
        }

        Component* resolvedComponent = componentResolver(_pendingId);
        _value = dynamic_cast<T*>(resolvedComponent);
        _pendingId = {};
    }

  private:
    T* _value = nullptr;
    boost::uuids::uuid _pendingId;
};
} // namespace dzemikk

#endif // DZEMIKK_SERIALIZEDREF_H
