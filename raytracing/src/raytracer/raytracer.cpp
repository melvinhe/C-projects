#include "raytracer.h"
#include "raytracescene.h"
#include "primitives.h"
#include "utils/texture.h"
#include "utils/lightmodel.h"
#include "extra.h"

RayTracer::RayTracer(Config config) :
    m_config(config)
{}

std::unordered_map<std::string, TextureMap::TextureInfo> textures;

void RayTracer::render(RGBA* imageData, const RayTraceScene& scene) {
    int k = 1;
    float theta_h = scene.getCamera().getHeightAngle();
    glm::mat4 cameraViewMatrix = glm::inverse(scene.getCamera().getViewMatrix());
    std::vector<RenderShapeData> shapes = scene.getShapes();
    // Adding to texture mapping info data structure
    for (const RenderShapeData& shape : shapes) {
        if (shape.primitive.material.textureMap.isUsed) {
            std::string filename = shape.primitive.material.textureMap.filename;
            QString file = QString::fromStdString(filename);
            TextureMap::TextureInfo texture_info = TextureMap::TextureInfo(TextureMap::loadTextureFromFile(file));
            textures[filename] = texture_info;
        }
    }
    // Up to 3 points supersampling
    if (m_config.extraCreditSupersampling) {
        int numSamples = 4; // Number of samples per pixel
        for (int row = 0; row < scene.height(); row++) {
            for (int col = 0; col < scene.width(); col++) {
                glm::vec4 averagedColor{0.0f, 0.0f, 0.0f, 0.0f};
                for (int s = 0; s < numSamples; s++) {
                    float x = (col + (s % 2) * 0.25 + 0.25) / static_cast<float>(scene.width()) - 0.5;
                    float y = (row + (s / 2) * 0.25 + 0.25) / static_cast<float>(scene.height()) - 0.5;
                    float V = -2.f * k * tan(theta_h / 2.0);
                    float U = static_cast<float>(scene.width()) / static_cast<float>(scene.height()) * -V;
                    float u = U * x;
                    float v = V * y;
                    glm::vec4 uvk = glm::vec4{u, v, -k, 1};
                    glm::vec4 eye = cameraViewMatrix * glm::vec4{0, 0, 0, 1};
                    glm::vec4 dir = cameraViewMatrix * (uvk - glm::vec4{0, 0, 0, 1});
                    std::vector<RenderShapeData> shapes = scene.getShapes();
                    RayTraceInfo info = traceRay(eye, dir, scene, 0); // Start tracing the ray with count 0
                    if (info.color != glm::vec4(0)) {
                        info.color = glm::clamp(info.color, glm::vec4(0), glm::vec4(1));
                        averagedColor += info.color;
                    }
                }
                averagedColor /= static_cast<float>(numSamples); // Average the colors
                imageData[col + row * scene.width()] = RGBA{
                    static_cast<uint8_t>(averagedColor.r * 255.f),
                    static_cast<uint8_t>(averagedColor.g * 255.f),
                    static_cast<uint8_t>(averagedColor.b * 255.f)
                };
            }
        }
    } else {
        // Original non-supersampled rendering code
        for (int row = 0; row < scene.height(); row++) {
            for (int col = 0; col < scene.width(); col++) {
                float x = (col + 0.5) / static_cast<float>(scene.width()) - 0.5;
                float y = (row + 0.5) / static_cast<float>(scene.height()) - 0.5;
                float V = -2.f * k * tan(theta_h / 2.0);
                float U = static_cast<float>(scene.width()) / static_cast<float>(scene.height()) * -V;
                float u = U * x;
                float v = V * y;
                glm::vec4 uvk = glm::vec4{u, v, -k, 1};
                glm::vec4 eye = cameraViewMatrix * glm::vec4{0, 0, 0, 1};
                glm::vec4 dir = cameraViewMatrix * (uvk - glm::vec4{0, 0, 0, 1});
                std::vector<RenderShapeData> shapes = scene.getShapes();
                RayTraceInfo info = traceRay(eye, dir, scene, 0); // Start tracing the ray with count 0
                if (info.color != glm::vec4(0)) {
                    info.color = glm::clamp(info.color, glm::vec4(0), glm::vec4(1));
                    imageData[col + row * scene.width()] = RGBA{static_cast<uint8_t>(info.color.r * 255.f),
                                                                static_cast<uint8_t>(info.color.g * 255.f),
                                                                static_cast<uint8_t>(info.color.b * 255.f)};
                }
            }
        }
    }
    // 3 points extra credit (anti-aliasing)
    if (m_config.extraCreditJaggies) {
        applySimpleBlurFilter(imageData, scene.width(), scene.height(), 10);
    }
}


