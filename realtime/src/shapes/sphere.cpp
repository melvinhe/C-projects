#include "Sphere.h"

void Sphere::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = fmax(param1, 2);
    m_param2 = fmax(param2, 3);
    setVertexData();
}

void Sphere::makeTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight) {
    glm::vec3 normal1 = glm::normalize(topLeft);
    glm::vec3 normal2 = glm::normalize(topRight);
    glm::vec3 normal3 = glm::normalize(bottomLeft);
    glm::vec3 normal4 = glm::normalize(bottomRight);
    // Insert the vertices and normals for the first triangle
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normal1);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, normal3);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normal4);
    // Insert the vertices and normals for the second triangle
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normal1);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normal4);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, normal2);
}

void Sphere::makeWedge(float currentTheta, float nextTheta) {
    float phiStep = glm::radians(180.f / static_cast<float>(m_param1));
    for (int i = 0; i < m_param1; i++) {
        float x1 = 0.5f * sin(i * phiStep) * sin(currentTheta);
        float y1 = 0.5f * cos(i * phiStep);
        float z1 = 0.5f * sin(i * phiStep) * cos(currentTheta);
        glm::vec3 topLeft = glm::vec3(x1, y1, z1);
        float x2 = 0.5f * sin(i * phiStep) * sin(nextTheta);
        float y2 = 0.5f * cos(i * phiStep);
        float z2 = 0.5f * sin(i * phiStep) * cos(nextTheta);
        glm::vec3 topRight = glm::vec3(x2, y2, z2);
        float x3 = 0.5f * sin((i+1) * phiStep) * sin(currentTheta);
        float y3 = 0.5f * cos((i+1) * phiStep);
        float z3 = 0.5f * sin((i+1) * phiStep) * cos(currentTheta);
        glm::vec3 bottomLeft = glm::vec3(x3, y3, z3);
        float x4 = 0.5f * sin((i+1) * phiStep) * sin(nextTheta);
        float y4 = 0.5f * cos((i+1) * phiStep);
        float z4 = 0.5f * sin((i+1) * phiStep) * cos(nextTheta);
        glm::vec3 bottomRight = glm::vec3(x4, y4, z4);
        makeTile(topLeft, topRight, bottomLeft, bottomRight);
    }
}

glm::vec3 Sphere::calculateVertex(float theta, float phi) {
    float radius = 0.5;  // Set the desired radius here
    float x = radius * glm::sin(phi) * glm::cos(theta);
    float y = radius * glm::sin(phi) * glm::sin(theta);
    float z = radius * glm::cos(phi);
    return glm::vec3(x, y, z);
}


void Sphere::makeSphere() {
    float thetaStep = glm::radians(360.f / static_cast<float>(m_param2));
    float currentTheta = 0;
    // Create wedges around the sphere.
    for (int i = 0; i < m_param2; i++) {
        float nextTheta = currentTheta + thetaStep;
        makeWedge(currentTheta, nextTheta);
        currentTheta = nextTheta;
    }
}

void Sphere::setVertexData() {
    makeSphere();
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Sphere::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
