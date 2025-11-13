#ifndef HEMISPHERICALDIRECT_H
#define HEMISPHERICALDIRECT_H

#include "shader.h"
#include "../core/hemisphericalsampler.h"

class HemisphericalDirect : public Shader
{
public:
    HemisphericalDirect();
    HemisphericalDirect(Vector3D bgColor_, int numSamples_);

    Vector3D computeColor(const Ray& r,
        const std::vector<Shape*>& objList,
        const std::vector<LightSource*>& lsList) const;

private:
    int numSamples;
    HemisphericalSampler sampler;
};

#endif // HEMISPHERICALDIRECT_H
