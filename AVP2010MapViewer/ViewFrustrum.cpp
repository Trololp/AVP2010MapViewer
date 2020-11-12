#include "ViewFrustrum.h"
#include "Debug.h"


ViewFrustum::ViewFrustum()
{
}

ViewFrustum::ViewFrustum(const ViewFrustum &)
{
}


ViewFrustum::~ViewFrustum()
{
}

void ViewFrustum::ConsturctFrustum(float screenDepth, XMMATRIX projectionMatrix, XMMATRIX viewMatrix)
{
	float zMinimum, r;
	XMMATRIX matrix;
	
	
	// Calculate the minimum Z distance in the frustum.
	zMinimum = -projectionMatrix.r[3].m128_f32[2] / projectionMatrix.r[2].m128_f32[2];
	r = screenDepth / (screenDepth - zMinimum);
	projectionMatrix.r[2].m128_f32[2] = r;
	projectionMatrix.r[3].m128_f32[2] = -r * zMinimum;
	// Create the transposed frustum matrix from the view matrix and updated projection matrix.
	//D3DXMatrixMultiply(&matrix, &viewMatrix, &projectionMatrix);
	matrix = XMMatrixTranspose(XMMatrixMultiply(viewMatrix, projectionMatrix));
	// Calculate near plane of frustum.
	m_planes[0] = matrix.r[3] + matrix.r[2];
	m_planes[1] = matrix.r[3] - matrix.r[2];
	m_planes[2] = matrix.r[3] + matrix.r[0];
	m_planes[3] = matrix.r[3] - matrix.r[0];
	m_planes[4] = matrix.r[3] - matrix.r[1];
	m_planes[5] = matrix.r[3] + matrix.r[1];
	//D3DXPlaneNormalize(&m_planes[0], &m_planes[0]);
	m_planes[0] = XMPlaneNormalize(m_planes[0]);
	
	for (int i = 0; i < 6; i++)
	{
		m_planes[i] = XMPlaneNormalize(m_planes[i]);
	}

	return;
}

bool ViewFrustum::CheckPoint(XMFLOAT3 v)
{
	// Check if the point is inside all six planes of the view frustum.
	for (int i = 0; i<6; i++)
	{
		if (XMPlaneDotCoord(m_planes[i], { v.x, v.y, v.z, 1.0f }).m128_f32[0] < 0.0f)
		{
			return false;
		}
	}

	return true;
}

bool ViewFrustum::CheckCube(XMFLOAT3 v, float r)
{
	// Check if any one point of the cube is in the view frustum.
	for (int i = 0; i<6; i++)
	{
		if (XMPlaneDotCoord(m_planes[i], { (v.x - r), (v.y - r), (v.z - r), 1.0f }).m128_f32[0] >= 0.0f)
		{
			continue;
		}

		if (XMPlaneDotCoord(m_planes[i], { (v.x + r), (v.y - r), (v.z - r), 1.0f }).m128_f32[0] >= 0.0f)
		{
			continue;
		}

		if (XMPlaneDotCoord(m_planes[i],{ (v.x - r), (v.y + r), (v.z - r), 1.0f }).m128_f32[0] >= 0.0f)
		{
			continue;
		}

		if (XMPlaneDotCoord(m_planes[i],{ (v.x + r), (v.y + r), (v.z - r), 1.0f }).m128_f32[0] >= 0.0f)
		{
			continue;
		}

		if (XMPlaneDotCoord(m_planes[i],{ (v.x - r), (v.y - r), (v.z + r), 1.0f }).m128_f32[0] >= 0.0f)
		{
			continue;
		}

		if (XMPlaneDotCoord(m_planes[i],{ (v.x + r), (v.y - r), (v.z + r), 1.0f }).m128_f32[0] >= 0.0f)
		{
			continue;
		}

		if (XMPlaneDotCoord(m_planes[i],{ (v.x - r), (v.y + r), (v.z + r), 1.0f }).m128_f32[0] >= 0.0f)
		{
			continue;
		}

		if (XMPlaneDotCoord(m_planes[i],{ (v.x + r), (v.y + r), (v.z + r), 1.0f }).m128_f32[0] >= 0.0f)
		{
			continue;
		}

		return false;
	}

	return true;
}

void ViewFrustum::DumpFrustum()
{
	dbgprint("Frustum", "planes: \n");
	for (int i = 0; i < 6; i++)
	{
		dbgprint("Frustum", "%d : %f %f %f %f \n", i, m_planes[i].m128_f32[0], m_planes[i].m128_f32[1], m_planes[i].m128_f32[2], m_planes[i].m128_f32[3]);
	}
}
