#include "inspectors/cameraInspector.h"

#include <ecs/components/camera.h>
#include <ui/propertyDrawer.h>

#include <imgui.h>
#include <array>

void editor::CameraInspector::draw(dzemikk::Camera* camera, const InspectorContext& ctx) {
    if (!camera) {
        return;
    }

    if (!ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }

    auto type = camera->getProjectionType();

    const std::array<const char*, 2> types = {"Perspective", "Orthographic"};

    if (PropertyDrawer::drawEnum("Projection Type", type, types.data(),
                                 static_cast<int>(types.size()))) {

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
        drawPerspective(camera);
    } else {
        drawOrthographic(camera);
    }
}

void editor::CameraInspector::drawPerspective(dzemikk::Camera* camera) {
    float fov = camera->getFov();
    if (PropertyDrawer::drawFloat("FOV", fov))
        camera->setFov(fov);

    float aspect = camera->getAspect();
    if (PropertyDrawer::drawFloat("Aspect", aspect))
        camera->setAspect(aspect);
}

void editor::CameraInspector::drawOrthographic(dzemikk::Camera* camera) {
    float left = camera->getLeft();
    if (PropertyDrawer::drawFloat("Left", left))
        camera->setLeft(left);

    float right = camera->getRightOrtographic();
    if (PropertyDrawer::drawFloat("Right", right))
        camera->setRight(right);

    float bottom = camera->getBottom();
    if (PropertyDrawer::drawFloat("Bottom", bottom))
        camera->setBottom(bottom);

    float top = camera->getTop();
    if (PropertyDrawer::drawFloat("Top", top))
        camera->setTop(top);
}
