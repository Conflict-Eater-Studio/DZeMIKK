#include "inspectors/cameraInspector.h"

#include "ecs/components/camera.h"
#include "ui/propertyDrawer.h"

#include <imgui.h>

void editor::CameraInspector::draw(dzemikk::Camera* camera, const InspectorContext& ctx) {
    if (!camera) {
        return;
    }

    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {

        bool changed = false;

        auto type = camera->getProjectionType();
        int currentType = static_cast<int>(type);

        const char* types[] = {"Perspective", "Orthographic"};

        if (ImGui::Combo("Projection Type", &currentType, types, IM_ARRAYSIZE(types))) {

            type = static_cast<dzemikk::Camera::ProjectionType>(currentType);
            changed = true;
        }

        float nearPlane = camera->getNear();
        float farPlane = camera->getFar();

        if (PropertyDrawer::drawFloat("Near", nearPlane)) {
            changed = true;
        }

        if (PropertyDrawer::drawFloat("Far", farPlane)) {
            changed = true;
        }

        if (type == dzemikk::Camera::ProjectionType::Perspective) {

            float fov = camera->getFov();
            float aspect = camera->getAspect();

            if (PropertyDrawer::drawFloat("FOV", fov)) {
                changed = true;
            }

            if (PropertyDrawer::drawFloat("Aspect", aspect)) {
                changed = true;
            }
        }

        if (type == dzemikk::Camera::ProjectionType::Orthographic) {

            float left = camera->getLeft();
            float right = camera->getRightOrtographic();
            float bottom = camera->getBottom();
            float top = camera->getTop();

            if (PropertyDrawer::drawFloat("Left", left)) {
                changed = true;
            }

            if (PropertyDrawer::drawFloat("Right", right)) {
                changed = true;
            }

            if (PropertyDrawer::drawFloat("Bottom", bottom)) {
                changed = true;
            }

            if (PropertyDrawer::drawFloat("Top", top)) {
                changed = true;
            }
        }

        if (changed) {
            camera->markDirty();
        }
    }
}