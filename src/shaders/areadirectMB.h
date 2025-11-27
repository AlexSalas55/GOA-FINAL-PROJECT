#ifndef AREADIRECTMB_H
#define AREADIRECTMB_H

#include "shader.h"

// Motion Blur by sampling camera at different times during shutter interval
class AreaDirectMB : public Shader
{
public:
    AreaDirectMB();
    AreaDirectMB(Vector3D bgColor_, int numSamples_, int numTimeSamples_, 
                 const Vector3D& cameraVelocity_);

    Vector3D computeColor(const Ray& r,
        const std::vector<Shape*>& objList,
        const std::vector<LightSource*>& lsList) const;

private:
    int numSamples;          // Light samples
    int numTimeSamples;      // Time samples (shutter samples)
    Vector3D cameraVelocity; // Camera movement per time unit
    
    Vector3D computeDirectIllumination(const Ray& r,
        const std::vector<Shape*>& objList,
        const std::vector<LightSource*>& lsList) const;
};

#endif // AREADIRECTMB_H
