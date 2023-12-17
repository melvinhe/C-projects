#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "rgba.h"
#include "utils/scenedata.h"
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QImage>
#include <QtCore>
#include <iostream>

struct TextureMap {
public:
    struct TextureInfo {
        RGBA* data;
        int width;
        int height;
    };
    static TextureInfo loadTextureFromFile(const QString &file);
    static glm::vec2 sphereMap(glm::vec3 xyz);
    static glm::vec2 cubeMap(glm::vec3 xyz);
    static glm::vec2 cylinderMap(glm::vec3 xyz);
    static glm::vec2 coneMap(glm::vec3 xyz);
    static glm::vec2 getUV(glm::vec3 intersection, PrimitiveType shape);
    static RGBA getTexture(glm::vec2 uv, std::string fileStr, float repeatU, float repeatV,
                           std::unordered_map<std::string, TextureMap::TextureInfo> &allTextures);
};
