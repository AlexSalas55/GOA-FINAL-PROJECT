#include <iostream>
#include <algorithm>

#include "core/film.h"
#include "core/matrix4x4.h"
#include "core/ray.h"
#include "core/utils.h"
#include "core/scene.h"


#include "shapes/sphere.h"
#include "shapes/infiniteplan.h"

#include "cameras/ortographic.h"
#include "cameras/perspective.h"

#include "shaders/intersectionshader.h"
#include "shaders/depthshader.h"
#include "shaders/normalshader.h"

#include "shaders/whittedintegrator.h"
#include "shaders/hemisphericaldirect.h"
#include "shaders/areadirect.h"
#include "shaders/purepathtracer.h"
#include "shaders/nee.h"
#include "shaders/areadirect-DOF.h"
#include "shaders/neeDOF.h"
#include "shaders/areadirectMB.h"


#include "materials/phong.h"
#include "materials/emissive.h"
#include "materials/mirror.h"
#include "materials/transmissive.h"

#include <chrono>

using namespace std::chrono;

typedef std::chrono::duration<double, std::milli> durationMs;


void buildSceneCornellBox(Camera*& cam, Film*& film,
    Scene myScene)
{
    /* **************************** */
/* Declare and place the camera */
/* **************************** */
    Matrix4x4 cameraToWorld = Matrix4x4::translate(Vector3D(0, 0, -3));
    double fovDegrees = 60;
    double fovRadians = Utils::degreesToRadians(fovDegrees);
    cam = new PerspectiveCamera(cameraToWorld, fovRadians, *film);

    /* ********* */
    /* Materials */
    /* ********* */
    Material* redDiffuse = new Phong(Vector3D(0.7, 0.2, 0.3), Vector3D(0, 0, 0), 100);
    Material* greenDiffuse = new Phong(Vector3D(0.2, 0.7, 0.3), Vector3D(0, 0, 0), 100);
    Material* greyDiffuse = new Phong(Vector3D(0.8, 0.8, 0.8), Vector3D(0, 0, 0), 100);
    Material* blueGlossy_20 = new Phong(Vector3D(0.2, 0.3, 0.8), Vector3D(0.8, 0.8, 0.8), 20);
    Material* blueGlossy_80 = new Phong(Vector3D(0.2, 0.3, 0.8), Vector3D(0.8, 0.8, 0.8), 80);
    Material* cyandiffuse = new Phong(Vector3D(0.2, 0.8, 0.8), Vector3D(0, 0, 0), 100);
    Material* pinkGlossy = new Phong(Vector3D(1.0, 0.4, 0.8), Vector3D(0.8, 0.8, 0.8), 50);



    //Task 5.3
    Material* mirror = new Mirror();
    //Material* mirror2 = new Mirror();

    //Task 5.4
    Material* transmissive = new Transmissive(0.7);
    //Material* trasnmissive2 = new Transmissive(0.7);


    /* ******* */
    /* Objects */
    /* ******* */
    double offset = 3.0;
    Matrix4x4 idTransform;
    // Construct the Cornell Box
    Shape* leftPlan = new InfinitePlan(Vector3D(-offset - 1, 0, 0), Vector3D(1, 0, 0), redDiffuse);
    Shape* rightPlan = new InfinitePlan(Vector3D(offset + 1, 0, 0), Vector3D(-1, 0, 0), greenDiffuse);
    Shape* topPlan = new InfinitePlan(Vector3D(0, offset, 0), Vector3D(0, -1, 0), greyDiffuse);
    Shape* bottomPlan = new InfinitePlan(Vector3D(0, -offset, 0), Vector3D(0, 1, 0), greyDiffuse);
    Shape* backPlan = new InfinitePlan(Vector3D(0, 0, 3 * offset), Vector3D(0, 0, -1), greyDiffuse);

    myScene.AddObject(leftPlan);
    myScene.AddObject(rightPlan);
    myScene.AddObject(topPlan);
    myScene.AddObject(bottomPlan);
    myScene.AddObject(backPlan);


    // Place the Spheres and square inside the Cornell Box
    double radius = 1;
    Matrix4x4 sphereTransform1;
    sphereTransform1 = Matrix4x4::translate(Vector3D(1.5, -offset + radius, 6));
    Shape* s1 = new Sphere(radius, sphereTransform1, redDiffuse);
    //Shape* s1 = new Sphere(radius, sphereTransform1, mirror2); 

    Matrix4x4 sphereTransform2;
    sphereTransform2 = Matrix4x4::translate(Vector3D(-1.5, -offset + 3 * radius, 4));
    //Shape* s2 = new Sphere(radius, sphereTransform2, blueGlossy_20);
    Shape* s2 = new Sphere(radius, sphereTransform2, redDiffuse);

    //Shape* square = new Square(Vector3D(offset + 0.999, -offset-0.2, 3.0), Vector3D(0.0, 4.0, 0.0), Vector3D(0.0, 0.0, 2.0), Vector3D(-1.0, 0.0, 0.0), cyandiffuse);
    Shape* square = new Square(Vector3D(offset + 0.999, -offset - 0.2, 3.0), Vector3D(0.0, 4.0, 0.0), Vector3D(0.0, 0.0, 2.0), Vector3D(-1.0, 0.0, 0.0), mirror);

    myScene.AddObject(s1);
    myScene.AddObject(s2);
    myScene.AddObject(square);

    PointLightSource* myPointLight = new PointLightSource(Vector3D(0, 2.5, 3.0), Vector3D(2.0));
    myScene.AddPointLight(myPointLight);
    //PointLightSource* secondLight = new PointLightSource(Vector3D(2.0, 2.5, 3.0), Vector3D(0.0, 2.0, 0.0));
    //myScene.AddPointLight(secondLight);
    //PointLightSource* thirdLight = new PointLightSource(Vector3D(-2.0, 2.5, 3.0), Vector3D(0.0, 0.0, 2.0));
    //myScene.AddPointLight(thirdLight);

}

