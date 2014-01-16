#pragma once

#include "BoundingVolume.h"
#include "Sphere.h"
#include <vector>

class AABB : public BoundingVolume
{
private:
	DirectX::XMFLOAT4	m_Bottom;
	DirectX::XMFLOAT4	m_Top;
	DirectX::XMFLOAT4	m_Bounds[8];
	std::vector<int>	m_Indices;
	Sphere				m_Sphere;
	DirectX::XMFLOAT4	m_HalfDiagonal;
	DirectX::XMFLOAT4	m_Size;
	
public:
	AABB(){}
	/**
	 * @param p_CenterPos the position in world space in m
	 * @param p_Size the halfsize of the box in m
	 */
	AABB( DirectX::XMFLOAT4 p_CenterPos, DirectX::XMFLOAT4 p_Size) : BoundingVolume()
	{
		m_Position = p_CenterPos;
		m_Size.x = p_Size.x;
		m_Size.y = p_Size.y;
		m_Size.z = p_Size.z;
		m_Size.w = p_Size.w;
		
		m_Type		= Type::AABBOX;

		calculateBounds();
	}
	/**
	* Destructor
	*/
	~AABB()
	{
		m_Indices.clear();
	}
	
	/**
	* Calculate corners, half diagonal and create bounding sphere for AABB.
	*/
	void calculateBounds()
	{
		using namespace DirectX;

		m_Bounds[0] = XMFLOAT4(- m_Size.x, - m_Size.y, - m_Size.z, 1.f);
		m_Bounds[1] = XMFLOAT4(+ m_Size.x, - m_Size.y, - m_Size.z, 1.f);
		m_Bounds[2] = XMFLOAT4(- m_Size.x, + m_Size.y, - m_Size.z, 1.f);
		m_Bounds[3] = XMFLOAT4(+ m_Size.x, + m_Size.y, - m_Size.z, 1.f);
		m_Bounds[4] = XMFLOAT4(- m_Size.x, - m_Size.y, + m_Size.z, 1.f);
		m_Bounds[5] = XMFLOAT4(+ m_Size.x, - m_Size.y, + m_Size.z, 1.f);
		m_Bounds[6] = XMFLOAT4(- m_Size.x, + m_Size.y, + m_Size.z, 1.f);
		m_Bounds[7] = XMFLOAT4(+ m_Size.x, + m_Size.y, + m_Size.z, 1.f);

		XMVECTOR vBot, vTop, vDiag;

		vBot = XMLoadFloat4(&m_Bounds[0]);
		vTop = XMLoadFloat4(&m_Bounds[7]);
		
		vDiag = vTop - vBot;
		vDiag *= 0.5f;

		m_Sphere.setRadius(XMVector3Length(vDiag).m128_f32[0]);
		m_Sphere.updatePosition(m_Position);

		DirectX::XMStoreFloat4(&m_HalfDiagonal, vDiag);
	}
	/**
	* Updates position for AABB with translation matrix.
	* @param p_translation, move the AABB in relative coordinates.
	*/
	void updatePosition(DirectX::XMFLOAT4X4& p_Translation)
	{
		DirectX::XMMATRIX tempTrans;

		tempTrans = XMLoadFloat4x4(&p_Translation);

		DirectX::XMVECTOR tPos;
		tPos = XMLoadFloat4(&m_Position);

		tPos = XMVector4Transform(tPos, tempTrans);

		XMStoreFloat4(&m_Position, tPos);

		calculateBounds();
	}
	/**
	 * Sets a new size for AABB and recalculates.
	 * @param p_Size, new size.
	 */
	void setSize(DirectX::XMFLOAT4 p_Size)
	{
		m_Size = p_Size;
		calculateBounds();
	}
	/**
	* @return the top corner in m
	*/
	DirectX::XMFLOAT4* getMax()
	{
		return &m_Bounds[7];
	}
	/**
	* @return the bottom corner in m
	*/
	DirectX::XMFLOAT4* getMin()
	{
		return &m_Bounds[0];
	}
	/**
	* @return a vector from center to top corner in m
	*/
	DirectX::XMFLOAT4* getHalfDiagonal()
	{
		return &m_HalfDiagonal;
	}
	/**
	* @return the sphere that surround the AABB
	*/
	Sphere*	getSphere()
	{
		return &m_Sphere;
	}
	/**
	 * Return a corner at the index specified.
	 * 
	 * @param p_Index index number in m_Bounds list
	 * @return a XMFLOAT4 corner.
	 */
	DirectX::XMFLOAT4 getBoundAt(unsigned p_Index)
	{
		return m_Bounds[p_Index];
	}
	/**
	 * Return a corner in world coordinates at the index specified.
	 * 
	 * @param p_Index index number in m_Bounds list
	 * @return a XMFLOAT4 corner.
	 */
	DirectX::XMFLOAT4 getBoundWorldCoordAt(unsigned p_Index)
	{
		return DirectX::XMFLOAT4(m_Bounds[p_Index].x + m_Position.x,m_Bounds[p_Index].y + m_Position.y, m_Bounds[p_Index].z + m_Position.z, 1.f);
	}
};
