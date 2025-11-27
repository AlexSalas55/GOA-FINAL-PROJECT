#include "areadirect-DOF.h"
#include "../core/utils.h"
#include "../lightsources/arealightsource.h"
#include <algorithm>
#include <cmath>

#define PI 3.14159265358979323846
#define NumRandomPosition 3  // Muestras para DOF (el artículo recomienda 20-30)
#define MAX_DEPTH 3  // Límite de profundidad de recursión

AreaDirectDOF::AreaDirectDOF()
    : Shader(), numSamples(20), focalLength(10.0f), sensorWidth(0.5f)
{
}

AreaDirectDOF::AreaDirectDOF(Vector3D bgColor_, int numSamples_, float focalLength_, float sensorWidth_)
    : Shader(bgColor_), numSamples(numSamples_)
{
    this->focalLength = focalLength_;
    this->sensorWidth = sensorWidth_;
}

Vector3D AreaDirectDOF::computeColor(const Ray& r,
    const std::vector<Shape*>& objList,
    const std::vector<LightSource*>& lsList) const
{
    // CRÍTICO: Solo aplicar DOF en rayos primarios (depth == 0)
    if (r.depth == 0)
    {
        Vector3D color(0.0);

        // Calcular el punto focal a lo largo del rayo
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
            color += computeColorInternal(modifiedRay, objList, lsList);
        }

        // Promediar todas las muestras
        return color / (double)NumRandomPosition;
    }
    else
    {
        // Para rayos secundarios (reflexiones, refracciones), NO aplicar DOF
        return computeColorInternal(r, objList, lsList);
    }
}

Vector3D AreaDirectDOF::computeColorInternal(const Ray& r,
    const std::vector<Shape*>& objList,
    const std::vector<LightSource*>& lsList) const
{
    // Límite de profundidad para evitar recursión infinita
    if (r.depth > MAX_DEPTH)
    {
        return bgColor;
    }

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

            // Manejar luces puntuales (área = 0)
            if (lightArea <= 0.0)
            {
                // Luz puntual - un solo sample
                Vector3D lightPos = areaLight->sampleLightPosition();
                Vector3D L = lightPos - its.itsPoint;
                double distance = L.length();

                if (distance > 0.0)
                {
                    Vector3D wi = L / distance;
                    double ndotwi = dot(n, wi);

                    if (ndotwi > 0.0)
                    {
                        Ray shadowRay(its.itsPoint, wi, r.depth, Epsilon, distance - Epsilon);
                        bool isVisible = !Utils::hasIntersection(shadowRay, objList);

                        if (isVisible)
                        {
                            Vector3D refl = material.getReflectance(n, wo, wi);
                            Ldir += Le * refl * ndotwi / (distance * distance);
                        }
                    }
                }
                continue;
            }

            double pdf = 1.0 / lightArea;

            // Reducir samples para rayos secundarios (optimización)
            int effectiveSamples = (r.depth <= 1) ? numSamples : std::max(1, numSamples / 10);

            for (int i = 0; i < effectiveSamples; i++)
            {
                // sample a random point on the light source
                Vector3D y = areaLight->sampleLightPosition();

                // direction from hit point to light sample
                Vector3D L = y - its.itsPoint;
                double distance = L.length();

                if (distance <= 0.0) continue;

                Vector3D wi = L / distance;

                // Validar dot products
                double ndotwi = dot(n, wi);
                double lndotwi = dot(lightNormal, -wi);

                if (ndotwi <= 0.0 || lndotwi <= 0.0) continue;

                // compute G(x, y) following the formula
                double G = (ndotwi * lndotwi) / (distance * distance);

                // check visibility V(x,y)
                Ray shadowRay(its.itsPoint, wi, r.depth, Epsilon, distance - Epsilon);
                bool isVisible = !Utils::hasIntersection(shadowRay, objList);

                if (isVisible && G > 0.0)
                {
                    Vector3D refl = material.getReflectance(n, wo, wi);
                    Ldir += Le * refl * G / pdf;
                }
            }

            // Promediar samples
            if (effectiveSamples > 0)
            {
                Ldir = Ldir / (double)effectiveSamples;
            }
        }

        // ambient term approximation for indirect illumination
        Vector3D Ia(0.1);
        Vector3D kd = material.getDiffuseReflectance();
        Vector3D Lamb = Ia * kd;

        color += Ldir + Lamb;
    }

    // Solo procesar reflexiones/refracciones si no alcanzamos MAX_DEPTH
    if (r.depth < MAX_DEPTH)
    {
        // specular reflection (Mirror)
        if (material.hasSpecular())
        {
            Vector3D wr = (2 * dot(n, wo) * n - wo).normalized();
            Ray reflRay(its.itsPoint + n * Epsilon, wr, r.depth + 1);
            color += computeColorInternal(reflRay, objList, lsList);
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
                color += computeColorInternal(refrRay, objList, lsList);
            }
            else
            {
                // Total internal reflection
                Vector3D wr = (2 * dot(n1, wo) * n1 - wo).normalized();
                Ray reflRay(its.itsPoint + n1 * Epsilon, wr, r.depth + 1);
                color += computeColorInternal(reflRay, objList, lsList);
            }
        }
    }

    return color;
}