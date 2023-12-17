#include <stdexcept>
#include "camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

Camera::Camera(SceneCameraData& metaData, int width, int height)
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

glm::mat4 Camera::getProjMatrix(float nearPlane, float farPlane) const {
    // used for testing:
    // return glm::perspective(getHeightAngle(), getAspectRatio(), nearPlane, farPlane);
    // return projMatrix;
    // recalculating here to try to debug:
    float heightAngle = getHeightAngle();
    float tanHalfFOV = tan(heightAngle / 2.0f);
    float aspectRatio = getAspectRatio();
    float frustumLength = farPlane - nearPlane;
    float yScale = 1.0f / tanHalfFOV;
    float xScale = yScale / aspectRatio;

    return glm::mat4(xScale, 0, 0, 0,
                     0, yScale, 0, 0,
                     0, 0, - (nearPlane + farPlane) / (frustumLength), - 1,
                     0, 0, -2.0f * farPlane * nearPlane / frustumLength, 0);
}


void Camera::updatePlane(float nearPlane, float farPlane) {
    float heightAngle = getHeightAngle();
    float tanHalfFOV = tan(heightAngle / 2.0f);
    float aspectRatio = getAspectRatio();
    float frustumLength = farPlane - nearPlane;
    float yScale = 1.0f / tanHalfFOV;
    float xScale = yScale / aspectRatio;

    glm::mat4 projectionMatrix = glm::mat4(xScale, 0, 0, 0,
                                           0, yScale, 0, 0,
                                           0, 0, - (nearPlane + farPlane) / (frustumLength), - 1,
                                           0, 0, -2.0f * farPlane * nearPlane / frustumLength, 0);
    this->projMatrix = projectionMatrix;
}

void Camera::resize(int width, int height) {
    sceneWidth = width;
    sceneHeight = height;
}



void Camera::translate(const glm::vec3& direction) {
    glm::mat4 translationMatrix = glm::mat4(1.0f);
    translationMatrix[3][0] = direction.x;
    translationMatrix[3][1] = direction.y;
    translationMatrix[3][2] = direction.z;

    // Update the camera's position
    sceneCameraMeta.pos = translationMatrix * sceneCameraMeta.pos;
}


void Camera::rotateHorizontal(float angle) {
    glm::vec3 axis = glm::normalize(glm::vec3(0.f, 1.f, 0.f));
    glm::mat4 rotationMatrix = glm::mat4(1.0f);
    float c = cos(angle);
    float s = sin(angle);
    float C = 1 - c;

    rotationMatrix[0][0] = c + axis.x * axis.x * C;
    rotationMatrix[1][0] = axis.x * axis.y * C - axis.z * s;
    rotationMatrix[2][0] = axis.x * axis.z * C + axis.y * s;

    rotationMatrix[0][1] = axis.y * axis.x * C + axis.z * s;
    rotationMatrix[1][1] = c + axis.y * axis.y * C;
    rotationMatrix[2][1] = axis.y * axis.z * C - axis.x * s;

    rotationMatrix[0][2] = axis.z * axis.x * C - axis.y * s;
    rotationMatrix[1][2] = axis.z * axis.y * C + axis.x * s;
    rotationMatrix[2][2] = c + axis.z * axis.z * C;

    // Update the camera's look direction
    glm::vec4 newLook = rotationMatrix * sceneCameraMeta.look;
    sceneCameraMeta.look = glm::vec4(glm::normalize(glm::vec3(newLook)), newLook.w);
}

void Camera::rotateVertical(float angle) {
    // Calculate perpendicular axis using sceneCameraMeta.look and sceneCameraMeta.up
    glm::vec3 look = glm::vec3(sceneCameraMeta.look);
    glm::vec3 up = glm::vec3(sceneCameraMeta.up);

    glm::vec3 axis = glm::normalize(glm::vec3(
        look.y * up.z - look.z * up.y,
        look.z * up.x - look.x * up.z,
        look.x * up.y - look.y * up.x
        ));

    glm::mat4 rotationMatrix = glm::mat4(1.0f);
    float c = cos(angle);
    float s = sin(angle);
    float C = 1 - c;

    rotationMatrix[0][0] = c + axis.x * axis.x * C;
    rotationMatrix[1][0] = axis.x * axis.y * C - axis.z * s;
    rotationMatrix[2][0] = axis.x * axis.z * C + axis.y * s;

    rotationMatrix[0][1] = axis.y * axis.x * C + axis.z * s;
    rotationMatrix[1][1] = c + axis.y * axis.y * C;
    rotationMatrix[2][1] = axis.y * axis.z * C - axis.x * s;

    rotationMatrix[0][2] = axis.z * axis.x * C - axis.y * s;
    rotationMatrix[1][2] = axis.z * axis.y * C + axis.x * s;
    rotationMatrix[2][2] = c + axis.z * axis.z * C;

    // Update the camera's look and up
    glm::vec4 newLook = rotationMatrix * sceneCameraMeta.look;
    glm::vec4 newUp = rotationMatrix * sceneCameraMeta.up;
    sceneCameraMeta.look = glm::vec4(glm::normalize(glm::vec3(newLook)), newLook.w);
    sceneCameraMeta.up = glm::vec4(glm::normalize(glm::vec3(newUp)), newUp.w);

}




glm::vec3 Camera::getForwardVector() const {
    return glm::normalize(glm::vec3(sceneCameraMeta.look));
}

glm::vec3 Camera::getRightVector() const {
    glm::vec3 up = glm::vec3(sceneCameraMeta.up);
    glm::vec3 look = -glm::vec3(sceneCameraMeta.look); // Negative look vector
    return glm::normalize(glm::cross(up, look));
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
