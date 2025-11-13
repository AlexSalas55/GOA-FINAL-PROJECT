#include "neeDOF.h"
#include "../core/utils.h"
#include "../lightsources/arealightsource.h"
#include <algorithm>

#define PI 3.14159265358979323846
#define NumRandomPosition 30 // Muestras para DOF (el artículo recomienda 20-30)

NEEDOF::NEEDOF()
    : Shader(), maxDepth(4)
{ }

NEEDOF::NEEDOF(Vector3D bgColor_, int maxDepth_, float focalLength, float sensorWidth)
    : Shader(bgColor_), maxDepth(maxDepth_)
{ 

    this->focalLength = focalLength;
	this->sensorWidth = sensorWidth;    
}

Vector3D NEEDOF::computeColor(const Ray& r,
    const std::vector<Shape*>& objList,
    const std::vector<LightSource*>& lsList) const
{
    // CRÍTICO: Solo aplicar DOF en rayos primarios (depth == 0)
    if (r.depth == 0)
    {
        Vector3D color(0.0);

        // Calcular el punto focal a lo largo del rayo
        // Según el artículo: focalPoint = rayOrigin + rayDirection * focalLength
        Vector3D focalPoint = r.o + r.d * this->focalLength;

        // Muestrear múltiples posiciones en el disco de apertura
        for (int i = 0; i < NumRandomPosition; i++)
        {
            // Método de rechazo para distribución uniforme en disco
            // (como se explica en el artículo)
            double offsetX, offsetY;
            do {
                offsetX = 2.0 * ((double)rand() / RAND_MAX) - 1.0;
                offsetY = 2.0 * ((double)rand() / RAND_MAX) - 1.0;
            } while (offsetX * offsetX + offsetY * offsetY > 1.0);

            // Escalar por el radio de apertura (sensorWidth)
            offsetX *= this->sensorWidth;
            offsetY *= this->sensorWidth;

            // Construir sistema de coordenadas local perpendicular al rayo
            // Según el artículo: necesitamos vectores perpendiculares a la dirección del rayo
            Vector3D up(0, 1, 0);
            if (std::abs(r.d.y) > 0.99) {
                up = Vector3D(1, 0, 0);  // Evitar colinealidad
            }

            // Vector "right" perpendicular a r.d (producto cruz)
            Vector3D right = Vector3D(
                r.d.y * up.z - r.d.z * up.y,
                r.d.z * up.x - r.d.x * up.z,
                r.d.x * up.y - r.d.y * up.x
            ).normalized();

            // Vector "trueUp" perpendicular a r.d y right
            Vector3D trueUp = Vector3D(
                right.y * r.d.z - right.z * r.d.y,
                right.z * r.d.x - right.x * r.d.z,
                right.x * r.d.y - right.y * r.d.x
            ).normalized();

            // Nueva posición en el plano de apertura
            // Según el artículo: desplazar el origen del rayo en el plano de la lente
            Vector3D randomPosition = r.o + right * offsetX + trueUp * offsetY;

            // Nueva dirección: desde la posición aleatoria hacia el punto focal
            // Esto es clave según el artículo: todos los rayos convergen en el punto focal
            Vector3D newDirection = (focalPoint - randomPosition).normalized();

            // Crear rayo modificado con depth=1 para evitar re-aplicar DOF
            Ray modifiedRay(randomPosition, newDirection, 1);
            color += computeRadiance(modifiedRay, objList, lsList);
        }

        // Promediar todas las muestras
        return color / (double)NumRandomPosition;
    }
    else
    {
        // Para rayos secundarios (reflexiones, refracciones), NO aplicar DOF
        return computeRadiance(r, objList, lsList);
    }









    return computeRadiance(r, objList, lsList);
}

Vector3D NEEDOF::computeRadiance(const Ray& r,
    const std::vector<Shape*>& objList,
    const std::vector<LightSource*>& lsList) const
{
    // find the closest intersection
    Intersection its;
    if (!Utils::getClosestIntersection(r, objList, its))
    {
        return bgColor;
    }

    const Material& material = its.shape->getMaterial();
    Vector3D n = its.normal.normalized();
    Vector3D wo = (-r.d).normalized();

    // Le = x.emittedLight()
    Vector3D Le(0.0);
    if (material.isEmissive())
    {
        Le = material.getEmissiveRadiance();
    }

    // Lr = ReflectedRadiance(x, -r.d, MaxDepth)
    Vector3D Lr(0.0);
    Lr = reflectedRadiance(its.itsPoint, wo, n, material, r.depth, objList, lsList);
    
    
    return Le + Lr; //return Le  (emissive light) + Lr (reflected light = direct + indirect)
}

Vector3D NEEDOF::reflectedRadiance(const Vector3D& x, const Vector3D& wo, const Vector3D& n,
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
        Lind= computeColor(reflRay, objList, lsList);  // recursively compute light along the reflected ray
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
            Ray refrRay(x- n1 * Epsilon, wt, depth + 1);
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


Vector3D NEEDOF::directRadiance(const Vector3D& x, const Vector3D& wo, const Vector3D& n,
    const Material& material,
    const std::vector<Shape*>& objList,
    const std::vector<LightSource*>& lsList) const
{
    Vector3D Ldir(0.0);

    // Sample all area light sources
    for (auto areaLight : lsList)
    {
        // Le, y, pdf = light.GetRandomPoint()
        Vector3D y = areaLight->sampleLightPosition();
        Vector3D Le = areaLight->getIntensity();
        double lightArea = areaLight->getArea();
        double pdf = 1.0 / lightArea;

        // ωi = Direction(x, y)
        Vector3D L = y - x;
        double distance = L.length();
        
        if (distance <= 0.0)
            continue;

        Vector3D wi = L / distance;

        Vector3D lightNormal = areaLight->getNormal();

        // compute G(x, y) following the formula
        double G = (dot(n, wi) * dot(lightNormal, -wi)) / (distance * distance);

        // check visibility V(x,y)
        Ray shadowRay(x, wi, 0.0, Epsilon, distance - Epsilon); //x= its.itsPoint
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
            Ldir += Le * refl * G * V_s / pdf;
        }
    }
    return Ldir;
}

Vector3D NEEDOF::indirectRadiance(const Vector3D& x, const Vector3D& wo, const Vector3D& n,
    const Material& material, int depth,
    const std::vector<Shape*>& objList,
    const std::vector<LightSource*>& lsList) const
{
    Vector3D Lind(0.0);
   
    // ωi, pdf = SampleHemisphere(x.normal)
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

            // Lind = ReflectedRadiance(y, −ωi) * x.BRDF(ωi, ωo) * (x.normal·ωi) / pdf
            // Calculate contribution from ANY material type (diffuse, mirror, transmissive)
            Vector3D Ly = reflectedRadiance(its.itsPoint, newWo, hitNormal, 
                                            hitMaterial, newR.depth, objList, lsList);
            Vector3D brdf = material.getReflectance(n, wo, wi);

            Lind = Ly * brdf * dot(wi, n) / pdf;
        }
    }
    return Lind;
}
