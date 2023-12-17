#include "Cube.h"

void Cube::updateParams(int param1) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    setVertexData();
}

void Cube::makeTile(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {
    // Calculate the normals for each triangle
    glm::vec3 normal1 = glm::normalize(glm::cross(bottomLeft - topLeft, bottomRight - topLeft));
    glm::vec3 normal2 = glm::normalize(glm::cross(bottomRight - bottomLeft, topLeft - bottomLeft));

    // Insert the vertices and normals for the first triangle
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normal1);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, normal1);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normal1);
    // Insert the vertices and normals for the second triangle
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normal2);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normal2);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, normal2);
}

void Cube::makeFace(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {

    //Modify so that makeFace works for any angle (such as side faces of a cube)
    glm::vec3 normal = glm::normalize(glm::cross(topRight - topLeft, bottomLeft - topLeft));

    // Calculate the tile width and height
    glm::vec3 tileWidthVector = (topRight - topLeft) / static_cast<float>(m_param1);
    glm::vec3 tileHeightVector = (bottomLeft - topLeft) / static_cast<float>(m_param1);

    // Loop through rows and columns to create the tiles
    for (int row = 0; row < m_param1; row++) {
        for (int col = 0; col < m_param1; col++) {
            // Calculate the vertices of the current tile
            glm::vec3 currentTopLeft = topLeft + static_cast<float>(col) * tileWidthVector + static_cast<float>(row) * tileHeightVector;
            glm::vec3 currentTopRight = currentTopLeft + tileWidthVector;
            glm::vec3 currentBottomLeft = currentTopLeft + tileHeightVector;
            glm::vec3 currentBottomRight = currentTopRight + tileHeightVector;

            // Create a tile using makeTile()
            makeTile(currentTopLeft, currentTopRight, currentBottomLeft, currentBottomRight);
        }
    }
}

void Cube::setVertexData() {

    makeFace(glm::vec3(-0.5f,  0.5f, 0.5f), // topLeft
             glm::vec3( 0.5f,  0.5f, 0.5f), // topRight
             glm::vec3(-0.5f, -0.5f, 0.5f), // bottomLeft
             glm::vec3( 0.5f, -0.5f, 0.5f)); // bottomRight

    // Back face
    makeFace(glm::vec3(0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f, 0.5f, -0.5f),
             glm::vec3(0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f));

    // Left face (doesn't display)
    makeFace(glm::vec3(-0.5f, -0.5f, 0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, 0.5f, 0.5f),
             glm::vec3(-0.5f, 0.5f, -0.5f));
    // This Left face displays
    makeFace(glm::vec3(-0.5f, -0.5f, 0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, 0.5f, 0.5f),
             glm::vec3(-0.5f, 0.5f, -0.5f));
    // Right face
    makeFace(glm::vec3(0.5f, 0.5f, 0.5f),
             glm::vec3(0.5f, 0.5f, -0.5f),
             glm::vec3(0.5f, -0.5f, 0.5f),
             glm::vec3(0.5f, -0.5f, -0.5f));
    // Top face
    makeFace(glm::vec3(0.5f, 0.5f, -0.5f),
             glm::vec3(0.5f, 0.5f, 0.5f),
             glm::vec3(-0.5f, 0.5f, -0.5f),
             glm::vec3(-0.5f, 0.5f, 0.5f));
    // Bottom face
    makeFace(glm::vec3(-0.5f, -0.5f, 0.5f),
             glm::vec3(0.5f, -0.5f, 0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3(0.5f, -0.5f, -0.5f));
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cube::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
