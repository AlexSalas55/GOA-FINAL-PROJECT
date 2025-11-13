#ifndef TRANSMISSIVE_MATERIAL_H
#define TRANSMISSIVE_MATERIAL_H

#include "material.h"

class Transmissive : public Material {
public:
    Transmissive(double muT) : muT(muT) {}

    Vector3D getReflectance(const Vector3D&, const Vector3D&, const Vector3D&) const override {
        return Vector3D(0.0); 
    }

    bool hasSpecular() const override { return false; }
    bool hasTransmission() const override { return true; }
    bool hasDiffuseOrGlossy() const override { return false; }
    bool isEmissive() const override { return false; }

    Vector3D getDiffuseReflectance() const override { return Vector3D(0.0); }
    double getIndexOfRefraction() const override { return muT; }

private:
	double muT; // Índice de refracción 
};


#endif // TRANSMISSIVE_MATERIAL_H