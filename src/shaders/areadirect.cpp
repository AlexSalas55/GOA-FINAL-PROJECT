#include "areadirect.h"
#include "../core/utils.h"
#include "../lightsources/arealightsource.h"
#include <algorithm>

#define PI 3.14159265358979323846

AreaDirect::AreaDirect()
    : Shader(), numSamples(256)
{ }

AreaDirect::AreaDirect(Vector3D bgColor_, int numSamples_)
    : Shader(bgColor_), numSamples(numSamples_)
{ }

Vector3D AreaDirect::computeColor(const Ray& r,
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
        // direct illumination via area light sampling
        Vector3D Ldir(0.0);

		// iterate over area light sources
        for (auto areaLight : lsList)
        {

            Vector3D Le = areaLight->getIntensity();
            Vector3D lightNormal = areaLight->getNormal();
            double lightArea = areaLight->getArea();
            double pdf = 1.0 / lightArea; // Uniform sampling PDF

            for (int i = 0; i < numSamples; i++)
            {
                // sample a random point on the light source
                Vector3D y = areaLight->sampleLightPosition();

                // direction from hit point to light sample
                Vector3D L = y - its.itsPoint;
                double distance = L.length();
                
                if (distance <= 0.0) continue;

				Vector3D wi = L / distance; //noramalized direction to light

                // compute G(x, y) following the formula
                double G = (dot(n, wi) * dot(lightNormal, -wi)) / (distance * distance);

                // check visibility V(x,y)
                Ray shadowRay(its.itsPoint, wi, 0.0, Epsilon, distance - Epsilon);
				bool isVisible = !Utils::hasIntersection(shadowRay, objList); // 1 if visible, 0 if blocked

                double V_s;
                if (isVisible) {
                    V_s = 1.0; // light reaches the point
                }
                else {
                    V_s = 0.0; // blocked, in shadow
                }

				if (G > 0.0) // only consider positive contributions
                {
                    Vector3D refl = material.getReflectance(n, wo, wi);

                    // Le * reflectance * G* V / pdf
                    Ldir += Le * refl * G* V_s / pdf;
                }
            }
        }

        // average the samples
        Ldir = Ldir / (double)numSamples;

        // ambient term approximation for indirect illumination
        Vector3D Ia(0.1); // ambient intensity
        Vector3D kd = material.getDiffuseReflectance();
        Vector3D Lamb = Ia * kd;

        color += Ldir + Lamb;
    }

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
