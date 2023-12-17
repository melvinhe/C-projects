#include "texture.h"
#include "scenedata.h"
#include <numbers>

/**
 * @brief Stores the image specified from the input file in this class's
 * `std::vector<RGBA> m_image`.
 * @param file: file path to an image
 * @return True if successfully loads image, False otherwise.
 */
TextureMap::TextureInfo TextureMap::loadTextureFromFile(const QString &file) {
    QImage myTexture;

    int width; int height;
    if (!myTexture.load(file)) {
        std::cout<<"Failed to load in image: " << file.toStdString() << std::endl;
        return TextureInfo{nullptr, 0, 0};
    }
    myTexture = myTexture.convertToFormat(QImage::Format_RGBX8888);
    width = myTexture.width();
    height = myTexture.height();

    RGBA* texture = new RGBA[width*height];
    QByteArray arr = QByteArray::fromRawData((const char*) myTexture.bits(), myTexture.sizeInBytes());

    for (int i = 0; i < arr.size() / 4.f; i++){
        texture[i] = RGBA{(std::uint8_t) arr[4*i], (std::uint8_t) arr[4*i+1], (std::uint8_t) arr[4*i+2], (std::uint8_t) arr[4*i+3]};
    }

    return TextureInfo{texture, width, height};
}


glm::vec2 TextureMap::coneMap(glm::vec3 xyz) {
    float epsilon = 0.0001f;
    if (fabs(xyz[1] + 0.5f) < epsilon) { return glm::vec2(0.5f + xyz[0], 0.5f + xyz[2]); }
    else { return glm::vec2(-atan2(xyz[2], xyz[0]) / (2.f*std::numbers::pi), xyz[1] + 0.5f); }
}

glm::vec2 TextureMap::cubeMap(glm::vec3 xyz) {
    float epsilon = 0.0001f;
    // check which face of cube point lies and map to UV
    // right face
    if (fabs(xyz[0] - 0.5f) < epsilon) { return glm::vec2(0.5f - xyz[2], 0.5f + xyz[1]); }
    // left face
    else if (fabs(xyz[0] + 0.5f) < epsilon) { return glm::vec2(0.5f + xyz[2], 0.5f + xyz[1]); }
    // top face
    else if (fabs(xyz[1] - 0.5f) < epsilon) { return glm::vec2(0.5f + xyz[0], 0.5f - xyz[2]); }
    // bottom face
    else if (fabs(xyz[1] + 0.5f) < epsilon) { return glm::vec2(0.5f + xyz[0], 0.5f + xyz[2]); }
    // front face
    else if (fabs(xyz[2] - 0.5f) < epsilon) { return glm::vec2(0.5f + xyz[0], 0.5f + xyz[1]); }
    // back face
    else { return glm::vec2(0.5f - xyz[0], 0.5f + xyz[1]); }
}

glm::vec2 TextureMap::cylinderMap(glm::vec3 xyz) {
    float epsilon = 0.0001f;
    if (fabs(xyz[1] - 0.5f) < epsilon) { return glm::vec2(0.5f + xyz[0], 0.5f - xyz[2]); }
    else if (fabs(xyz[1] + 0.5f) < epsilon) { return glm::vec2(0.5f + xyz[0], 0.5f + xyz[2]); }
    else { return glm::vec2(-atan2(xyz[2], xyz[0]) / (2.f*std::numbers::pi), xyz[1] + 0.5); }
}

glm::vec2 TextureMap::sphereMap(glm::vec3 xyz) {
    float epsilon = 0.0001f;
    float v = (asin(xyz[1] / 0.5f) / std::numbers::pi) + 0.5f;
    if (v == 0 || v == 1) { return glm::vec2(0.5f, v); }
    else { return glm::vec2(-atan2(xyz[2], xyz[0]) / (2.f*std::numbers::pi), v); }
}

glm::vec2 TextureMap::getUV(glm::vec3 intersection, PrimitiveType shape) {
    switch (shape) {
    case PrimitiveType::PRIMITIVE_SPHERE:
        return sphereMap(intersection);
    case PrimitiveType::PRIMITIVE_CUBE:
        return cubeMap(intersection);
    case PrimitiveType::PRIMITIVE_CYLINDER:
        return cylinderMap(intersection);
    case PrimitiveType::PRIMITIVE_CONE:
        return coneMap(intersection);
    default:
        return glm::vec2(0.f);
    }
}


RGBA TextureMap::getTexture(glm::vec2 uv, std::string fileStr, float repeatU, float repeatV,
                      std::unordered_map<std::string, TextureMap::TextureInfo> &allTextures) {
    TextureMap::TextureInfo currentTexture = allTextures[fileStr];
    int c = static_cast<int>(std::floor((uv[0] * repeatU * currentTexture.width))) % currentTexture.width;
    int r = static_cast<int>(std::floor(((1 - uv[1]) * repeatV * currentTexture.height))) % currentTexture.width;
     // prevents out-of-bounds error for coordinates
    if (uv[0] == 1.f) { c -= 1; }
    if (uv[1] == 1.f) { r -= 1; }
    return currentTexture.data[r * currentTexture.width + c];
}


