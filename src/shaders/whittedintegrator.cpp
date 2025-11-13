#include "whittedintegrator.h"

#include "../core/utils.h"
#include "../core/intersection.h"
#include "../shapes/shape.h"
#include "../materials/material.h"
#include "../lightsources/lightsource.h"

#include <algorithm>
#include <cmath>

// constructor: fondo negro
WhittedIntegrator::WhittedIntegrator()
    : Shader(Vector3D(0.0)) {}

// constructor: color de fondo concreto
WhittedIntegrator::WhittedIntegrator(Vector3D bgColor_)
    : Shader(bgColor_) {}

Vector3D WhittedIntegrator::computeColor(const Ray& r,
    const std::vector<Shape*>& objList,
    const std::vector<LightSource*>& lsList) const
{
    // intersección más cercana
    Intersection its;
    if (!Utils::getClosestIntersection(r, objList, its)) {
        return bgColor; // rayo no golpea nada --> fondo negro
    }

    const Material& mat = its.shape->getMaterial();
    Vector3D n = its.normal.normalized(); // normal en el punto
    Vector3D wo = (-r.d).normalized();    // dirección hacia la cámara

    Vector3D color(0.0); // color pixel

    // Término ambiente  
    if (mat.hasDiffuseOrGlossy()) {
        double ambient = 0.1; // constante fija
		color += ambient * mat.getDiffuseReflectance(); //coeficiente difuso
    }
    

    if (mat.hasDiffuseOrGlossy()) {
        for (const LightSource* ls : lsList) { // ls es cada fuente de luz
            Vector3D lightPos = ls->sampleLightPosition();
            Vector3D Li = ls->getIntensity();

            Vector3D L = lightPos - its.itsPoint; // vector hacia la luz
            double dist = L.length();
            if (dist <= 0.0) continue; // luz en el mismo punto (evitar división por cero)
            Vector3D wi = L / dist; // dirección de la luz (normalizada)

            // shadow ray: checkea si hay algún objeto entre la luz y el punto (sombra)
            Ray shadowRay(its.itsPoint, wi, 0.0, Epsilon, dist - Epsilon);
            bool isVisible = !Utils::hasIntersection(shadowRay, objList); // 1 si es visible, 0 si está bloqueado

            double V_s;
            if (isVisible) {
                V_s = 1.0; // luz llega al punto
            } else {
                V_s = 0.0; // luz bloqueada / sombra
            }

            Vector3D fr = mat.getReflectance(n, wo, wi); // reflectancia Phong

            // Lo(x, wo) += L_s^i * fr(n, wi, wo) * (n · wi) * V_s(x)
            color += Li * fr * std::max(0.0, dot(n, wi)) * V_s; 
        }
    }

    // reflexión perfecta (Task 4.5.3) 
    if (mat.hasSpecular()) {
        Vector3D wr = (2 * dot(n, -r.d) * n - (-r.d)).normalized(); // r.d apunta a la cámara, invertimos (-r.d)
		Ray reflRay(its.itsPoint + n * Epsilon, wr, r.depth + 1);
        color += computeColor(reflRay, objList, lsList);
    }

	// transmisión perfecta (Task 4.5.4) 
    if (mat.hasTransmission()) {
        float muT = mat.getIndexOfRefraction();
        Vector3D n1= n;
        if (dot(n, wo) > 0) {
            n1 = n; 
        }
        else {
			n1 = -n;
			muT = 1.0 / (float)muT; //si el rayo sale del material, invertimos el indice de refracción
        }
        float radicand = 1 - muT * muT * (1 - dot(n1, wo) * dot(n1, wo));
        
        if (radicand >= 0) {
            Vector3D wt = (-muT * wo + n1 * (muT * dot(n1, wo) - sqrt(radicand))).normalized(); 
            Ray refrRay(its.itsPoint, wt, r.depth + 1);
            color += computeColor(refrRay, objList, lsList);
        } else {
            //en este caso reflexión total
            Vector3D wr = (2 * dot(n1, -r.d) * n1 - (-r.d)).normalized(); // r.d apunta a la cámara, invertimos (-r.d)
            Ray reflRay(its.itsPoint, wr, r.depth + 1);
            color += computeColor(reflRay, objList, lsList);
        }
    }
    return color;
}