//void buildSceneCornellBox2(Camera*& cam, Film*& film,
//    Scene myScene)
//{
//    /* **************************** */
///* Declare and place the camera */
///* **************************** */
//    Matrix4x4 cameraToWorld = Matrix4x4::translate(Vector3D(0, 0, -3));
//    double fovDegrees = 60;
//    double fovRadians = Utils::degreesToRadians(fovDegrees);
//    cam = new PerspectiveCamera(cameraToWorld, fovRadians, *film);
//
//    /* ********* */
//    /* Materials */
//    /* ********* */
//    Material* redDiffuse = new Phong(Vector3D(0.7, 0.2, 0.3), Vector3D(0, 0, 0), 100);
//    Material* greenDiffuse = new Phong(Vector3D(0.2, 0.7, 0.3), Vector3D(0, 0, 0), 100);
//    Material* greyDiffuse = new Phong(Vector3D(0.8, 0.8, 0.8), Vector3D(0, 0, 0), 100);
//    Material* blueGlossy_20 = new Phong(Vector3D(0.2, 0.3, 0.8), Vector3D(0.2, 0.2, 0.2), 20);
//    Material* blueGlossy_80 = new Phong(Vector3D(0.2, 0.3, 0.8), Vector3D(0.2, 0.2, 0.2), 80);
//    Material* cyandiffuse = new Phong(Vector3D(0.2, 0.8, 0.8), Vector3D(0, 0, 0), 100);
//    Material* emissive = new Emissive(Vector3D(25, 25, 25), Vector3D(0.5));
//    Material* pinkGlossy = new Phong(Vector3D(1.0, 0.2, 0.7), Vector3D(0.2, 0.2, 0.2), 20);
//
//
//    Material* mirror = new Mirror();
//    Material* transmissive = new Transmissive(0.7);
//
//    /* ******* */
//    /* Objects */
//    /* ******* */
//    double offset = 3.0;
//    Matrix4x4 idTransform;
//    // Construct the Cornell Box
//    Shape* leftPlan = new InfinitePlan(Vector3D(-offset - 1, 0, 0), Vector3D(1, 0, 0), redDiffuse);
//    Shape* rightPlan = new InfinitePlan(Vector3D(offset + 1, 0, 0), Vector3D(-1, 0, 0), greenDiffuse);
//    Shape* topPlan = new InfinitePlan(Vector3D(0, offset, 0), Vector3D(0, -1, 0), greyDiffuse);
//    Shape* bottomPlan = new InfinitePlan(Vector3D(0, -offset, 0), Vector3D(0, 1, 0), greyDiffuse);
//    Shape* backPlan = new InfinitePlan(Vector3D(0, 0, 3 * offset), Vector3D(0, 0, -1), greyDiffuse);
//    Shape* square_emissive = new Square(Vector3D(-1.0, 3.0, 3.0), Vector3D(2.0, 0.0, 0.0), Vector3D(0.0, 0.0, 2.0), Vector3D(0.0, -1.0, 0.0), emissive);
//
//
//    myScene.AddObject(leftPlan);
//    myScene.AddObject(rightPlan);
//    myScene.AddObject(topPlan);
//    myScene.AddObject(bottomPlan);
//    myScene.AddObject(backPlan);
//    myScene.AddObject(square_emissive);
//
//
//    // Place the Spheres inside the Cornell Box
//    double radius = 1;
//    Matrix4x4 sphereTransform1;
//    sphereTransform1 = Matrix4x4::translate(Vector3D(1.5, -offset + radius, 6));
//    Shape* s1 = new Sphere(radius, sphereTransform1, pinkGlossy);
//
//    Matrix4x4 sphereTransform2;
//    sphereTransform2 = Matrix4x4::translate(Vector3D(-1.5, -offset + 3 * radius, 4));
//    Shape* s2 = new Sphere(radius, sphereTransform2, pinkGlossy);
//
//    Shape* square = new Square(Vector3D(offset + 0.999, -offset - 0.2, 3.0), Vector3D(0.0, 4.0, 0.0), Vector3D(0.0, 0.0, 2.0), Vector3D(-1.0, 0.0, 0.0), cyandiffuse);
//
//    myScene.AddObject(s1);
//    myScene.AddObject(s2);
//    myScene.AddObject(square);
//}
void buildSceneDepthOfField(Camera*& cam, Film*& film, Scene myScene)
{
    /* **************************** */
    /* Declare and place the camera */
    /* **************************** */
    // We position the camera further back to view the long row of spheres
    Matrix4x4 cameraToWorld = Matrix4x4::translate(Vector3D(0, 1, -8));
    double fovDegrees = 50; // Slightly narrower FOV helps compress depth visually
    double fovRadians = Utils::degreesToRadians(fovDegrees);
    cam = new PerspectiveCamera(cameraToWorld, fovRadians, *film);

    /* ********* */
    /* Materials */
    /* ********* */
    // Using Glossy (Phong with exponent) materials to create nice highlights for Bokeh
    Material* whiteDiffuse = new Phong(Vector3D(0.8, 0.8, 0.8), Vector3D(0, 0, 0), 100);
    Material* redGlossy = new Phong(Vector3D(0.9, 0.2, 0.2), Vector3D(0.4, 0.4, 0.4), 40);
    Material* greenGlossy = new Phong(Vector3D(0.2, 0.9, 0.2), Vector3D(0.4, 0.4, 0.4), 40);
    Material* blueGlossy = new Phong(Vector3D(0.2, 0.2, 0.9), Vector3D(0.4, 0.4, 0.4), 40);
    Material* goldGlossy = new Phong(Vector3D(0.9, 0.7, 0.2), Vector3D(0.5, 0.5, 0.5), 60);

    // Bright light source to ensure spheres are well lit
    Material* strongEmissive = new Emissive(Vector3D(30, 30, 30), Vector3D(1.0));

    /* ******* */
    /* Objects */
    /* ******* */

    // 1. The Floor (Infinite Plan)
    // Placed slightly lower to accommodate the spheres
    Shape* floorPlan = new InfinitePlan(Vector3D(0, -2, 0), Vector3D(0, 1, 0), whiteDiffuse);
    myScene.AddObject(floorPlan);

    // 2. The Light Source
    // A large ceiling light to cast highlights on the spheres
    Shape* ceilingLight = new Square(Vector3D(-2.0, 8.0, 5.0), Vector3D(4.0, 0.0, 0.0), Vector3D(0.0, 0.0, 15.0), Vector3D(0.0, -1.0, 0.0), strongEmissive);
    myScene.AddObject(ceilingLight);

    // 3. The Spheres (The main subjects)
    double radius = 1.0;

    // Sphere 1: Foreground (Close to Camera) - Z = -2
    // If you focus here, the back spheres will be very blurry.
    Matrix4x4 t1 = Matrix4x4::translate(Vector3D(-1.5, -1, -2));
    Shape* s1 = new Sphere(radius, t1, redGlossy);
    myScene.AddObject(s1);

    // Sphere 2: Mid-Ground - Z = 2
    // A balanced focus point.
    Matrix4x4 t2 = Matrix4x4::translate(Vector3D(-0.5, -1, 2));
    Shape* s2 = new Sphere(radius, t2, greenGlossy);
    myScene.AddObject(s2);

    // Sphere 3: Background - Z = 7
    Matrix4x4 t3 = Matrix4x4::translate(Vector3D(0.5, -1, 7));
    Shape* s3 = new Sphere(radius, t3, blueGlossy);
    myScene.AddObject(s3);

    // Sphere 4: Far Background - Z = 14
    // This will be extremely blurry if you focus on the Red sphere.
    Matrix4x4 t4 = Matrix4x4::translate(Vector3D(1.5, -1, 14));
    Shape* s4 = new Sphere(radius, t4, goldGlossy);
    myScene.AddObject(s4);
}


