#pragma once
#include <DirectXMath.h>

struct Light
{

	DirectX::XMFLOAT3	lightPos;
    DirectX::XMFLOAT3	lightColor;
	DirectX::XMFLOAT3	lightDirection;
    DirectX::XMFLOAT2	spotlightAngles;// X must be smaller than Y
    float				lightRange;
	float				lightIntensity;

	Light()
	{
		lightPos = DirectX::XMFLOAT3(0.f,0.f,0.f);
		lightColor = DirectX::XMFLOAT3(0.f,0.f,0.f);
		lightDirection = DirectX::XMFLOAT3(0.f,0.f,0.f);
		spotlightAngles = DirectX::XMFLOAT2(0.f,0.f);
		lightRange = 0.f;
		lightIntensity = 0.f;
	}

	Light( DirectX::XMFLOAT3 p_position, DirectX::XMFLOAT3 p_color,
		DirectX::XMFLOAT3 p_direction, DirectX::XMFLOAT2 p_angles,
		float p_range, float p_intensity )
	{
		lightPos = p_position;
		lightColor = p_color;
		DirectX::XMStoreFloat3(&lightDirection, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&p_direction)));
		spotlightAngles = p_angles;
		lightRange = p_range;
		lightIntensity = p_intensity;
	}
};