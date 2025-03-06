#include "ArcBallCamera.h"
#include "Camera.h"
#include <DirectXMath.h>

using namespace DirectX;

ArcBallCamera::ArcBallCamera()
{

}

ArcBallCamera::~ArcBallCamera() 
{

}

void ArcBallCamera::UpdateArcballCamera(float deltaTime, Camera* camera)
{
    // Rotate around target at a fixed vertical angle
    theta += speed * deltaTime;  // Auto-rotate 

    // Convert spherical coordinates to Cartesian
    float x = radius * cosf(phi) * sinf(theta);
    float y = radius * sinf(phi);
    float z = radius * cosf(phi) * cosf(theta);

    // Compute new camera position
    XMFLOAT3 cameraPos = { target.x + x, target.y + y, target.z + z };

    // Update the camera position
    camera->setPosition(cameraPos.x, cameraPos.y, cameraPos.z);

    // "Look At" logic
    XMFLOAT3 direction = {
        target.x - cameraPos.x,
        target.y - cameraPos.y,
        target.z - cameraPos.z
    };

    // Convert to yaw & pitch
    float yaw = atan2(direction.x, direction.z);

    
    //pitch = 20.0f;  

    camera->setRotation(pitch, XMConvertToDegrees(yaw), 0.0f);

    camera->update();
}
