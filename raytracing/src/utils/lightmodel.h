#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "sceneparser.h"
#include "scenedata.h"
#include "texture.h"
#include "rgba.h"

struct LightModel {
    // Function to calculate Phong illumination
    static RGBA phong(glm::vec3 normal,
                      SceneGlobalData data,
                      SceneMaterial material,
                      glm::vec4 cameraDir,
                      std::vector<SceneLightData> lights,
                      glm::vec4 position,
                      std::vector<RenderShapeData> shapes,
                      std::unordered_map<std::string, TextureMap::TextureInfo> textures);


    // Function to convert a glm::vec4 illumination to an RGBA color
    static RGBA toRGBA(const glm::vec4 &illumination);

    // Function to calculate the total illumination considering different light types
    static RGBA calculateIllumination(const glm::vec3& normal, const SceneGlobalData& globalData,
                                      const SceneMaterial& material, const glm::vec4& cameraDir,
                                      const std::vector<SceneLightData>& lights, const glm::vec4& position);

    // Function to calculate Phong illumination for directional lights
    static RGBA calculateDirectionalLight(const glm::vec3& normal, const SceneGlobalData& globalData,
                                          const SceneMaterial& material, const glm::vec4& cameraDir,
                                          const SceneLightData& light);

    // Function to calculate Phong illumination for point lights with attenuation
    static RGBA calculatePointLight(const glm::vec3& normal, const SceneGlobalData& globalData,
                                    const SceneMaterial& material, const glm::vec4& cameraDir,
                                    const SceneLightData& light, const glm::vec4& position);

    // Function to calculate Phong illumination for spot lights with angular falloff and attenuation
    static RGBA calculateSpotLight(const glm::vec3& normal, const SceneGlobalData& globalData,
                                   const SceneMaterial& material, const glm::vec4& cameraDir,
                                   const SceneLightData& light, const glm::vec4& position);

    static float calculateAttenuation(const glm::vec3& function, float distance);

    static float calculateAngularFalloff(float angle, float penumbra, const glm::vec3& lightDir, const glm::vec3& pointDir);

    static bool isPointInShadow(glm::vec4 intersectionPoint, glm::vec4 lightPosition, const std::vector<RenderShapeData>& shapes);

};