void buildMotionBlurScene(Camera*& cam, Film*& film, Scene myScene)
{
    /* **************************** */
    /* 1. Camera Setup              */
    /* **************************** */
    // Position camera back (-6) and slightly up (1) to see the depth
    Matrix4x4 cameraToWorld = Matrix4x4::translate(Vector3D(0.0, 1.0, -6.0));
    double fovDegrees = 55; // Slightly wider to see more objects
    double fovRadians = Utils::degreesToRadians(fovDegrees);
    cam = new PerspectiveCamera(cameraToWorld, fovRadians, *film);

    /* ********* */
    /* Materials */
    /* ********* */
    // Glossy materials for moving objects (creates nice streaks)
    Material* glossyRed = new Phong(Vector3D(0.9, 0.1, 0.1), Vector3D(0.8, 0.8, 0.8), 80);
    Material* glossyBlue = new Phong(Vector3D(0.1, 0.2, 0.9), Vector3D(0.8, 0.8, 0.8), 80);
    Material* glossyYellow = new Phong(Vector3D(0.8, 0.8, 0.1), Vector3D(0.8, 0.8, 0.8), 80);
    Material* glossyCyan = new Phong(Vector3D(0.1, 0.8, 0.8), Vector3D(0.8, 0.8, 0.8), 80);

    // Matte materials for the environment
    Material* floorMat = new Phong(Vector3D(0.25, 0.25, 0.25), Vector3D(0.1, 0.1, 0.1), 20);
    Material* wallMat = new Phong(Vector3D(0.6, 0.6, 0.6), Vector3D(0.0, 0.0, 0.0), 10);
    Material* pillarMat = new Phong(Vector3D(0.4, 0.4, 0.4), Vector3D(0.1, 0.1, 0.1), 20);

    // Strong Light
    Material* lightMat = new Emissive(Vector3D(35, 35, 35), Vector3D(1.0));

    /* ******* */
    /* Objects */
    /* ******* */

    // --- ENVIRONMENT ---

    // 1. Floor
    Shape* floor = new InfinitePlan(Vector3D(0, -2.0, 0), Vector3D(0, 1, 0), floorMat);
    myScene.AddObject(floor);

    // 2. Back Wall (Static reference, Z = 14)
    Shape* backWall = new InfinitePlan(Vector3D(0, 0, 14), Vector3D(0, 0, -1), wallMat);
    myScene.AddObject(backWall);

    // 3. Ceiling Light (Wide area light to catch highlights)
    Shape* light = new Square(Vector3D(-5.0, 8.0, 5.0), Vector3D(10.0, 0.0, 0.0), Vector3D(0.0, 0.0, 10.0), Vector3D(0.0, -1.0, 0.0), lightMat);
    myScene.AddObject(light);

    // --- OBJECTS AT DIFFERENT DEPTHS ---

    // ZONE 1: FOREGROUND (Z = -3 to -1) -> EXTREME BLUR
    // These are very close to the camera.
    Matrix4x4 tRed = Matrix4x4::translate(Vector3D(-2.5, -0.8, -3.0));
    Shape* sRed = new Sphere(0.8, tRed, glossyRed);
    myScene.AddObject(sRed);

    Matrix4x4 tSmallCyan = Matrix4x4::translate(Vector3D(1.5, -1.2, -1.0));
    Shape* sCyan = new Sphere(0.5, tSmallCyan, glossyCyan);
    myScene.AddObject(sCyan);


    // ZONE 2: MID-GROUND (Z = 3 to 6) -> MEDIUM BLUR
    // These will show clear motion, but not as extreme as the foreground.
    Matrix4x4 tYellow = Matrix4x4::translate(Vector3D(0.5, 0.0, 4.0));
    Shape* sYellow = new Sphere(1.5, tYellow, glossyYellow);
    myScene.AddObject(sYellow);


    // ZONE 3: BACKGROUND (Z = 11 to 14) -> SHARP / STATIC
    // These are far away against the wall. Parallax is minimal.
    Matrix4x4 tBlue = Matrix4x4::translate(Vector3D(3.0, 1.5, 12.0));
    Shape* sBlue = new Sphere(2.5, tBlue, glossyBlue);
    myScene.AddObject(sBlue);

    // A large "pillar" structure in the back corner to fill space
    Matrix4x4 tPillar = Matrix4x4::translate(Vector3D(-4.0, 0.0, 13.0));
    Shape* sPillar = new Sphere(3.0, tPillar, pillarMat);
    myScene.AddObject(sPillar);
}


