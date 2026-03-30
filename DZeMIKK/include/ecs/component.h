#ifndef DZEMIKK_COMPONENT_H
#define DZEMIKK_COMPONENT_H

namespace dzemikk {
class GameObject;
class Scene;
class Component {
  public:
    Component() = default;
    Component(const Component& other) = delete;
    Component(Component&& other) noexcept = delete;
    Component& operator=(const Component& other) = delete;
    Component& operator=(Component&& other) noexcept = delete;
    virtual ~Component() = default;

  protected:
    GameObject* _owner = nullptr;

  public:
    /*
     * @brief Sets the owner GameObject of this component.
     *
     * @param owner The GameObject that owns this component.
     */
    void setOwner(GameObject* owner);

    // --- Getters
    /*
     * @brief Gets the owner GameObject of this component.
     */
    [[nodiscard]] GameObject* getOwner() const;
};
} // namespace dzemikk

#endif // DZEMIKK_COMPONENT_H