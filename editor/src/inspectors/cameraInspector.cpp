#include "inspectors/cameraInspector.h"

#include "ecs/components/camera.h"
#include "ui/propertyDrawer.h"

#include <imgui.h>

void editor::CameraInspector::draw(dzemikk::Camera* camera, const InspectorContext& ctx) {
    if (!camera) {
        return;
    }

    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {

        auto type = camera->getProjectionType();

        const char* types[] = {"Perspective", "Orthographic"};

        if (PropertyDrawer::drawEnum("Projection Type", type, types, IM_ARRAYSIZE(types))) {

            camera->setProjectionType(type);
        }

        float nearPlane = camera->getNear();

        if (PropertyDrawer::drawFloat("Near", nearPlane)) {
            camera->setNear(nearPlane);
        }

        float farPlane = camera->getFar();

        if (PropertyDrawer::drawFloat("Far", farPlane)) {
            camera->setFar(farPlane);
        }

        if (type == dzemikk::Camera::ProjectionType::Perspective) {

            float fov = camera->getFov();

            if (PropertyDrawer::drawFloat("FOV", fov)) {
                camera->setFov(fov);
            }

            float aspect = camera->getAspect();

            if (PropertyDrawer::drawFloat("Aspect", aspect)) {
                camera->setAspect(aspect);
            }
        }

        if (type == dzemikk::Camera::ProjectionType::Orthographic) {

            float left = camera->getLeft();

            if (PropertyDrawer::drawFloat("Left", left)) {
                camera->setLeft(left);
            }

            float right = camera->getRightOrtographic();

            if (PropertyDrawer::drawFloat("Right", right)) {
                camera->setRight(right);
            }

            float bottom = camera->getBottom();

            if (PropertyDrawer::drawFloat("Bottom", bottom)) {
                camera->setBottom(bottom);
            }

            float top = camera->getTop();

            if (PropertyDrawer::drawFloat("Top", top)) {
                camera->setTop(top);
            }
        }
    }
}