void buildSceneSphere(Camera*& cam, Film*& film,
    Scene myScene)
{
    /* **************************** */
      /* Declare and place the camera */
      /* **************************** */
      // By default, this gives an ID transform
      //  which means that the camera is located at (0, 0, 0)
      //  and looking at the "+z" direction
    Matrix4x4 cameraToWorld;
    double fovDegrees = 60;
    double fovRadians = Utils::degreesToRadians(fovDegrees);
    cam = new PerspectiveCamera(cameraToWorld, fovRadians, *film);


    /* ************************** */
    /* DEFINE YOUR MATERIALS HERE */
    /* ************************** */
    Material* green_100 = new Phong(Vector3D(0.2, 0.7, 0.3), Vector3D(0.2, 0.6, 0.2), 50);

    // Define and place a sphere
    Matrix4x4 sphereTransform1;
    sphereTransform1 = sphereTransform1.translate(Vector3D(-1.25, 0.5, 4.0));
    Shape* s1 = new Sphere(1.0, sphereTransform1, green_100);

    // Define and place a sphere
    Matrix4x4 sphereTransform2;
    sphereTransform2 = sphereTransform2.translate(Vector3D(1.25, 0.0, 6));
    Shape* s2 = new Sphere(1.25, sphereTransform2, green_100);

    // Define and place a sphere
    Matrix4x4 sphereTransform3;
    sphereTransform3 = sphereTransform3.translate(Vector3D(1.0, -0.75, 3.5));
    Shape* s3 = new Sphere(0.25, sphereTransform3, green_100);

    // Store the objects in the object list
    myScene.AddObject(s1);
    myScene.AddObject(s2);
    myScene.AddObject(s3);

}

