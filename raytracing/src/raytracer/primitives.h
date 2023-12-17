#pragma once

#include <glm/glm.hpp>

class Primitives {
public:
    struct IntersectData {
        float t;          // Intersection distance along the ray
        glm::vec4 normal; // Surface normal at the intersection point
        glm::vec4 intersectPoint;
    };

    static IntersectData intersectCube(glm::vec4 eye, glm::vec4 dir, glm::mat4 invCTM);
    static IntersectData intersectCone(glm::vec4 eye, glm::vec4 dir, glm::mat4 invCTM);
    static IntersectData intersectSphere(glm::vec4 eye, glm::vec4 dir, glm::mat4 invCTM);
    static IntersectData intersectCylinder(glm::vec4 eye, glm::vec4 dir, glm::mat4 invCTM);
};
