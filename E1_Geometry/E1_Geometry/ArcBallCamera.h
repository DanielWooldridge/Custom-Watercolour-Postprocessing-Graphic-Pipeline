#pragma once
#include "BaseShader.h"
#include "FPCamera.h"
using namespace DirectX;

class ArcBallCamera : FPCamera
{
public:
    ArcBallCamera();
    ~ArcBallCamera();

    void EnableArcballCamera(float speed, float radius, XMFLOAT3 target);
    void UpdateArcballCamera(float deltaTime, XMFLOAT2 mouseDelta, bool isRotating);

    void SetTarget(XMFLOAT3 target);
    void SetRadius(float radius);

private:
    XMFLOAT3 target;  
    float radius;      
    float rotationSpeed;
    float yaw, pitch;  
};

