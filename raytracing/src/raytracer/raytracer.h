#pragma once

#include <glm/glm.hpp>
#include "utils/rgba.h"
#include "raytracescene.h"

// A forward declaration for the RaytraceScene class

class RayTraceScene;

// A class representing a ray-tracer

class RayTracer
{
public:
    struct Config {
        bool enableShadow        = false;
        bool enableReflection    = false;
        bool enableRefraction    = false;
        bool enableTextureMap    = false;
        bool enableTextureFilter = false;
        bool enableParallelism   = false;
        bool enableSuperSample   = false;
        bool enableAcceleration  = false;
        bool enableDepthOfField  = false;
        bool extraCreditJaggies = false;
        bool extraCreditSupersampling = false;
        bool extraCreditParallelization = false;
        int maxRecursiveDepth    = 4;
        bool onlyRenderNormals   = false;
    };

    struct RayTraceInfo {
        SceneColor color;
        glm::vec4 intersectionPoint;
        SceneMaterial material;
        glm::vec3 normal;
    };

public:
    RayTracer(Config config);

    // Renders the scene synchronously.
    // The ray-tracer will render the scene and fill imageData in-place.
    // @param imageData The pointer to the imageData to be filled.
    // @param scene The scene to be rendered.
    void render(RGBA *imageData, const RayTraceScene &scene);

private:
    const Config m_config;
    RayTraceInfo traceRay(const glm::vec4& rayEye, const glm::vec4& rayDir, const RayTraceScene& scene, int recursionDepth);
    //RayTraceInfo traceRay(const glm::vec4& eye, const glm::vec4& dir, const std::vector<RenderShapeData>& shapes, const RayTraceScene& scene, int count);

};

