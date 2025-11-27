#include "areadirectMB.h"
#include "../core/utils.h"
#include "../lightsources/arealightsource.h"

#define PI 3.14159265358979323846

AreaDirectMB::AreaDirectMB()
    : Shader(), numSamples(256), numTimeSamples(10), cameraVelocity(0, 0, 0)
{ }

AreaDirectMB::AreaDirectMB(Vector3D bgColor_, int numSamples_, int numTimeSamples_, 
                           const Vector3D& cameraVelocity_)
    : Shader(bgColor_), numSamples(numSamples_), numTimeSamples(numTimeSamples_), 
      cameraVelocity(cameraVelocity_)
{ }

Vector3D AreaDirectMB::computeColor(const Ray& r,
    const std::vector<Shape*>& objList,
    const std::vector<LightSource*>& lsList) const
{
    // Motion blur by averaging multiple time samples
    Vector3D finalColor(0.0);
    
    for (int t = 0; t < numTimeSamples; t++)
    {
        // Sample random time in shutter interval [0, 1]
        double time = (double)rand() / RAND_MAX;
        
        // Offset ray origin by camera velocity * time
        Vector3D offset = cameraVelocity * time;
        Ray offsetRay(r.o + offset, r.d, r.depth);
        
        finalColor += computeDirectIllumination(offsetRay, objList, lsList);
    }
    
    return finalColor / (double)numTimeSamples;
}

Vector3D AreaDirectMB::computeDirectIllumination(const Ray& r,
    const std::vector<Shape*>& objList,
    const std::vector<LightSource*>& lsList) const
{
    Intersection its;
    if (!Utils::getClosestIntersection(r, objList, its))
    {
        return bgColor;
    }

    const Material& material = its.shape->getMaterial();
    Vector3D n = its.normal.normalized();
    Vector3D wo = (-r.d).normalized();

    Vector3D color(0.0);

    // Emitted radiance
    if (material.isEmissive())
    {
        color += material.getEmissiveRadiance();
    }

    if (material.hasDiffuseOrGlossy())
    {
        // Direct illumination from area lights
        for (auto areaLight : lsList)
        {
            Vector3D Ldir(0.0);
            Vector3D Le = areaLight->getIntensity();
            Vector3D lightNormal = areaLight->getNormal();
            double lightArea = areaLight->getArea();
            double pdf = 1.0 / lightArea;

            for (int s = 0; s < numSamples; s++)
            {
                Vector3D y = areaLight->sampleLightPosition();
                Vector3D L = y - its.itsPoint;
                double distance = L.length();

                if (distance <= 0.0)
                    continue;

                Vector3D wi = L / distance;
                double G = (dot(n, wi) * dot(lightNormal, -wi)) / (distance * distance);

                Ray shadowRay(its.itsPoint, wi, 0.0, Epsilon, distance - Epsilon);
                bool isVisible = !Utils::hasIntersection(shadowRay, objList);

                if (G > 0.0 && isVisible)
                {
                    Vector3D refl = material.getReflectance(n, wo, wi);
                    Ldir += Le * refl * G / pdf;
                }
            }

            color += Ldir / (double)numSamples;
        }
    }

    return color;
}
