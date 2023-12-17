#include <stdexcept>
#include "raytracescene.h"
#include "utils/sceneparser.h"

RayTraceScene::RayTraceScene(int width, int height, const RenderData &metaData)
    : sceneWidth(width), sceneHeight(height), sceneMeta(metaData) {
}

const int& RayTraceScene::width() const {
    return sceneWidth;
}

const int& RayTraceScene::height() const {
    return sceneHeight;
}

const SceneGlobalData& RayTraceScene::getGlobalData() const {
    return sceneMeta.globalData;
}

const Camera& RayTraceScene::getCamera() const {
    static Camera camera(sceneMeta.cameraData, sceneWidth, sceneHeight);
    return camera;
}

RenderData RayTraceScene::getRenderData() const {
    return sceneMeta;
}

std::vector<RenderShapeData> RayTraceScene::getShapes() const {
    return sceneMeta.shapes;
}
std::vector<SceneLightData> RayTraceScene::getLights() const {
    return sceneMeta.lights;
}
