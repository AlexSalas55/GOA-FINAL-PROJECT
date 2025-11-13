#ifndef AREADIRECTDOF_H
#define AREADIRECTDOF_H

#include "shader.h"

class AreaDirectDOF : public Shader
{
public:
    AreaDirectDOF();
    AreaDirectDOF(Vector3D bgColor_, int numSamples_, float focalLength, float sensorWidth );

    Vector3D computeColor(const Ray& r,
        const std::vector<Shape*>& objList,
        const std::vector<LightSource*>& lsList) const;

private:
    int numSamples;

    //---------------------------------------	// Focal length and sensor width (for depth of field)

    Vector3D computeColorInternal(const Ray& r,
        const std::vector<Shape*>& objList,
        const std::vector<LightSource*>& lsList) const;
    
    float focalLength;
    float sensorWidth;


    //---------------------------------------	
};

#endif // AREADIRECT_H
