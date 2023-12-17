#include "Cone.h"

void Cone::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = fmax(param2, 3);
    setVertexData();
}


void Cone::makeWedge(float currentTheta, float nextTheta) {
    for (int i = 0; i < m_param1; i++) {
        // Calculate the vertices for the current tile.
        float theta1 = i;
        float theta2 = (i + 1);
        glm::vec3 bottomLeft = calculateVertex(currentTheta, theta2);
        glm::vec3 bottomRight = calculateVertex(nextTheta, theta2);
        glm::vec3 topLeft = calculateVertex(currentTheta, theta1);
        glm::vec3 topRight = calculateVertex(nextTheta, theta1);
        glm::vec3 newNormTopRight = glm::normalize(glm::vec3(topRight[0], 0.f, topRight[2]));
        glm::vec3 newNormTopLeft = glm::normalize(glm::vec3(topLeft[0], 0.f, topLeft[2]));
        glm::vec3 newNormBottomLeft = glm::normalize(glm::vec3(bottomLeft[0], 0.f, bottomLeft[2]));
        glm::vec3 newNormBottomRight = glm::normalize(glm::vec3(bottomRight[0], 0.f, bottomRight[2]));
        if (i != m_param1 - 1) {
            insertVec3(m_vertexData, topLeft);
            insertVec3(m_vertexData, glm::normalize(glm::vec3(newNormTopLeft[0], 0.5f, newNormTopLeft[2])));
            insertVec3(m_vertexData, bottomRight);
            insertVec3(m_vertexData, glm::normalize(glm::vec3(newNormBottomRight[0],  0.5f, newNormBottomRight[2])));
            insertVec3(m_vertexData, topRight);
            insertVec3(m_vertexData, glm::normalize(glm::vec3(newNormTopRight[0], 0.5f, newNormTopRight[2])));

            insertVec3(m_vertexData, topLeft);
            insertVec3(m_vertexData, glm::normalize(glm::vec3(newNormTopLeft[0], 0.5f, newNormTopLeft[2])));
            insertVec3(m_vertexData, bottomLeft);
            insertVec3(m_vertexData, glm::normalize(glm::vec3(newNormBottomLeft[0],  0.5f, newNormBottomLeft[2])));
            insertVec3(m_vertexData, bottomRight);
            insertVec3(m_vertexData, glm::normalize(glm::vec3(newNormBottomRight[0],  0.5f, newNormBottomRight[2])));
        } else {
            // tip (element-wise average of the two normals at the base of the triangle)
            glm::vec3 tipNormal = (glm::normalize(glm::vec3(newNormTopRight[0], 0.5f, newNormTopRight[2])) + glm::normalize(glm::vec3(newNormTopLeft[0], 0.5f, newNormTopLeft[2]))) * 0.5f;
            glm::vec3 topCenter = calculateVertex(currentTheta + (nextTheta - currentTheta) * 0.5f, m_param1);
            insertVec3(m_vertexData, topCenter);
            insertVec3(m_vertexData, tipNormal);
            insertVec3(m_vertexData, topRight);
            insertVec3(m_vertexData, glm::normalize(glm::vec3(newNormTopRight[0], 0.5f, newNormTopRight[2])));
            insertVec3(m_vertexData, topLeft);
            insertVec3(m_vertexData, glm::normalize(glm::vec3(newNormTopLeft[0], 0.5f, newNormTopLeft[2])));
        }
    }
}


void Cone::makeCone() {
    float thetaStep = glm::radians(360.f / static_cast<float>(m_param2));
    // float halfThetaStep = thetaStep / 2.0f;
    // Create the base of the cone
    glm::vec3 center = glm::vec3(0.0f, -0.5f, 0.0f);
    for (int col = 0; col < m_param2; col++) {
        float theta1 = col * glm::radians(360.f / static_cast<float>(m_param2));
        float theta2 = (col + 1) * glm::radians(360.f / static_cast<float>(m_param2));
        glm::vec3 vertex1 = calculateVertex(theta1, 0);
        glm::vec3 vertex2 = calculateVertex(theta2, 0);
        // Calculate the normal for the bottom tile
        glm::vec3 normal = glm::vec3(0.0f, -1.0f, 0.0f);
        // Create a bottom tile for the cap using the calculated vertices
        insertVec3(m_vertexData, vertex1);
        insertVec3(m_vertexData, normal);
        insertVec3(m_vertexData, vertex2);
        insertVec3(m_vertexData, normal);
        insertVec3(m_vertexData, center);
        insertVec3(m_vertexData, normal);
    }
    // Create the sides of the cone
    for (int j = 0; j < m_param2; j++) {
        float theta1 = j * thetaStep;
        float theta2 = (j + 1) * thetaStep;
        makeWedge(theta1, theta2);
    }

}


glm::vec3 Cone::calculateVertex(float theta, int row) {
    // Calculate the position of a vertex on the cone based on theta and row
    theta = theta + 90.0f * (M_PI / 180.0f);
    float radius = 0.5f - (static_cast<float>(row) / m_param1) * 0.5f;
    float x = radius * glm::cos(theta);
    float y = -0.5f + static_cast<float>(row) / m_param1;
    float z = radius * glm::sin(theta);
    return glm::vec3(x, y, z);
}

void Cone::setVertexData() {
    makeCone();
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cone::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
