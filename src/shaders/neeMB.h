#ifndef NEEMB_H
#define NEEMB_H

#include "shader.h"
#include "../core/hemisphericalsampler.h"

// Motion Blur NEE: samples camera at different times during shutter interval
class NEEMB : public Shader
{
public:
    NEEMB();
    NEEMB(Vector3D bgColor_, int maxDepth_, int numTimeSamples_, 
          const Vector3D& cameraVelocity_);

    Vector3D computeColor(const Ray& r,
        const std::vector<Shape*>& objList,
        const std::vector<LightSource*>& lsList) const;

private:
    int maxDepth;
    int numTimeSamples;
    Vector3D cameraVelocity;
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

#endif // NEEMB_H
