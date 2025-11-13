#ifndef NEEDOF_H
#define NEEDOF_H

#include "shader.h"
#include "../core/hemisphericalsampler.h"

class NEEDOF : public Shader
{
public:
    NEEDOF();
    NEEDOF(Vector3D bgColor_, int maxDepth_, float focalLength, float sensorWidth);

    Vector3D computeColor(const Ray& r,
        const std::vector<Shape*>& objList,
        const std::vector<LightSource*>& lsList) const;


private:
    int maxDepth;
    HemisphericalSampler sampler;

    Vector3D computeRadiance(const Ray& r,
        const std::vector<Shape*>& objList,
        const std::vector<LightSource*>& lsList) const;

    Vector3D reflectedRadiance(const Vector3D& x, const Vector3D& wo, const Vector3D& n,
        const Material& material, int depth,
        const std::vector<Shape*>& objList,
        const std::vector<LightSource*>& lsList) const;

    Vector3D directRadiance(const Vector3D& x, const Vector3D& wo, const Vector3D& n,
        const Material& material,
        const std::vector<Shape*>& objList,
        const std::vector<LightSource*>& lsList) const;

    Vector3D indirectRadiance(const Vector3D& x, const Vector3D& wo, const Vector3D& n,
        const Material& material, int depth,
        const std::vector<Shape*>& objList,
        const std::vector<LightSource*>& lsList) const;

    float focalLength;
    float sensorWidth;

};

#endif // NEE_H
