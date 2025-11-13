#include "hemisphericaldirect.h"
#include "../core/utils.h"
#include <algorithm>
#include <cmath>

#define PI 3.14159265358979323846

HemisphericalDirect::HemisphericalDirect()
    : Shader(), numSamples(256)
{ }

HemisphericalDirect::HemisphericalDirect(Vector3D bgColor_, int numSamples_)
    : Shader(bgColor_), numSamples(numSamples_)
{ }

Vector3D HemisphericalDirect::computeColor(const Ray& r,
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
        // Direct illumination via hemispherical sampling
        Vector3D Ldir(0.0);
        double pdf = 1.0 / (2.0 * PI); // Uniform hemispherical sampling PDF

        for (int i = 0; i < numSamples; i++)
        {
            // Sample a random direction in the hemisphere
            Vector3D wi = sampler.getSample(n);

            // Create a shadow ray from the hit point in the sampled direction
            Ray shadowRay(its.itsPoint, wi);
            shadowRay.maxT = INFINITY; // Large value to reach any light

            // Check if the shadow ray hits an emissive surface
            Intersection lightIts;
            if (Utils::getClosestIntersection(shadowRay, objList, lightIts))
            {
                const Material& lightMaterial = lightIts.shape->getMaterial();
                if (lightMaterial.isEmissive())
                {   
                    // Get the emissive radiance from the light source
                    Vector3D Le_light = lightMaterial.getEmissiveRadiance();

                    Vector3D refl = material.getReflectance(n, wo, wi);

                    // Add the contribution
                    Ldir += Le_light * refl * dot(wi, n) / pdf;
                }
            }
        }

        // Average the samples
        Ldir = Ldir / (double)numSamples;

        // Ambient term approximation for indirect illumination
        // In this section we will assume Lind(x, ωo) = Ia*kd.
        Vector3D Ia(0.1); // Ambient intensity
        Vector3D kd = material.getDiffuseReflectance();
        Vector3D Lamb = Ia * kd;

        color += Ldir + Lamb;
    }

    //following the same logic as in WhittedIntegrator
     
    // specular reflection (Mirror)
    if (material.hasSpecular())
    {
        Vector3D wr = (2 * dot(n, wo) * n - wo).normalized();
        Ray reflRay(its.itsPoint + n * Epsilon, wr, r.depth + 1);
        color += computeColor(reflRay, objList, lsList);
    }

    // transmission (Transmissive)
    if (material.hasTransmission())
    {
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
            Ray refrRay(its.itsPoint - n1 * Epsilon, wt, r.depth + 1);
            color += computeColor(refrRay, objList, lsList);
        }
        else
        {
            Vector3D wr = (2 * dot(n1, wo) * n1 - wo).normalized();
            Ray reflRay(its.itsPoint + n1 * Epsilon, wr, r.depth + 1);
            color += computeColor(reflRay, objList, lsList);
        }
    }

    return color;
}