RayTracer::RayTraceInfo RayTracer::traceRay(const glm::vec4& rayEye, const glm::vec4& rayDir, const RayTraceScene& scene, int recursionDepth) {
    SceneGlobalData globalData = scene.getGlobalData();
    std::vector<SceneLightData> lights = scene.getLights();
    if (recursionDepth >= m_config.maxRecursiveDepth) {
        // Maximum reflection depth reached, return black
        return {glm::vec4{0, 0, 0, 0}, glm::vec4{0, 0, 0, 0}, SceneMaterial{}, glm::vec3{0, 0, 0}};
    }
    std::vector<RenderShapeData> shapes = scene.getShapes();
    float lowest_t = std::numeric_limits<float>::max();
    float ks = scene.getGlobalData().ks;
    RayTraceInfo info = {glm::vec4{0, 0, 0, 0}, glm::vec4{0, 0, 0, 0}, SceneMaterial{}, glm::vec3{0, 0, 0}};
    for (const RenderShapeData& shape : shapes) {
        glm::mat4 invCTM = glm::inverse(shape.ctm);
        Primitives::IntersectData intersect;
        if (shape.primitive.type == PrimitiveType::PRIMITIVE_CUBE) {
            intersect = Primitives::intersectCube(rayEye, rayDir, invCTM);
        } else if (shape.primitive.type == PrimitiveType::PRIMITIVE_SPHERE) {
            intersect = Primitives::intersectSphere(rayEye, rayDir, invCTM);
        } else if (shape.primitive.type == PrimitiveType::PRIMITIVE_CYLINDER) {
            intersect = Primitives::intersectCylinder(rayEye, rayDir, invCTM);
        } else if (shape.primitive.type == PrimitiveType::PRIMITIVE_CONE) {
            intersect = Primitives::intersectCone(rayEye, rayDir, invCTM);
        }
        float t = intersect.t;
        if (t != -1.f && t < lowest_t) {
            lowest_t = t;
            glm::mat3 upper3 = glm::mat3(shape.ctm);
            glm::vec3 surfaceNormal = glm::normalize(glm::inverse(glm::transpose(upper3)) * intersect.normal);
            SceneMaterial material = shape.primitive.material;
            glm::vec4 cameraDir = -rayDir;
            glm::vec4 position = rayEye + t * rayDir;
            RGBA ph = LightModel::phong(surfaceNormal, globalData, material, cameraDir, lights, position, shapes, textures);
            info.color = SceneColor{ph.r / 255.f, ph.g / 255.f, ph.b / 255.f, 255.f};
            info.intersectionPoint = position;
            info.material = material;
            info.normal = surfaceNormal;
            float epsilon = 0.001f;
            // Implement reflection by recursively tracing the reflected ray
            if (glm::length(material.cReflective) > 0.f && ks > 0.f) {
                glm::vec4 surface_normal = glm::vec4{surfaceNormal[0], surfaceNormal[1], surfaceNormal[2], 0.f};
                glm::vec4 normalized_dir = -glm::normalize(rayDir);
                glm::vec4 reflectedDir = -glm::normalize(normalized_dir - 2.0f * surface_normal * glm::dot(surface_normal, normalized_dir));
                RayTraceInfo reflectionInfo = traceRay(position + epsilon * reflectedDir, reflectedDir, scene, recursionDepth + 1);
                // Check if the reflected ray intersects with any objects
                if (reflectionInfo.color != glm::vec4(0)) {
                    info.color = SceneColor{
                        info.color[0] + ks * material.cReflective[0] * reflectionInfo.color[0],
                        info.color[1] + ks * material.cReflective[1] * reflectionInfo.color[1],
                        info.color[2] + ks * material.cReflective[2] * reflectionInfo.color[2],
                        1.0f
                    };
                }
            }
        }
    }
    return info;
}
