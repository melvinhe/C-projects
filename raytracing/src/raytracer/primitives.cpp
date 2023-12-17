#include "primitives.h"
#include <vector>
#include <limits>
#include <algorithm>
#include <cmath>

Primitives::IntersectData Primitives::intersectCube(glm::vec4 eye, glm::vec4 dir, glm::mat4 invCTM){
    // Transform the ray into the cube's local coordinates
    glm::vec4 localOrigin = invCTM * eye;
    float x = localOrigin[0];
    float y = localOrigin[1];
    float z = localOrigin[2];
    glm::vec4 localDirection = invCTM * dir;
    float a = localDirection[0];
    float b = localDirection[1];
    float c = localDirection[2];
    float t1, t2, t3, t4, t5, t6;
    t1 = (-0.5 - x) / a;
    t2 = (0.5 - x) / a;
    t3 = (-0.5 - y) / b;
    t4 = (0.5 - y) / b;
    t5 = (-0.5 - z) / c;
    t6 = (0.5 - z) / c;
    glm::vec4 intersectionPointX = glm::vec4{x+t1*a, y+t1*b, z+t1*c, 1.0};
    glm::vec4 intersectionPointX2 = glm::vec4{x+t2*a, y+t2*b, z+t2*c, 1.0};
    glm::vec4 intersectionPointY = glm::vec4{x+t3*a, y+t3*b, z+t3*c, 1.0};
    glm::vec4 intersectionPointY2 = glm::vec4{x+t4*a, y+t4*b, z+t4*c, 1.0};
    glm::vec4 intersectionPointZ = glm::vec4{x+t5*a, y+t5*b, z+t5*c, 1.0};
    glm::vec4 intersectionPointZ2 = glm::vec4{x+t6*a, y+t6*b, z+t6*c, 1.0};

    if(intersectionPointX[1] > 0.5 || intersectionPointX[1] < -0.5 || intersectionPointX[2] > 0.5 || intersectionPointX[2] < -0.5 || t1 < 0){
        t1 = FLT_MAX;
    }
    if(intersectionPointX2[1] > 0.5 || intersectionPointX2[1] < -0.5 || intersectionPointX2[2] > 0.5 || intersectionPointX2[2] < -0.5 || t2 < 0){
        t2 = FLT_MAX;
    }
    if(intersectionPointY[0] > 0.5 || intersectionPointY[0] < -0.5 || intersectionPointY[2] > 0.5 || intersectionPointY[2] < -0.5 || t3 < 0){
        t3 = FLT_MAX;
    }
    if(intersectionPointY2[0] > 0.5 || intersectionPointY2[0] < -0.5 || intersectionPointY2[2] > 0.5 || intersectionPointY2[2] < -0.5 || t4 < 0){
        t4 = FLT_MAX;
    }
    if(intersectionPointZ[0] > 0.5 || intersectionPointZ[0] < -0.5 || intersectionPointZ[1] > 0.5 || intersectionPointZ[1] < -0.5 || t5 < 0){
        t5 = FLT_MAX;
    }
    if(intersectionPointZ2[0] > 0.5 || intersectionPointZ2[0] < -0.5 || intersectionPointZ2[1] > 0.5 || intersectionPointZ2[1] < -0.5 || t6 < 0){
        t6 = FLT_MAX;
    }
    std::vector<float> tValues = {t1, t2, t3, t4, t5, t6};
    float minT = *std::min_element(tValues.begin(), tValues.end());
    if (minT == FLT_MAX) {
        return IntersectData{-1.f, glm::vec4{0, 0, 0, 0}};
    }
    minT = fmax(0.f, minT);
    if(minT == 0.f){
        minT = -1.f;
    }
    glm::vec4 normal = glm::vec4{0,0,0,0};
    if (minT == t1) {
        normal = glm::vec4{-1, 0, 0, 0};
    } else if (minT == t2) {
        normal = glm::vec4{1, 0, 0, 0};
    } else if (minT == t3) {
        normal = glm::vec4{0, -1, 0, 0};
    } else if (minT == t4) {
        normal = glm::vec4{0, 1, 0, 0};
    } else if (minT == t5) {
        normal = glm::vec4{0, 0, -1, 0};
    } else if (minT == t6) {
        normal = glm::vec4{0, 0, 1, 0};
    }
    glm::vec4 intersectionPoint = glm::vec4{x+minT*a, y+minT*b, z+minT*c, 1.0f};
    return IntersectData{minT, normal, intersectionPoint};
}


