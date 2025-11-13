#ifndef NEE_H
#define NEE_H

#include "shader.h"
#include "../core/hemisphericalsampler.h"

class NEE : public Shader
{
public:
    NEE();
    NEE(Vector3D bgColor_, int maxDepth_);

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
};

#endif // NEE_H
