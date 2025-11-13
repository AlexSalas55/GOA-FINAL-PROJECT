#ifndef PUREPATHTRACER_H
#define PUREPATHTRACER_H

#include "shader.h"
#include "../core/hemisphericalsampler.h"

class PurePathTracer : public Shader
{
public:
    PurePathTracer();
    PurePathTracer(Vector3D bgColor_, int maxDepth_);

    Vector3D computeColor(const Ray& r,
        const std::vector<Shape*>& objList,
        const std::vector<LightSource*>& lsList) const;

private:
    int maxDepth;
    HemisphericalSampler sampler;
};

#endif // PUREPATHTRACER_H
