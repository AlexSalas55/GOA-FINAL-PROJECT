#include "arealightsource.h"
#include <cstdlib>

AreaLightSource::AreaLightSource(Square* areaLightsource_) :
    myAreaLightsource(areaLightsource_)
{ }



Vector3D AreaLightSource::getIntensity() const
{
    return myAreaLightsource->getMaterial().getEmissiveRadiance();
}


Vector3D AreaLightSource::sampleLightPosition() const
{
    // Generate random point inside the area light source (rectangle)
    // Random point = corner + rand1 * v1 + rand2 * v2
    double rand1 = (double)rand() / RAND_MAX;  // [0, 1]
    double rand2 = (double)rand() / RAND_MAX;  // [0, 1]
    
    Vector3D randomPoint = myAreaLightsource->corner + 
                          rand1 * myAreaLightsource->v1 + 
                          rand2 * myAreaLightsource->v2;
    
    return randomPoint;
}

