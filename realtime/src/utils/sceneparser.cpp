#include "sceneparser.h"
#include "scenefilereader.h"
#include <glm/gtx/transform.hpp>

#include <chrono>
#include <iostream>

bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readJSON();
    if (!success) {
        return false;
    }

    renderData.globalData = fileReader.getGlobalData();
    renderData.cameraData = fileReader.getCameraData();

    SceneNode* rootNode = fileReader.getRootNode();
    renderData.shapes.clear();
    renderData.lights.clear();

    // Start traversal from the root node with an identity matrix
    glm::mat4 identity = glm::mat4(1.0f,0.0f,0.0f,0.0f,
                                   0.0f,1.0f,0.0f,0.0f,
                                   0.0f,0.0f,1.0f,0.0f,
                                   0.0f,0.0f,0.0f,1.0f
                                   );
    traverseSceneGraph(rootNode, identity, renderData);
    return success;
}

void SceneParser::traverseSceneGraph(SceneNode* node, glm::mat4 cumulativeTransform, RenderData &renderData) {
    if (node == nullptr) {
        return;
    }
    for (ScenePrimitive* prim : node->primitives) {
        RenderShapeData shapeData;
        shapeData.primitive = *prim;
        shapeData.ctm = cumulativeTransform;
        renderData.shapes.push_back(shapeData);
    }

    for (SceneLight* light : node->lights) {
        SceneLightData lightData;
        lightData.id = light->id;
        lightData.type = light->type;
        lightData.color = light->color;
        lightData.function = light->function;
        lightData.penumbra = light->penumbra;
        lightData.angle = light->angle;
        lightData.width = light->width;
        lightData.height = light->height;
        lightData.pos = cumulativeTransform * glm::vec4(0.0f,0.0f,0.0f,1.0f);
        lightData.dir = cumulativeTransform * light->dir;

        renderData.lights.push_back(lightData);
    }

    for (SceneNode* child : node->children) {
        glm::mat4 newTransform = glm::mat4(1.0f);
        for (SceneTransformation* tm : child->transformations) {
            TransformationType tp = tm->type;
            if (tp == TransformationType::TRANSFORMATION_TRANSLATE) {
                newTransform *= glm::translate(tm->translate);
            } else if (tp == TransformationType::TRANSFORMATION_SCALE) {
                newTransform *= glm::scale(tm->scale);
            } else if (tp == TransformationType::TRANSFORMATION_ROTATE) {
                newTransform *= glm::rotate(tm->angle, tm->rotate);
            } else {
                newTransform *= tm->matrix;
            }
        }
        traverseSceneGraph(child, cumulativeTransform * newTransform, renderData);
    }
}
