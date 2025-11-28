#include "neeMB.h"
#include "../core/utils.h"
#include "../lightsources/arealightsource.h"
#include <algorithm>

#define PI 3.14159265358979323846

NEEMB::NEEMB()
    : Shader(), maxDepth(4), numTimeSamples(10), cameraVelocity(0, 0, 0)
{ }

NEEMB::NEEMB(Vector3D bgColor_, int maxDepth_, int numTimeSamples_, 
             const Vector3D& cameraVelocity_)
    : Shader(bgColor_), maxDepth(maxDepth_), numTimeSamples(numTimeSamples_), 
      cameraVelocity(cameraVelocity_)
{ }

Vector3D NEEMB::computeColor(const Ray& r,
    const std::vector<Shape*>& objList,
    const std::vector<LightSource*>& lsList) const
{
    // Motion blur: average multiple time samples during shutter
    if (r.depth == 0)
    {
        Vector3D finalColor(0.0);
        
        for (int t = 0; t < numTimeSamples; t++)
        {
            // Random time in shutter interval [0, 1]
            double time = (double)rand() / RAND_MAX;
            
            // Offset ray by camera velocity * time
            Vector3D offset = cameraVelocity * time;
            Ray offsetRay(r.o + offset, r.d, r.depth);
            
            finalColor += computeRadiance(offsetRay, objList, lsList);
        }
        
        return finalColor / (double)numTimeSamples;
    }
    
    return computeRadiance(r, objList, lsList);
}

Vector3D NEEMB::computeRadiance(const Ray& r,
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

    Vector3D Le(0.0);
    if (material.isEmissive())
    {
        Le = material.getEmissiveRadiance();
    }

    Vector3D Lr(0.0);
    Lr = reflectedRadiance(its.itsPoint, wo, n, material, r.depth, objList, lsList);
    
    return Le + Lr;
}

Vector3D NEEMB::reflectedRadiance(const Vector3D& x, const Vector3D& wo, const Vector3D& n,
    const Material& material, int depth,
    const std::vector<Shape*>& objList,
    const std::vector<LightSource*>& lsList) const
{
    Vector3D Ldir(0.0);
    Vector3D Lind(0.0);

    if (material.hasDiffuseOrGlossy()) {
        Ldir = directRadiance(x, wo, n, material, objList, lsList);
        Lind = indirectRadiance(x, wo, n, material, depth, objList, lsList);
    }
    else if (material.hasSpecular()) {
        Vector3D wr = (2 * dot(n, wo) * n - wo).normalized();
        Ray reflRay(x + n * Epsilon, wr, depth + 1);
        Lind = computeColor(reflRay, objList, lsList);
    }
    else if (material.hasTransmission()) {
        float muT = material.getIndexOfRefraction();
        Vector3D n1 = n;

        if (dot(n, wo) < 0)
        {
            n1 = -n;
            muT = 1.0 / muT;
        }

        float radicand = 1 - muT * muT * (1 - dot(n1, wo) * dot(n1, wo));

        if (radicand >= 0)
        {
            Vector3D wt = (-muT * wo + n1 * (muT * dot(n1, wo) - sqrt(radicand))).normalized();
            Ray refrRay(x - n1 * Epsilon, wt, depth + 1);
            Lind += computeColor(refrRay, objList, lsList);
        }
        else
        {
            Vector3D wr = (2 * dot(n1, wo) * n1 - wo).normalized();
            Ray reflRay(x + n1 * Epsilon, wr, depth + 1);
            Lind += computeColor(reflRay, objList, lsList);
        }
    }

    return Ldir + Lind;
}

Vector3D NEEMB::directRadiance(const Vector3D& x, const Vector3D& wo, const Vector3D& n,
    const Material& material,
    const std::vector<Shape*>& objList,
    const std::vector<LightSource*>& lsList) const
{
    Vector3D Ldir(0.0);

    for (auto areaLight : lsList)
    {
        Vector3D y = areaLight->sampleLightPosition();
        Vector3D Le = areaLight->getIntensity();
        double lightArea = areaLight->getArea();
        double pdf = 1.0 / lightArea;

        Vector3D L = y - x;
        double distance = L.length();
        
        if (distance <= 0.0)
            continue;

        Vector3D wi = L / distance;
        Vector3D lightNormal = areaLight->getNormal();

        double G = (dot(n, wi) * dot(lightNormal, -wi)) / (distance * distance);

        Ray shadowRay(x, wi, 0.0, Epsilon, distance - Epsilon);
        bool isVisible = !Utils::hasIntersection(shadowRay, objList);

        double V_s = isVisible ? 1.0 : 0.0;

        if (G > 0.0)
        {
            Vector3D refl = material.getReflectance(n, wo, wi);
            Ldir += Le * refl * G * V_s / pdf;
        }
    }
    return Ldir;
}

Vector3D NEEMB::indirectRadiance(const Vector3D& x, const Vector3D& wo, const Vector3D& n,
    const Material& material, int depth,
    const std::vector<Shape*>& objList,
    const std::vector<LightSource*>& lsList) const
{
    Vector3D Lind(0.0);
   
    Vector3D wi = sampler.getSample(n);
    double pdf = 1.0 / (2.0 * PI);
    Ray newR(x, wi, depth + 1);

    if (depth < maxDepth){
        Intersection its;
        if (Utils::getClosestIntersection(newR, objList, its))
        {
            const Material& hitMaterial = its.shape->getMaterial();
            Vector3D hitNormal = its.normal.normalized();
            Vector3D newWo = (-newR.d).normalized();

            Vector3D Ly = reflectedRadiance(its.itsPoint, newWo, hitNormal, 
                                            hitMaterial, newR.depth, objList, lsList);
            Vector3D brdf = material.getReflectance(n, wo, wi);

            Lind = Ly * brdf * dot(wi, n) / pdf;
        }
    }
    return Lind;
}
