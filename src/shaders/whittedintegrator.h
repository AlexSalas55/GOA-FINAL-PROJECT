#ifndef WHITTEDINTEGRATOR_H
#define WHITTEDINTEGRATOR_H

#include "shader.h"

class WhittedIntegrator : public Shader {
public:
    WhittedIntegrator();
    WhittedIntegrator(Vector3D bgColor_);
    Vector3D computeColor(const Ray &r, const std::vector<Shape*> &objList, const std::vector<LightSource*> &lsList) const override;
};

#endif // WHITTEDINTEGRATOR_H
