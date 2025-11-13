#include "phong.h"

#include <iostream>
#include <algorithm>
#include <cmath>

Phong::Phong()
{ }

Phong::Phong(Vector3D Kd_, Vector3D Ks_, float alpha_)
{
    rho_d = Kd_;
    Ks = Ks_;
    alpha = alpha_;

    // Ensure energy conservation: kd + ks ≤ 1
    double sumR = rho_d.x + Ks.x;
    double sumG = rho_d.y + Ks.y;
    double sumB = rho_d.z + Ks.z;

    if (sumR > 1.0) {
       rho_d.x /= sumR;
       Ks.x /= sumR;
    }
    if (sumG > 1.0) {
       rho_d.y /= sumG;
       Ks.y /= sumG;
    }
    if (sumB > 1.0) {
       rho_d.z /= sumB;
       Ks.z /= sumB;
    }
}


Vector3D Phong::getReflectance(const Vector3D& n, const Vector3D& wo,
    const Vector3D& wi) const {

    Vector3D wr = 2 * dot(n,wi) * n - wi;

    Vector3D refl = (rho_d / 3.14159265359) + ((alpha+2)/ (2 * 3.14159265359)) * Ks * pow((dot(wo, wr)),alpha);

    return refl;

};

double Phong::getIndexOfRefraction() const
{
    std::cout << "Warning! Calling \"Material::getIndexOfRefraction()\" for a non-transmissive material"
              << std::endl;

    return -1;
}


Vector3D Phong::getEmissiveRadiance() const
{
    return Vector3D(0.0);
}


Vector3D Phong::getDiffuseReflectance() const
{
    return rho_d;
}

