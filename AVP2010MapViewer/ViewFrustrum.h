#pragma once
#include "AVP2010MapViewer.h"

class ViewFrustum
{
public:
	ViewFrustum();
	ViewFrustum(const ViewFrustum&);
	virtual ~ViewFrustum();

	void ConsturctFrustum(float screenDepth, XMMATRIX projectionMatrix, XMMATRIX viewMatrix);

	bool CheckPoint(XMFLOAT3 v);
	bool CheckCube(XMFLOAT3 v, float r);
	void DumpFrustum();

private:
	XMVECTOR m_planes[6];
};

