#include "lightmodel.h"
#include "scenedata.h"
#include "raytracer/primitives.h"
#include <iostream>


// Implementing the phong illumination functionality
RGBA LightModel::phong(glm::vec3 normal, SceneGlobalData data, SceneMaterial material, glm::vec4 cameraDir, std::vector<SceneLightData> lights,
                       glm::vec4 position, std::vector<RenderShapeData> shapes, std::unordered_map<std::string, TextureMap::TextureInfo>textures) {
    glm::vec4 illumination(0, 0, 0, 1);
    float kd = data.kd;
    float ka = data.ka;
    float ks = data.ks;
    glm::vec3 ambient = material.cAmbient;
    glm::vec3 diffuse = material.cDiffuse;
    glm::vec3 specular = material.cSpecular;
    illumination[0] += ka * ambient[0];
    illumination[1] += ka * ambient[1];
    illumination[2] += ka * ambient[2];
    normal = glm::normalize(normal);
    for (const SceneLightData &light : lights) {
        if (light.type == LightType::LIGHT_DIRECTIONAL){
            glm::vec3 lightdir = light.dir;
            lightdir = glm::normalize(light.dir);
            // Check if the point is in shadow
            PrimitiveType intersectedShape;
            glm::vec4 intersectedPoint;
            bool isShadowed = false;
            glm::vec4 shadowRayDir = -light.dir;  // Directional light, shadow ray points towards the light
            glm::vec4 shadowRayOrigin = position + 0.001f * shadowRayDir; // Slightly offset the origin to avoid self-intersection
            for (const RenderShapeData& shape : shapes) {
                glm::mat4 invCTM = glm::inverse(shape.ctm);
                PrimitiveType prim = shape.primitive.type;
                if (prim == PrimitiveType::PRIMITIVE_CUBE) {
                    Primitives::IntersectData intersect = Primitives::intersectCube(shadowRayOrigin, shadowRayDir, invCTM);
                    if (intersect.t > 0) {
                        isShadowed = true;
                        intersectedShape = PrimitiveType::PRIMITIVE_CUBE;
                        intersectedPoint = intersect.intersectPoint;
                        break; // Exit the loop if shadowed
                    }
                } else if (shape.primitive.type == PrimitiveType::PRIMITIVE_CYLINDER) {
                    Primitives::IntersectData intersect = Primitives::intersectCylinder(shadowRayOrigin, shadowRayDir, invCTM);
                    if (intersect.t > 0) {
                        isShadowed = true;
                        intersectedShape = PrimitiveType::PRIMITIVE_CYLINDER;
                        intersectedPoint = intersect.intersectPoint;
                        break; // Exit the loop if shadowed
                    }
                } else if (shape.primitive.type == PrimitiveType::PRIMITIVE_CONE) {
                    Primitives::IntersectData intersect = Primitives::intersectCone(shadowRayOrigin, shadowRayDir, invCTM);
                    if (intersect.t > 0) {
                        isShadowed = true;
                        intersectedShape = PrimitiveType::PRIMITIVE_CONE;
                        intersectedPoint = intersect.intersectPoint;
                        break; // Exit the loop if shadowed
                    }
                } else if (shape.primitive.type == PrimitiveType::PRIMITIVE_SPHERE) {
                    Primitives::IntersectData intersect = Primitives::intersectSphere(shadowRayOrigin, shadowRayDir, invCTM);
                    if (intersect.t > 0) {
                        isShadowed = true;
                        intersectedShape = PrimitiveType::PRIMITIVE_SPHERE;
                        intersectedPoint = intersect.intersectPoint;
                        break; // Exit the loop if shadowed
                    }
                }
            }
            if (!isShadowed) {
                float blend;
                SceneColor textureColor;
                if(material.textureMap.isUsed) {
                    blend = material.blend;
                    float repeatU = material.textureMap.repeatU;
                    float repeatV = material.textureMap.repeatV;
                    std::string filename = material.textureMap.filename;
                    TextureMap::TextureInfo texture_info = textures[filename];
                    glm::vec2 uv = TextureMap::getUV(intersectedPoint, intersectedShape);
                    RGBA texture_rgba = TextureMap::getTexture(uv, filename, repeatU, repeatV, textures);
                    textureColor = SceneColor{texture_rgba.r/255.f, texture_rgba.g/255.f, texture_rgba.b/255.f, 255.f};
                } else {
                    blend = 0.f;
                    textureColor = SceneColor{0,0,0,0};
                }
                // The point is not in shadow, calculate illumination
                float cosTheta = glm::dot(normal, -lightdir);
                if (cosTheta > 0) {
                    illumination[0] += light.color[0] * (kd * diffuse[0] * (1.f - blend) + blend * textureColor[0]) * cosTheta;
                    illumination[1] += light.color[1] * (kd * diffuse[1] * (1.f - blend) + blend * textureColor[1]) * cosTheta;
                    illumination[2] += light.color[2] * (kd * diffuse[2] * (1.f - blend) + blend * textureColor[2]) * cosTheta;
                }
                glm::vec3 reflectedLightDirection = glm::normalize(2.0f * glm::dot(lightdir, normal) * normal - lightdir);
                float cosAlpha = glm::max(0.0f, glm::dot(-reflectedLightDirection, glm::normalize(glm::vec3(cameraDir))));
                float specularTerm = glm::pow(cosAlpha, material.shininess);
                illumination[0] += light.color[0] * ks * specular[0] * specularTerm;
                illumination[1] += light.color[1] * ks * specular[1] * specularTerm;
                illumination[2] += light.color[2] * ks * specular[2] * specularTerm;
            }
        } else if (light.type == LightType::LIGHT_POINT) {
            glm::vec3 lightdir = glm::normalize(glm::vec3(position - light.pos));
            float distance = glm::distance(position, light.pos);
            float attenuationFactor = std::min(1.0f, 1.0f / (light.function[0] + light.function[1] * distance + light.function[2] * distance * distance));
            bool isShadowed = false;
            PrimitiveType intersectedShape;
            glm::vec4 intersectedPoint;
            // Check for shadows by casting a ray from the intersection point to the light source
            for (const RenderShapeData& shape : shapes) {
                glm::mat4 invCTM = glm::inverse(shape.ctm);
                glm::vec4 shadowRayDir = glm::normalize(light.pos - position);
                glm::vec4 shadowRayOrigin = position + 0.001f * shadowRayDir; // Slightly offset the origin to avoid self-intersection
                PrimitiveType prim = shape.primitive.type;
                if (prim == PrimitiveType::PRIMITIVE_CUBE) {
                    Primitives::IntersectData intersect = Primitives::intersectCube(shadowRayOrigin, shadowRayDir, invCTM);
                    if (intersect.t > 0 && intersect.t < distance) {
                        isShadowed = true;
                        intersectedShape = PrimitiveType::PRIMITIVE_CUBE;
                        intersectedPoint = intersect.intersectPoint;
                        break; // Exit the loop if shadowed
                    }
                } else if (shape.primitive.type == PrimitiveType::PRIMITIVE_CYLINDER) {
                    Primitives::IntersectData intersect = Primitives::intersectCylinder(shadowRayOrigin, shadowRayDir, invCTM);
                    if (intersect.t > 0 && intersect.t < distance) {
                        isShadowed = true;
                        intersectedShape = PrimitiveType::PRIMITIVE_CYLINDER;
                        intersectedPoint = intersect.intersectPoint;
                        break; // Exit the loop if shadowed
                    }
                } else if (shape.primitive.type == PrimitiveType::PRIMITIVE_CONE) {
                    Primitives::IntersectData intersect = Primitives::intersectCone(shadowRayOrigin, shadowRayDir, invCTM);
                    if (intersect.t > 0 && intersect.t < distance) {
                        isShadowed = true;
                        intersectedShape = PrimitiveType::PRIMITIVE_CONE;
                        intersectedPoint = intersect.intersectPoint;
                        break; // Exit the loop if shadowed
                    }
                } else if (shape.primitive.type == PrimitiveType::PRIMITIVE_SPHERE) {
                    Primitives::IntersectData intersect = Primitives::intersectSphere(shadowRayOrigin, shadowRayDir, invCTM);
                    if (intersect.t > 0 && intersect.t < distance) {
                        isShadowed = true;
                        intersectedShape = PrimitiveType::PRIMITIVE_SPHERE;
                        intersectedPoint = intersect.intersectPoint;
                        break; // Exit the loop if shadowed
                    }
                }
            }
            if (!isShadowed) {
                float blend;
                SceneColor textureColor;
                if(material.textureMap.isUsed) {
                    blend = material.blend;
                    float repeatU = material.textureMap.repeatU;
                    float repeatV = material.textureMap.repeatV;
                    std::string filename = material.textureMap.filename;
                    TextureMap::TextureInfo texture_info = textures[filename];
                    glm::vec2 uv = TextureMap::getUV(intersectedPoint, intersectedShape);
                    RGBA texture_rgba = TextureMap::getTexture(uv, filename, repeatU, repeatV, textures);
                    textureColor = SceneColor{texture_rgba.r/255.f, texture_rgba.g/255.f, texture_rgba.b/255.f, 255.f};
                } else {
                    blend = 0.f;
                    textureColor = SceneColor{0,0,0,0};
                }
                // adding diffuse
                float cosTheta = glm::dot(normal, -lightdir);
                if(cosTheta > 0){
                    illumination[0] += light.color[0] * (kd * diffuse[0] * (1.f - blend) + blend * textureColor[0]) * cosTheta * attenuationFactor;
                    illumination[1] += light.color[1] * (kd * diffuse[1] * (1.f - blend) + blend * textureColor[1]) * cosTheta * attenuationFactor;
                    illumination[2] += light.color[2] * (kd * diffuse[2] * (1.f - blend) + blend * textureColor[2]) * cosTheta * attenuationFactor;
                }
                // adding specular
                glm::vec3 reflectedLightDirection = glm::normalize(2.0f * glm::dot(lightdir, normal) * normal - lightdir);
                float cosAlpha = glm::max(0.0f, glm::dot(-reflectedLightDirection, glm::normalize(glm::vec3(cameraDir))));
                float specularTerm = glm::pow(cosAlpha, material.shininess);
                illumination[0] += light.color[0] * ks * specular[0] * specularTerm * attenuationFactor;
                illumination[1] += light.color[1] * ks * specular[1] * specularTerm * attenuationFactor;
                illumination[2] += light.color[2] * ks * specular[2] * specularTerm * attenuationFactor;
            }
        } else if (light.type == LightType::LIGHT_SPOT) {
            glm::vec3 lightdir = glm::normalize(glm::vec3(position - light.pos));
            float distance = glm::distance(light.pos, position);
            float attenuationFactor = std::min(1.0f, 1.0f / (light.function[0] + light.function[1] * distance + light.function[2] * distance * distance));
            bool isShadowed = false;
            PrimitiveType intersectedShape;
            glm::vec4 intersectedPoint;
            // Check for shadows by casting a ray from the intersection point to the light source
            for (const RenderShapeData& shape : shapes) {
                glm::mat4 invCTM = glm::inverse(shape.ctm);
                glm::vec4 shadowRayDir = glm::normalize(light.pos - position);
                glm::vec4 shadowRayOrigin = position + 0.001f * shadowRayDir; // Slightly offset the origin to avoid self-intersection
                PrimitiveType prim = shape.primitive.type;
                if (prim == PrimitiveType::PRIMITIVE_CUBE) {
                    Primitives::IntersectData intersect = Primitives::intersectCube(shadowRayOrigin, shadowRayDir, invCTM);
                    if (intersect.t > 0 && intersect.t < distance) {
                        isShadowed = true;
                        intersectedShape = PrimitiveType::PRIMITIVE_CUBE;
                        intersectedPoint = intersect.intersectPoint;
                        break; // Exit the loop if shadowed
                    }
                } else if (shape.primitive.type == PrimitiveType::PRIMITIVE_CYLINDER) {
                    Primitives::IntersectData intersect = Primitives::intersectCylinder(shadowRayOrigin, shadowRayDir, invCTM);
                    if (intersect.t > 0 && intersect.t < distance) {
                        isShadowed = true;
                        intersectedShape = PrimitiveType::PRIMITIVE_CYLINDER;
                        intersectedPoint = intersect.intersectPoint;
                        break; // Exit the loop if shadowed
                    }
                } else if (shape.primitive.type == PrimitiveType::PRIMITIVE_CONE) {
                    Primitives::IntersectData intersect = Primitives::intersectCone(shadowRayOrigin, shadowRayDir, invCTM);
                    if (intersect.t > 0 && intersect.t < distance) {
                        isShadowed = true;
                        intersectedShape = PrimitiveType::PRIMITIVE_CONE;
                        intersectedPoint = intersect.intersectPoint;
                        break; // Exit the loop if shadowed
                    }
                } else if (shape.primitive.type == PrimitiveType::PRIMITIVE_SPHERE) {
                    Primitives::IntersectData intersect = Primitives::intersectSphere(shadowRayOrigin, shadowRayDir, invCTM);
                    if (intersect.t > 0 && intersect.t < distance) {
                        isShadowed = true;
                        intersectedShape = PrimitiveType::PRIMITIVE_SPHERE;
                        intersectedPoint = intersect.intersectPoint;
                        break; // Exit the loop if shadowed
                    }
                }
            }
            if (!isShadowed) {
                float blend;
                SceneColor textureColor;
                if(material.textureMap.isUsed) {
                    blend = material.blend;
                    float repeatU = material.textureMap.repeatU;
                    float repeatV = material.textureMap.repeatV;
                    std::string filename = material.textureMap.filename;
                    TextureMap::TextureInfo texture_info = textures[filename];
                    glm::vec2 uv = TextureMap::getUV(intersectedPoint, intersectedShape);
                    RGBA texture_rgba = TextureMap::getTexture(uv, filename, repeatU, repeatV, textures);
                    textureColor = SceneColor{texture_rgba.r/255.f, texture_rgba.g/255.f, texture_rgba.b/255.f, 255.f};
                } else {
                    blend = 0.f;
                    textureColor = SceneColor{0,0,0,0};
                }
                // adding diffuse
                glm::vec3 spotlightdir = glm::normalize(-light.dir);
                float x = acos(glm::dot(spotlightdir, -lightdir));
                float theta_outer = light.angle;
                float penumbra = light.penumbra;
                float theta_inner = theta_outer - penumbra;
                if (x <= theta_inner) {
                    // adding diffuse
                    float cosTheta = glm::dot(normal, -lightdir);
                    if(cosTheta > 0){
                        illumination[0] += light.color[0] * (kd * diffuse[0] * (1.f - blend) + blend * textureColor[0]) * cosTheta * attenuationFactor;
                        illumination[1] += light.color[1] * (kd * diffuse[1] * (1.f - blend) + blend * textureColor[1]) * cosTheta * attenuationFactor;
                        illumination[2] += light.color[2] * (kd * diffuse[2] * (1.f - blend) + blend * textureColor[2]) * cosTheta * attenuationFactor;
                    }
                    // adding specular
                    glm::vec3 reflectedLightDirection = glm::normalize(2.0f * glm::dot(lightdir, normal) * normal - lightdir);
                    float cosAlpha = glm::max(0.0f, glm::dot(-reflectedLightDirection, glm::normalize(glm::vec3(cameraDir))));
                    float specularTerm = glm::pow(cosAlpha, material.shininess);
                    illumination[0] += light.color[0] * ks * specular[0] * specularTerm * attenuationFactor;
                    illumination[1] += light.color[1] * ks * specular[1] * specularTerm * attenuationFactor;
                    illumination[2] += light.color[2] *ks * specular[2] * specularTerm * attenuationFactor;
                } else if (theta_inner < x && x <= theta_outer) {
                    float val = (x - theta_inner) / (theta_outer - theta_inner);
                    float falloff = -2.0f * val * val * val + 3.0f * val * val;
                    // adding diffuse
                    float cosTheta = glm::dot(normal, -lightdir);
                    if(cosTheta > 0){
                        illumination[0] += light.color[0] * (kd * diffuse[0] * (1.f - blend) + blend * textureColor[0]) * cosTheta * (1.f - falloff) * attenuationFactor;
                        illumination[1] += light.color[1] * (kd * diffuse[1] * (1.f - blend) + blend * textureColor[1]) * cosTheta * (1.f - falloff) * attenuationFactor;
                        illumination[2] += light.color[2] * (kd * diffuse[2] * (1.f - blend) + blend * textureColor[2]) * cosTheta * (1.f - falloff) * attenuationFactor;
                    }
                    // adding specular
                    glm::vec3 reflectedLightDirection = glm::normalize(2.0f * glm::dot(lightdir, normal) * normal - lightdir);
                    float cosAlpha = glm::max(0.0f, glm::dot(-reflectedLightDirection, glm::normalize(glm::vec3(cameraDir))));
                    float specularTerm = glm::pow(cosAlpha, material.shininess);
                    illumination[0] += light.color[0] * ks * specular[0] * specularTerm * attenuationFactor * (1.f - falloff);
                    illumination[1] += light.color[1] * ks * specular[1] * specularTerm * attenuationFactor * (1.f - falloff);
                    illumination[2] += light.color[2] *ks * specular[2] * specularTerm * attenuationFactor * (1.f - falloff);
                }
            }
        }
    }
    RGBA returnValue = toRGBA(illumination);
    return returnValue;
}


RGBA LightModel::toRGBA(const glm::vec4 &illumination) {
    // Clamp and scale the illumination values to the range [0, 255]
    uint8_t r = static_cast<uint8_t>(255.0f*std::min(std::max(illumination[0], 0.0f), 1.0f));
    uint8_t g = static_cast<uint8_t>(255.0f*std::min(std::max(illumination[1], 0.0f), 1.0f));
    uint8_t b = static_cast<uint8_t>(255.0f*std::min(std::max(illumination[2], 0.0f), 1.0f));
    // Create and return the RGBA struct
    return RGBA{r, g, b, 255};
}

