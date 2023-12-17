#include <stdexcept>
#include "camera.h"

Camera::Camera(const SceneCameraData& metaData, int width, int height)
    : sceneCameraMeta(metaData), sceneWidth(width), sceneHeight(height) {
    // Ensure that sceneWidth and sceneHeight are properly initialized.
}

glm::mat4 Camera::getViewMatrix() const {
    glm::vec4 pos = sceneCameraMeta.pos;
    glm::vec3 up = glm::vec3(sceneCameraMeta.up);
    glm::vec4 look = sceneCameraMeta.look;
    glm::vec3 w = -glm::normalize(look);
    glm::vec3 v = glm::normalize(up - glm::dot(up, w) * w);
    glm::vec3 u = glm::cross(v, w);

    glm::mat4 m_translate = glm::mat4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        -pos.x, -pos.y, -pos.z, 1.0f
        );

    glm::mat4 m_rotate = glm::mat4(
        u.x, v.x, w.x, 0.0f,
        u.y, v.y, w.y, 0.0f,
        u.z, v.z, w.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
        );

    return m_rotate * m_translate;
}

float Camera::getAspectRatio() const {
    return static_cast<float>(sceneWidth) / static_cast<float>(sceneHeight);
}

float Camera::getHeightAngle() const {
    return sceneCameraMeta.heightAngle;
}

float Camera::getFocalLength() const {
    return sceneCameraMeta.focalLength;
}

float Camera::getAperture() const {
    return sceneCameraMeta.aperture;
}

glm::vec4 Camera::getPos() const {
    return sceneCameraMeta.pos;
}
