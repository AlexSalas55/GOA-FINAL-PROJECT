#ifndef MIRROR_MATERIAL_H
#define MIRROR_MATERIAL_H

#include "material.h"

class Mirror : public Material {
public:
    Mirror() {}               

    Vector3D getReflectance(const Vector3D& n, const Vector3D& wo, const Vector3D& wi) const override {
        return Vector3D(0.0); 
    }

    bool hasSpecular() const override { return true; } 
    bool hasTransmission() const override { return false; }
    bool hasDiffuseOrGlossy() const override { return false; }
    bool isEmissive() const override { return false; }

    Vector3D getDiffuseReflectance() const override { return Vector3D(0.0); }
	Vector3D getEmissiveRadiance() const override { return Vector3D(0.0); }

};

#endif // MIRROR_MATERIAL_H