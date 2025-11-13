#include "purepathtracer.h"
#include "../core/utils.h"
#include <algorithm>

#define PI 3.14159265358979323846

PurePathTracer::PurePathTracer()
    : Shader(), maxDepth(4)
{ }

PurePathTracer::PurePathTracer(Vector3D bgColor_, int maxDepth_)
    : Shader(bgColor_), maxDepth(maxDepth_)
{ }

Vector3D PurePathTracer::computeColor(const Ray& r,
    const std::vector<Shape*>& objList,
    const std::vector<LightSource*>& lsList) const
{    
    // x = IntersectScene(r)
    Intersection its;
    if (!Utils::getClosestIntersection(r, objList, its))
    {
        return bgColor;
    }

    const Material& material = its.shape->getMaterial();
    Vector3D n = its.normal.normalized();
    Vector3D wo = (-r.d).normalized(); // direction to camera

    // Lo = x.emittedLight()
    Vector3D Lo(0.0);
    if (material.isEmissive())
    {
        Lo = material.getEmissiveRadiance();
    }

    if (material.hasDiffuseOrGlossy())
    {
        // ωi, pdf = SampleHemisphere(x.normal)
        Vector3D wi = sampler.getSample(n); //this calls hemisphericalSampler, which generates random directions on the hemisphere around the surface normal
        double pdf = 1.0 / (2.0 * PI); 

        // the ray bounces around the scene until a max number of bounces (maxDepth) is reached
        if (r.depth < maxDepth)
        {
            // Ray newR = Ray(x, ωi, r.depth+1)
            Ray newR(its.itsPoint, wi, r.depth + 1);

            // Lo += ComputeRadiance(newR, scene, MaxDepth) * x.BRDF(ωi, -ray.d) * (x.normal·ωi) / pdf
            Vector3D Li = computeColor(newR, objList, lsList); // Recursive call
            Vector3D brdf = material.getReflectance(n, wo, wi);

            Lo += Li * brdf * dot(wi, n) / pdf;
        }
    }

    // perfect specular reflection (Mirror)
    if (material.hasSpecular())
    {
        Vector3D wr = (2 * dot(n, wo) * n - wo).normalized();
        Ray reflRay(its.itsPoint + n * Epsilon, wr, r.depth + 1);
        Lo += computeColor(reflRay, objList, lsList);
    }

    // perfect transmission (Transmissive)
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
            Lo += computeColor(refrRay, objList, lsList);
        }
        else
        {
            Vector3D wr = (2 * dot(n1, wo) * n1 - wo).normalized();
            Ray reflRay(its.itsPoint + n1 * Epsilon, wr, r.depth + 1);
            Lo += computeColor(reflRay, objList, lsList);
        }
    }

    // return Lo
    return Lo;
}
