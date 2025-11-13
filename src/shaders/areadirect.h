#ifndef AREADIRECT_H
#define AREADIRECT_H

#include "shader.h"

class AreaDirect : public Shader
{
public:
    AreaDirect();
    AreaDirect(Vector3D bgColor_, int numSamples_);

    Vector3D computeColor(const Ray& r,
        const std::vector<Shape*>& objList,
        const std::vector<LightSource*>& lsList) const;

private:
    int numSamples;
};

#endif // AREADIRECT_H
