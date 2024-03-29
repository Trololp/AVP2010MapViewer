#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>


// Файлы заголовков C RunTime
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <vector>
#include <algorithm>
#include "resource.h"
//#include "umHalf.h"

using namespace DirectX;
#pragma comment(lib, "HACKS.lib")
#pragma warning(disable : 4996)

//Macro
#define SFPS(pos) SetFilePointer(f, pos, 0, FILE_BEGIN)
#define SFPC(pos) SetFilePointer(f, pos, 0, FILE_CURRENT)
#define READ(v) ReadFile(f, &(v), sizeof(v), &a, NULL)
#define READP(p, n) ReadFile(f, p, n, &a, NULL)
#define WRITE(v) WriteFile(f2, &(v), sizeof(v), &a, NULL)
#define WRITEP(p, n) WriteFile(f2, p, n, &a, NULL)
#define NULL_ROT { 0.0f, 0.0f, 0.0f, 1.0f }
//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------

struct avp_vtx
{
	XMFLOAT3 Pos;
	UINT8 COLOR[4]; //rgba
	SHORT Normal[4];
	SHORT Tangent1[4];
	SHORT Tangent2[4];
	XMFLOAT4 TexCoord1;
	XMFLOAT4 TexCoord2;
	XMFLOAT4 TexCoord3;
	XMFLOAT4 TexCoord4;
};

struct simple_vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

struct model_vtx
{
	XMFLOAT4 Pos;
	UINT8 BlendIndices[4];
	SHORT BlendWeight[4];
	SHORT Tangent[4];
	SHORT Binormal[4];
	SHORT Normal[4];
	XMFLOAT2 TexCoord1;
	float TextCoord2;
};


typedef struct
{
	DWORD id = 0;
	DWORD hash = 0;
	ID3D11ShaderResourceView* pSRView = nullptr;
	wchar_t* path = nullptr;
} avp_texture;

struct Map_mesh_info
{
	DWORD triangles_count;
	DWORD Vertex_buf_num;
	DWORD Min_vertex_index;
	DWORD Num_vertices;
	DWORD Index_buf_num;
	DWORD start_index;
	DWORD mat1_hash;
	DWORD mat2_hash;
	DWORD emod_id; // EMOD_id
	float bound_x;
	float bound_x_;
	float bound_y;
	float bound_y_;
	float bound_z;
	float bound_z_;
};

struct Material
{
	DWORD mat_hash;
	DWORD mat_num;
	DWORD texture_diff_id = 0;
	DWORD texture_norm_id = 0;
	DWORD texture_diff2_id = 0;
	DWORD texture_alpha_id = 0;
	DWORD texture_5_id = 0;
	DWORD texture_6_id = 0;
	DWORD texture_7_id = 0;
	DWORD texture_8_id = 0;
	DWORD texture_9_id = 0;
	DWORD texture_10_id = 0;
	bool is_alpha;
};

struct bbox
{
	XMFLOAT3 p1;
	XMFLOAT3 p2;
	XMFLOAT4 Color;
	XMFLOAT4 rot = NULL_ROT;  //quaternion rotation
};

struct asura_bbox
{
	float x1;
	float x2;
	float y1;
	float y2;
	float z1;
	float z2;
};

struct line
{
	XMFLOAT3 p1;
	XMFLOAT3 p2;
	XMFLOAT4 Color;
};

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
};

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void Render();