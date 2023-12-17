#include "Cylinder.h"

void Cylinder::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = fmax(param2, 3);
    setVertexData();
}

void Cylinder::makeTile(glm::vec3 topLeft, glm::vec3 topRight, glm::vec3 bottomLeft, glm::vec3 bottomRight) {
    // Insert the vertices and normals for the first triangle
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, glm::normalize(glm::vec3(topRight[0], 0, topRight[2])));
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, glm::normalize(glm::vec3(topLeft[0], 0, topLeft[2])));
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, glm::normalize(glm::vec3(topLeft[0], 0, topLeft[2])));
    // Insert the vertices and normals for the second triangle
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, glm::normalize(glm::vec3(topRight[0], 0, topRight[2])));
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, glm::normalize(glm::vec3(bottomLeft[0], 0, bottomLeft[2])));
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, glm::normalize(glm::vec3(bottomRight[0], 0, bottomRight[2])));
}



glm::vec3 Cylinder::calculateVertex(float theta, int row) {
    // Calculate the position of a vertex on the cylinder based on theta and row
    theta = theta + 90.0f * (M_PI / 180.0f);
    float radius = 0.5f;
    float x = radius * glm::cos(theta);
    float y = -0.5f + static_cast<float>(row) / m_param1;
    float z = radius * glm::sin(theta);
    return glm::vec3(x, y, z);
}

void Cylinder::makeCylinder() {
    float thetaStep = glm::radians(360.f / static_cast<float>(m_param2));
    // Create the sides of the cylinder
    for (int row = 0; row < m_param1; row++) {
        for (int col = 0; col < m_param2; col++) {
            float theta1 = col * thetaStep;
            float theta2 = (col + 1) * thetaStep;
            glm::vec3 topLeft = calculateVertex(theta1, row);
            glm::vec3 topRight = calculateVertex(theta2, row);
            glm::vec3 bottomLeft = calculateVertex(theta1, row + 1);
            glm::vec3 bottomRight = calculateVertex(theta2, row + 1);
            // Create a tile using makeTile()
            makeTile(topLeft, topRight, bottomLeft, bottomRight);
        }
    }
    // Create the top and bottom caps of the cylinder
    makeCap(true);  // Top cap
    makeCap(false); // Bottom cap
}

void Cylinder::makeCap(bool isTopCap) {
    // Create the top or bottom cap of the cylinder

    glm::vec3 center = isTopCap ? glm::vec3(0.0f, 0.5f, 0.0f) : glm::vec3(0.0f, -0.5f, 0.0f);
    for (int j = 0; j < m_param2; j++) {
        float theta1 = j * glm::radians(360.f / static_cast<float>(m_param2));
        float theta2 = (j + 1) * glm::radians(360.f / static_cast<float>(m_param2));
        glm::vec3 vertex1 = isTopCap ? calculateVertex(theta1, m_param1) : calculateVertex(theta1, 0);
        glm::vec3 vertex2 = isTopCap ? calculateVertex(theta2, m_param1) : calculateVertex(theta2, 0);
        // Calculate the normal for the cap
        glm::vec3 normal = glm::vec3(0.0f, isTopCap ? 1.0f : -1.0f, 0.0f);
        for (int i = 0; i < m_param1; i++) {
            // Create a tile for the cap using the calculated vertices
            if (isTopCap) {
                insertVec3(m_vertexData, center);
                insertVec3(m_vertexData, normal);
                insertVec3(m_vertexData, vertex2);
                insertVec3(m_vertexData, normal);
                insertVec3(m_vertexData, vertex1);
                insertVec3(m_vertexData, normal);
            } else {
                insertVec3(m_vertexData, vertex1);
                insertVec3(m_vertexData, normal);
                insertVec3(m_vertexData, vertex2);
                insertVec3(m_vertexData, normal);
                insertVec3(m_vertexData, center);
                insertVec3(m_vertexData, normal);
            }
        }
    }
}


void Cylinder::setVertexData() {
    makeCylinder();
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cylinder::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
