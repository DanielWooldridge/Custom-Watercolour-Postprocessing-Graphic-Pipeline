#pragma once
#include <DirectXMath.h>

class ArcBallCamera
{
public:
    ArcBallCamera();
    ~ArcBallCamera();

    void UpdateArcballCamera(float deltaTime, class Camera* camera);

    void SetRadius(float r) {  radius = r; }
    void SetSpeed(float s) { speed = s; }
    void SetAngle(float angle) { pitch = angle; }
    void SetTarget(DirectX::XMFLOAT3 t) { target = t; }

private:
    float radius = 100.0f;
    float speed = 0.5f;
    float theta = 0.0f;
    const float phi = 0.3f; 
    DirectX::XMFLOAT3 target;
    float pitch;
};
