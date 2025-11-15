#ifndef NEEDOF_H
#define NEEDOF_H

#include "shader.h"
#include "../core/hemisphericalsampler.h"

class NEEDOF : public Shader
{
public:
    NEEDOF();

    // Enfoque manual (foco fijo)
    NEEDOF(Vector3D bgColor_, int maxDepth_, float focalLength, float sensorWidth);

    // Enfoque por coordenadas (no requiere focalLength)
    // focusPointWS: punto de enfoque en espacio de mundo
    NEEDOF(Vector3D bgColor_, int maxDepth_, float sensorWidth, const Vector3D& focusPointWS);

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

    // Si no hay punto de enfoque, se usa este foco fijo
    float focalLength = 0.0f;
    // Radio de apertura (controla la intensidad del desenfoque)
    float sensorWidth = 0.0f;

    // Enfoque por coordenadas (opcional)
    bool hasFocusPoint = false;
    Vector3D focusPointWS = Vector3D(0.0, 0.0, 0.0);
};

#endif // NEE_H