void raytrace(Camera*& cam, Shader*& shader, Film*& film,
    std::vector<Shape*>*& objectsList, std::vector<LightSource*>*& lightSourceList)
{

    double my_PI = 0.0;
    double n_estimations = 0.0;
    unsigned int sizeBar = 40;

    size_t resX = film->getWidth();
    size_t resY = film->getHeight();

    // Main raytracing loop
    // Out-most loop invariant: we have rendered lin lines
    for (size_t lin = 0; lin < resY; lin++)
    {
        // Show progression 
        double progress = (double)lin / double(resY);
        Utils::printProgress(progress);

        // Inner loop invariant: we have rendered col columns
        for (size_t col = 0; col < resX; col++)
        {
            // Compute the pixel position in NDC
            double x = (double)(col + 0.5) / resX;
            double y = (double)(lin + 0.5) / resY;
            // Generate the camera ray
            Ray cameraRay = cam->generateRay(x, y);
            Vector3D pixelColor = Vector3D(0.0);

            // Compute ray color according to the used shader
            pixelColor += shader->computeColor(cameraRay, *objectsList, *lightSourceList);

            // Store the pixel color
            film->setPixelValue(col, lin, pixelColor);
        }
    }


}

// Path Tracing Algorithm with multiple samples per pixel (spp)
void raytracePathTracer(Camera*& cam, Shader*& shader, Film*& film,
    std::vector<Shape*>*& objectsList, std::vector<LightSource*>*& lightSourceList, int spp)
{
    unsigned int sizeBar = 40;

    size_t resX = film->getWidth();
    size_t resY = film->getHeight();

    for (size_t lin = 0; lin < resY; lin++)
    {
        double progress = (double)lin / double(resY);
        Utils::printProgress(progress);

        for (size_t col = 0; col < resX; col++)
        {
            double x = (double)(col + 0.5) / resX;
            double y = (double)(lin + 0.5) / resY;

            Vector3D pixelColor = Vector3D(0.0);

            for (int s = 0; s < spp; s++) //iterate over the number of samples
            {
                Ray cameraRay = cam->generateRay(x, y);

                pixelColor += shader->computeColor(cameraRay, *objectsList, *lightSourceList);
            }

            pixelColor = pixelColor / (double)spp;

            film->setPixelValue(col, lin, pixelColor);
        }
    }
}


