#ifndef DZEMIKK_CAMERA_SYSTEM_H
#define DZEMIKK_CAMERA_SYSTEM_H

#include <vector>
#include "renderer/renderPasses/IRenderPass.h"

namespace dzemikk {
class Camera;
class CameraSystem {
  public:
#pragma region Camera Management

    /**
     * @brief Sets the active scene camera for 3D rendering.
     */
    void setActiveSceneCamera(Camera* camera);

    /**
     * @brief Sets the active UI camera for overlay rendering.
     */
    void setActiveUICamera(Camera* camera);

    /**
     * @brief Finds and sets the active scene camera by ID.
     */
    void setActiveSceneCameraById(int cameraId);

    /**
     * @brief Finds and sets the active UI camera by ID.
     */
    void setActiveUICameraById(int cameraId);

    /**
     * @brief Returns the currently active scene camera.
     */
    [[nodiscard]] const Camera* getActiveSceneCamera() const;

    /**
     * @brief Returns the currently active UI camera.
     */
    [[nodiscard]] const Camera* getActiveUICamera() const;

#pragma endregion

    void update(RenderContext& ctx);

  private:
    std::vector<Camera*> _cameras;
    Camera* _sceneCamera;
    Camera* _uiCamera;   

};
} // namespace dzemikk
#endif // DZEMIKK_CAMERA_SYSTEM_H