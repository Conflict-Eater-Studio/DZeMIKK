#ifndef DZEMIKK_COMPONENT_H
#define DZEMIKK_COMPONENT_H

#pragma once
#include <boost/uuid.hpp>

namespace dzemikk {
class GameObject;
class Scene;
class Component {
  public:
    Component();
    Component(const boost::uuids::uuid& uuid);
    Component(const Component& other) = delete;
    Component(Component&& other) noexcept = delete;
    Component& operator=(const Component& other) = delete;
    Component& operator=(Component&& other) noexcept = delete;
    virtual ~Component() = default;

    /**
     * @brief Gets the type name of this component.
     */
    [[nodiscard]] virtual std::string typeName() const = 0;

    /**
     * @brief Sets the owner GameObject of this component.
     *
     * @param owner The GameObject that owns this component.
     */
    void setOwner(GameObject* owner);

    /**
     * @brief Sets the unique identifier of this component.
     *
     * @param uuid The unique identifier for this component.
     */
    void setId(const boost::uuids::uuid& uuid);

    /**
     * @brief Enables or disables this component.
     *
     * @param enabled If true, the component is enabled; if false, it is disabled.
     */
    void enabled(bool enabled);

    // --- Getters
    /**
     * @brief Gets the owner GameObject of this component.
     */
    [[nodiscard]] GameObject* getOwner() const;

    /**
     * @brief Gets the unique identifier of this component.
     */
    [[nodiscard]] boost::uuids::uuid getId() const;

    /**
     * @brief Checks if this component is enabled.
     *
     * @return True if the component is enabled, false otherwise.
     */
    [[nodiscard]] bool isEnabled() const;

  protected:
    boost::uuids::uuid _id;
    GameObject* _owner = nullptr;
    bool _isEnabled = true;
};
} // namespace dzemikk

#endif // DZEMIKK_COMPONENT_H