Primitives::IntersectData Primitives::intersectSphere(glm::vec4 eye, glm::vec4 dir, glm::mat4 invCTM) {
    // Transform the ray into the sphere's local coordinates
    glm::vec4 p = invCTM * eye;
    float px = p[0];
    float py = p[1];
    float pz = p[2];
    glm::vec4 d = invCTM * dir;
    float dx = d[0];
    float dy = d[1];
    float dz = d[2];
    float A = dx*dx + dy*dy + dz*dz;
    float B = 2.f*px*dx + 2.f*py*dy + 2.f*pz*dz;
    float C = px*px + py*py + pz*pz - 0.25;
    float determinant = B*B - 4.f * A * C;
    if (determinant < 0){
        return IntersectData{-1.f, glm::vec4{0,0,0,0}};
    }
    float positiveT = (-B + sqrt(determinant))/(2.f*A);
    float negativeT = (-B - sqrt(determinant))/(2.f*A);
    float minT = fmin(positiveT, negativeT);
    minT = fmax(0.f, minT);
    if(minT == 0.f){
        minT = -1.f;
    }
    glm::vec4 intersectionPoint = glm::vec4{px+minT*dx, py+minT*dy, pz+minT*dz, 1.0};

    glm::vec4 normal = glm::vec4(glm::normalize(intersectionPoint));
    return IntersectData{minT, normal, intersectionPoint};
}


Primitives::IntersectData Primitives::intersectCylinder(glm::vec4 eye, glm::vec4 dir, glm::mat4 invCTM) {
    // Transform the ray into the cylinder's local coordinates
    glm::vec4 p = invCTM * eye;
    float px = p[0];
    float py = p[1];
    float pz = p[2];
    glm::vec4 d = invCTM * dir;
    float dx = d[0];
    float dy = d[1];
    float dz = d[2];
    float A = dx * dx + dz * dz;
    float B = 2.f * (dx * px + dz * pz);
    float C = px * px + pz * pz - 0.25;
    float determinant = B * B - 4.f * A * C;
    if (determinant < 0) {
        return IntersectData{-1.f, glm::vec4{0, 0, 0, 0}, glm::vec4{0, 0, 0, 0}};
    }
    float positiveT = (-B + sqrt(determinant)) / (2.f * A);
    float negativeT = (-B - sqrt(determinant)) / (2.f * A);
    //float minT = fmin(positiveT, negativeT);
    if((py+ positiveT * dy < -0.5) || (py + positiveT * dy > 0.5) || positiveT < 0){
        positiveT = FLT_MAX;
    }
    if((py + negativeT * dy < -0.5) || (py+ negativeT * dy > 0.5) || negativeT < 0){
        negativeT = FLT_MAX;
    }
    float minT = fmin(positiveT, negativeT);
    minT = fmax(0.f, minT);
    if(minT == 0.f){
        minT = -1.f;
    }
    // Render the cap if we intersect it first, or if we don't intersect the cylinder (but still intersect the cap)
    float capT = (-0.5 - py) / dy;
    if(capT >= 0){
        if (((px + capT * dx) * (px + capT * dx) + (pz + capT * dz) * (pz + capT * dz) < 0.25) && (capT < minT)) {
            glm::vec4 intersectionPoint = glm::vec4{px + capT * dx, py + capT * dy, pz + capT * dz, 1.0};
            glm::vec4 normal = glm::vec4(0.0, -1.0, 0.0, 0.0);
            return IntersectData{capT, normal, intersectionPoint};
        }
    }
    // Check the other cap
    capT = (0.5 - py) / dy;
    if(capT >= 0){
        if (((px + capT * dx) * (px + capT * dx) + (pz + capT * dz) * (pz + capT * dz) < 0.25) && (capT < minT)) {
            glm::vec4 intersectionPoint = glm::vec4{px + capT * dx, py + capT * dy, pz + capT * dz, 1.0};
            glm::vec4 normal = glm::vec4(0.0, 1.0, 0.0, 0.0);
            return IntersectData{capT, normal, intersectionPoint};
        }
    }
    //check if the cylinder is in bounds
    if (minT == FLT_MAX) {
        return IntersectData{-1.0, glm::vec4(0.f, 0.f, 0.f, 0.f)};
    }
    glm::vec4 intersectionPoint = glm::vec4{px + minT * dx, py + minT * dy, pz + minT * dz, 1.0};
    glm::vec4 normal = glm::normalize(glm::vec4(intersectionPoint[0], 0.0, intersectionPoint[2], 0.0));
    return IntersectData{minT, normal, intersectionPoint};
}


