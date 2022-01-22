#include "Collisions.h"
#include "EMOD_load.h"
#include "ViewFrustrum.h"
#include "Console.h"
#include "Debug.h"


extern ID3D11Device*           g_pd3dDevice;
extern ID3D11DeviceContext*    g_pImmediateContext;
extern ID3D11VertexShader*     g_pVertexShader_s;
extern ID3D11PixelShader*      g_pPixelShader_s;
extern ID3D11InputLayout*      g_pVertexLayout_s;

extern EMOD_entry* g_emod_entrys;
extern unsigned int g_emod_count;
extern bool g_emod_loaded;
extern ViewFrustum				g_frustum;

extern Collisions*				g_pCollisions;

WORD g_cube_index_buff[36] = {
	// front
	0,1,2,
	0,2,3,
	// back
	4,6,5,
	4,7,6,
	// left
	4,5,1,
	4,1,0,
	// right
	3,2,6,
	3,6,7,
	//top
	1,5,6,
	1,6,2,
	// bottom
	4,0,3,
	4,3,7
};




DWORD Command_collision_emod_id(DWORD*);

Command cmd_collision_emod_id = {
	"collision_emod_id",
	CONCMD_RET_ZERO,
	CONARG_INT,
	Command_collision_emod_id,
	nullptr
};

DWORD Command_collision_emod_id(DWORD* args)
{
	if(g_pCollisions)
	{
		DWORD emod_id = args[0];
		
		if (emod_id == 9999)
		{
			g_pCollisions->m_draw_by_emod_id = false;
		}

		g_pCollisions->m_emod_id = emod_id;
		g_pCollisions->m_draw_by_emod_id = true;
	}
	return 0;
}

int Global_index_buffer_for_boxes(ID3D11Buffer** Index_buffer)
{
	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = 36 * sizeof(WORD);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = g_cube_index_buff;
	HRESULT hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, Index_buffer);
	if (FAILED(hr))
	{
		dbgprint("Collisions", "Global_index_buffer_for_boxes failed !\n");
		return 0;
	}
	return 1;
}

int Collisions_to_box(bbox* bb, ID3D11Buffer** vtx_buffer)
{
	HRESULT hr = S_OK;


	unsigned int bboxes_count = 1;
	unsigned int size_of_vertex_data = 8 * sizeof(simple_vertex) * bboxes_count; // box is 8 points 
	simple_vertex* vertices = (simple_vertex*)malloc(size_of_vertex_data);

	//dbgprint("Helpers", "Bbox: count: %d\n", bboxes_count);

	XMFLOAT4 clr0 = { 0.0f, 1.0f, 1.0f, 0.5f }; // top
	XMFLOAT4 clr1 = { 0.0f, 0.5f, 0.5f, 0.5f }; // bottom

	XMFLOAT3 p1 = bb->p1;
	XMFLOAT3 p2 = bb->p2;
	XMFLOAT3 a = p1;                        //     box         dx11 coordinate system
	XMFLOAT3 b = XMFLOAT3(p2.x, p1.y, p1.z);//     h-----g       z+ 
	XMFLOAT3 c = XMFLOAT3(p2.x, p1.y, p2.z);//    /|    /|	    /
	XMFLOAT3 d = XMFLOAT3(p1.x, p1.y, p2.z);//   e-----f |	   0-----x+
	XMFLOAT3 e = XMFLOAT3(p1.x, p2.y, p1.z);//   | d---|-c	   |  
	XMFLOAT3 f = XMFLOAT3(p2.x, p2.y, p1.z);//   |/    |/	   |
	XMFLOAT3 g = p2;						//   a-----b	   y+
	XMFLOAT3 h = XMFLOAT3(p1.x, p2.y, p2.z);

	vertices[0] = { a, clr0 };
	vertices[1] = { e, clr1 };
	vertices[2] = { f, clr1 };
	vertices[3] = { b, clr0 };
	vertices[4] = { d, clr0 };
	vertices[5] = { h, clr1 };
	vertices[6] = { g, clr1 };
	vertices[7] = { c, clr0 };


	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = size_of_vertex_data;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertices;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, vtx_buffer);
	if (FAILED(hr))
	{
		dbgprint("Collision", "Bbox CreateBuffer failed ! \n");
		return 0;
	}
	free(vertices);
	return 1;
}


Collisions::Collisions()
{
	if (!g_emod_loaded)
	{
		dbgprint("Collision", "no EMOD chunk loaded\n");
		//probably should not leave but ok ?
		return;
	}
		
	for (int i = 0; i < g_emod_count; i++)
	{
		Collision_Box cb;

		EMOD_entry* emod_p = &(g_emod_entrys[i]);

		cb.pos = emod_p->pos;
		cb.radius = emod_p->radius;
		cb.emod_id = i;

		switch (emod_p->some_flags)
		{
		case 0x0:
		case 0x4:
			cb.cb_type = COLLISION_BOX_REND;
			break;
		case 0xC:
			cb.cb_type = COLLISION_BOX_WATER;
			break;
		case 0x2A:
			cb.cb_type = COLLISION_BOX_COLL;
			dbgprint("Collisions", "flags: %08x name: %s emod_id: %d\n", emod_p->some_flags, emod_p->name, i);
			break;
		default:
			cb.cb_type = COLLISION_BOX_UNK;
			dbgprint("Collisions", "flags: %08x name: %s emod_id: %d\n", emod_p->some_flags, emod_p->name, i);
			break;
		}

		if (Collisions_to_box(&(emod_p->bb), &(cb.vtx_buf)))
		{
			m_boxes.push_back(cb);
		}
	}

	Global_index_buffer_for_boxes(&m_global_index_buffer);

}


Collisions::~Collisions()
{
	for (auto cb : m_boxes)
	{
		if (cb.vtx_buf)
			cb.vtx_buf->Release();
	}
		

	std::vector<Collision_Box>().swap(m_boxes);
}

void Collisions::Draw()
{
	if (!m_displayable)
		return;

	Collision_box_types curr_display_type;

	if (m_display_specific_type)
	{
		curr_display_type = m_display_this_type;
	}
	else
	{
		// basic collisions nothing fancy
		curr_display_type = COLLISION_BOX_UNK;
	}

	g_pImmediateContext->VSSetShader(g_pVertexShader_s, nullptr, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader_s, nullptr, 0);
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout_s);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT stride = sizeof(simple_vertex);
	UINT offset = 0;


	if (m_draw_by_emod_id)
	{
		if (m_emod_id >= 0 && m_emod_id < m_boxes.size())
		{
			g_pImmediateContext->IASetVertexBuffers(0, 1, &(m_boxes[m_emod_id].vtx_buf), &stride, &offset);
			g_pImmediateContext->IASetIndexBuffer(m_global_index_buffer, DXGI_FORMAT_R16_UINT, 0);

			g_pImmediateContext->DrawIndexed(36, 0, 0); // drawing cube
		}

		return;
	}

	for (auto cb : m_boxes)
	{

		if (cb.cb_type == curr_display_type)
		{
			if (g_frustum.CheckCube(cb.pos, cb.radius))
			{
				g_pImmediateContext->IASetVertexBuffers(0, 1, &(cb.vtx_buf), &stride, &offset);
				g_pImmediateContext->IASetIndexBuffer(m_global_index_buffer, DXGI_FORMAT_R16_UINT, 0);

				g_pImmediateContext->DrawIndexed(36, 0, 0); // drawing cube
			}
		}
	}
}