//------------TASK 1---------------------//
void PaintImage(Film* film)
{
    unsigned int sizeBar = 40;

    size_t resX = film->getWidth();
    size_t resY = film->getHeight();

    // Main Image Loop
    for (size_t lin = 0; lin < resY; lin++)
    {
        // Show progression
        if (lin % (resY / sizeBar) == 0)
            std::cout << ".";

        for (size_t col = 0; col < resX; col++)
        {
            //CHANGE...()
            Vector3D random_color = Vector3D((float)col / (float)resX, (float)lin / (float)resY, 1.0); //el valor maxim d'un color es 1.0, si es mes de 1 es crampeja           

            film->setPixelValue(col, lin, random_color);

        }
    }
}

int main()
{
    std::string separator = "\n----------------------------------------------\n";
    std::string separatorStar = "\n**********************************************\n";
    std::cout << separator << "RT-ACG - Ray Tracer for \"Advanced Computer Graphics\"" << separator << std::endl;

    Film* film;
    film = new Film(720, 512);


    Vector3D bgColor(0.0, 0.0, 0.0); // Background color (for rays which do not intersect anything)
    Vector3D intersectionColor(1, 0, 0);

    Vector3D cameraVelocity(2.0, 0.0, 0.0);

	// ----------------------- SHADERS -------------------------//

    Shader* DOFshader = new AreaDirectDOF(bgColor, 10, 10.21f, 0.5f);
    Shader* MBshader = new AreaDirectMB(bgColor, 10, 15, cameraVelocity); //Change 5 to 40


    // Build the scene---------------------------------------------------------
    Camera* cam;
    Scene myScene;


	// ------------------------------- Motion Blur Scene -------------------------//

    buildMotionBlurScene(cam, film, myScene); 
    auto start = high_resolution_clock::now();
    raytrace(cam, MBshader, film, myScene.objectsList, myScene.LightSourceList);


	//------------------------------- Depth of Field with Area Direct -------------------------//


	//buildSceneDepthOfField(cam, film, myScene);
 //   auto start = high_resolution_clock::now();
 //   int spp = 20;
 //   raytracePathTracer(cam, DOFshader, film, myScene.objectsList, myScene.LightSourceList, spp);

	//-----------------------------------------------------------------------------------------//


    auto stop = high_resolution_clock::now();
    // Save the final result to file
    std::cout << "\n\nSaving the result to file output.bmp\n" << std::endl;
    film->save();
    film->saveEXR();

    float durationS = (durationMs(stop - start) / 1000.0).count();
    std::cout << "FINAL_TIME(s): " << durationS << std::endl;


    std::cout << "\n\n" << std::endl;
    return 0;
}