Primitives::IntersectData Primitives::intersectCone(glm::vec4 eye, glm::vec4 dir, glm::mat4 invCTM) {
    glm::vec4 p = invCTM * eye;
    float px = p[0];
    float py = p[1];
    float pz = p[2];
    glm::vec4 d = invCTM * dir;
    float dx = d[0];
    float dy = d[1];
    float dz = d[2];
    float A = dx * dx + dz * dz - 0.25 * dy * dy;
    float B = 2.0f * px * dx + 2.0f * pz * dz - 0.5 * py * dy + 0.25 * dy;
    float C = px * px + pz * pz - 0.25 * py * py + 0.25 * py - 1.0 / 16.0;
    float determinant = B * B - 4.0f * A * C;
    if (determinant < 0) {
        return IntersectData{-1.0f, glm::vec4{0, 0, 0, 0}};
    }
    float positiveT = (-B + sqrt(determinant)) / (2.0f * A);
    float negativeT = (-B - sqrt(determinant)) / (2.0f * A);
    if((py+ positiveT * dy < -0.5) || (py + positiveT * dy > 0.5) || positiveT < 0){
        positiveT = FLT_MAX;
    }
    if((py + negativeT * dy < -0.5) || (py+ negativeT * dy > 0.5) || negativeT < 0){
        negativeT = FLT_MAX;
    }
    float minT = fmin(positiveT, negativeT);
    minT = fmax(0.f, minT);
    if(minT == 0.f){
        minT = -1.f;
    }
    // Calculate the intersection point and radius for the sides
    glm::vec4 intersectionPoint = glm::vec4{px + minT * dx, py + minT * dy, pz + minT * dz, 1.0};
    float radius = sqrt(intersectionPoint[0] * intersectionPoint[0] + intersectionPoint[2] * intersectionPoint[2]);
    // Set the normal based on the radius
    glm::vec4 normal = glm::vec4(intersectionPoint[0], 0.5f * radius, intersectionPoint[2], 0.0f);
    normal = glm::normalize(normal);
    // Check the cap and out of bounds
    if (py + minT * dy < -0.5 || py + minT * dy > 0.5) {
        float capT = (-0.5 - py) / dy;
        if ((px + capT * dx) * (px + capT * dx) + (pz + capT * dz) * (pz + capT * dz) < 0.25 &&
            (capT < minT || (py + minT * dy > 0.5) || (py + minT * dy < -0.5))) {
            minT = fmin(minT, capT);
            glm::vec4 normal = glm::vec4(0.0, -1.0, 0.0, 0.0);
            return IntersectData{minT, normal};
        }
        return IntersectData{-1.0, glm::vec4{0, 0, 0, 0}};
    }
    return IntersectData{minT, normal, intersectionPoint};
}
