#include "PHEN_loader.h"
#include "AVP2010MapViewer.h"
#include "Console.h"
#include "ViewFrustrum.h"
#include "Debug.h"

#define PHEN_BITSET_OF_INTEREST 0x0800 // alien jumping code uses this flags
#define PHEN_BITSET_OF_INTEREST2 0x02A // there you can stand on it


struct Asura40bHdr
{
	DWORD magic;
	UINT chunk_size;
	UINT ver1;
	UINT ver2;
	UINT amt_volumes;
	UINT points_arr_cnt; // size 0xC
	UINT amt_indexes; // size 0x8
	UINT amt_flags;
	UINT amt_mat_hashes;
	UINT amt_unk;
};

struct PHEN_struct_with_bboxes
{
	DWORD indexes_offset;
	DWORD point_offset;
	DWORD id1;
	DWORD id2;
	asura_bbox bb;
	DWORD flags_offset;
	DWORD unk;
	WORD  vol_flags; // like 0 or 1 all time?
};

struct PHEN_indexes
{
	WORD i0;
	WORD i1;
	WORD i2;
	WORD i3;
};

struct PHEN_draw_segment {
	XMFLOAT3 pos;
	float obj_radius;
	ID3D11Buffer* idx_buffer;
	UINT base_vertex_offset;
	UINT amt_indexes;
};

// externs
extern ID3D11Device*           g_pd3dDevice;
extern ID3D11DeviceContext*    g_pImmediateContext;
extern ID3D11VertexShader*     g_pVertexShader_s;
extern ID3D11PixelShader*      g_pPixelShader_s;
extern ID3D11InputLayout*      g_pVertexLayout_s;
extern std::vector <bbox>		g_bboxes;
extern std::vector <line>		g_lines;

extern ID3D11RasterizerState*  g_Env_RS;
extern ID3D11RasterizerState*  g_Wireframe_RS;

extern int read_padded_str(HANDLE file, char* dest);
extern wchar_t* g_path1;

extern ViewFrustum				g_frustum;

// PHEN stuff
PHEN_struct_with_bboxes* g_p_PHEN_volumes;
UINT g_PHEN_amt_vertex;
UINT g_PHEN_amt_volumes;
XMFLOAT3* g_p_PHEN_vertex;
PHEN_indexes* g_p_PHEN_index;
WORD* g_p_PHEN_flags;

//Render stuff
ID3D11Buffer* g_pPHEN_vertex_buffer;
PHEN_draw_segment* g_pPHEN_draw_segments;


void Dump_PHEN_volumes(PHEN_struct_with_bboxes* vols, UINT size)
{
	dbgprint(__FUNCTION__, "PHEN dump start amt: %d\n", size);

	UINT idx_sum = 0;

	for (UINT i = 0; i < size; i++)
	{
		PHEN_struct_with_bboxes* v = &(vols[i]);
		dbgprint(__FUNCTION__, "id: %d \n", i);
		dbgprint(__FUNCTION__, "indexes_id: %d point_offset: %d amt_index: %d id2: %d \n", v->indexes_offset, v->point_offset, v->id1, v->id2);
		dbgprint(__FUNCTION__, "{%f %f %f %f %f %f} \n", v->bb.x1, v->bb.x2, v->bb.y1, v->bb.y2, v->bb.z1, v->bb.z2);
		dbgprint(__FUNCTION__, "unk: %d flags_offset: %d vol_flags: %d\n", v->unk, v->flags_offset, v->vol_flags);
		idx_sum += v->id1;
	}

	//dbgprint(__FUNCTION__, "idx_sum: %d \n", idx_sum);

	return;
}

void test_bound_volume_draw(UINT id)
{
	PHEN_struct_with_bboxes* test_volume = &(g_p_PHEN_volumes[id]);
	
	bbox* bb = bbox_from_asura_bb(&(test_volume->bb), NULL_ROT, Colors::White);
	g_bboxes.push_back(*bb);
	delete bb;

	UINT base_vtx_offset = test_volume->point_offset;
	UINT base_idx_offset = test_volume->indexes_offset;

	UINT idx_amt = test_volume->id2;

	XMFLOAT4 clr_code = { 0.0f, 1.0f, 1.0f, 1.0f }; // cyan

	for (UINT i = 0; i < idx_amt; i++)
	{
		PHEN_indexes idx = g_p_PHEN_index[base_idx_offset + i];

		XMFLOAT3 v1 = g_p_PHEN_vertex[base_vtx_offset + idx.i0];
		XMFLOAT3 v2 = g_p_PHEN_vertex[base_vtx_offset + idx.i1];
		XMFLOAT3 v3 = g_p_PHEN_vertex[base_vtx_offset + idx.i2];


		if (idx.i3 == 0xFFFF)
		{
			g_lines.push_back({ v1, v2, clr_code });
			g_lines.push_back({ v2, v3, clr_code });
			g_lines.push_back({ v3, v1, clr_code }); // Triangle
		}
		else
		{
			XMFLOAT3 v4 = g_p_PHEN_vertex[base_vtx_offset + idx.i3];

			g_lines.push_back({ v1, v2, clr_code });
			g_lines.push_back({ v2, v3, clr_code });
			g_lines.push_back({ v3, v1, clr_code });
			g_lines.push_back({ v1, v4, clr_code });
			g_lines.push_back({ v4, v3, clr_code }); // Quad
		}

	}
}

bool PHEN_make_vertex_buffer()
{
	
	XMFLOAT4 clr_black = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMFLOAT4 clr_green = { 0.0f, 1.0f, 0.0f, 1.0f };
	XMFLOAT4 clr_red = { 1.0f, 0.0f, 0.0f, 1.0f };

	simple_vertex* big_buffer = (simple_vertex*)malloc(sizeof(simple_vertex)*g_PHEN_amt_vertex);

	if (!big_buffer) return false;

	for (UINT i = 0; i < g_PHEN_amt_vertex; i++)
	{
		big_buffer[i].Pos = g_p_PHEN_vertex[i];
		big_buffer[i].Color = clr_black;
	}


	//coloring intresting triangles
	for (UINT vol_id = 0; vol_id < g_PHEN_amt_volumes; vol_id++)
	{
		UINT flags_offset = g_p_PHEN_volumes[vol_id].flags_offset;
		UINT points_offset = g_p_PHEN_volumes[vol_id].point_offset;
		UINT index_offset = g_p_PHEN_volumes[vol_id].indexes_offset;
		DWORD idk_bool = g_p_PHEN_volumes[vol_id].vol_flags & 1;

		//dbgprint(__FUNCTION__, "idk_bool: %d\n", idk_bool);

		if (idk_bool) // all mesh have same material properties
		{
			if ((g_p_PHEN_flags[flags_offset] & PHEN_BITSET_OF_INTEREST) != 0)
			{
				for (UINT i = 0; i < g_p_PHEN_volumes[vol_id].id1; i++)
				{
					PHEN_indexes idx = g_p_PHEN_index[index_offset + i];

					big_buffer[points_offset + idx.i0].Color = clr_red;
					big_buffer[points_offset + idx.i1].Color = clr_red;
					big_buffer[points_offset + idx.i2].Color = clr_red;
					if (idx.i3 != 0xFFFF) big_buffer[points_offset + idx.i3].Color = clr_red;
				}
			}
			else if ((g_p_PHEN_flags[flags_offset] & PHEN_BITSET_OF_INTEREST2) != 0)
			{
				for (UINT i = 0; i < g_p_PHEN_volumes[vol_id].id1; i++)
				{
					PHEN_indexes idx = g_p_PHEN_index[index_offset + i];

					big_buffer[points_offset + idx.i0].Color = clr_green;
					big_buffer[points_offset + idx.i1].Color = clr_green;
					big_buffer[points_offset + idx.i2].Color = clr_green;
					if (idx.i3 != 0xFFFF) big_buffer[points_offset + idx.i3].Color = clr_green;
				}
			}
		}
		else 
		{
			for (UINT i = 0; i < g_p_PHEN_volumes[vol_id].id1; i++)
			{
				if((g_p_PHEN_flags[flags_offset + i] & PHEN_BITSET_OF_INTEREST) != 0){

					PHEN_indexes idx = g_p_PHEN_index[index_offset + i];

					big_buffer[points_offset + idx.i0].Color = clr_red;
					big_buffer[points_offset + idx.i1].Color = clr_red;
					big_buffer[points_offset + idx.i2].Color = clr_red;
					if (idx.i3 != 0xFFFF) big_buffer[points_offset + idx.i3].Color = clr_red;
				}
				else if ((g_p_PHEN_flags[flags_offset] & PHEN_BITSET_OF_INTEREST2) != 0)
				{
					PHEN_indexes idx = g_p_PHEN_index[index_offset + i];

					big_buffer[points_offset + idx.i0].Color = clr_green;
					big_buffer[points_offset + idx.i1].Color = clr_green;
					big_buffer[points_offset + idx.i2].Color = clr_green;
					if (idx.i3 != 0xFFFF) big_buffer[points_offset + idx.i3].Color = clr_green;
				}
			}
		}
	}

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = g_PHEN_amt_vertex * sizeof(simple_vertex);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = big_buffer;
	HRESULT hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pPHEN_vertex_buffer);
	if (FAILED(hr))
	{
		dbgprint(__FUNCTION__, "CreateBuffer failed hr = %d! \n", hr);
		return 0;
	}
	free(big_buffer);
	return 1;
}

bool PHEN_make_index_buffers()
{

	bool error_at_index_create = false;

	g_pPHEN_draw_segments = (PHEN_draw_segment*)malloc(sizeof(PHEN_draw_segment) * g_PHEN_amt_volumes);
	if (!g_pPHEN_draw_segments) return false;

	for (UINT vol_id = 0; vol_id < g_PHEN_amt_volumes; vol_id++) {

		asura_bbox* bbp = &(g_p_PHEN_volumes[vol_id].bb);

		

		float obj_radius = max(max((bbp->x2 - bbp->x1), (bbp->y2 - bbp->y1)), (bbp->z2 - bbp->z1)) / 2;

		XMFLOAT3 pos = { (bbp->x1 + bbp->x2) / 2, (bbp->y1 + bbp->y2) / 2, (bbp->z1 + bbp->z2) / 2 };
		
		// designations
		g_pPHEN_draw_segments[vol_id].pos = pos;
		g_pPHEN_draw_segments[vol_id].obj_radius = obj_radius;
		g_pPHEN_draw_segments[vol_id].base_vertex_offset = g_p_PHEN_volumes[vol_id].point_offset;
	


		UINT amt_indexes_needed = 0;
		UINT indexes_offset = g_p_PHEN_volumes[vol_id].indexes_offset;
		for (UINT i = 0; i < g_p_PHEN_volumes[vol_id].id1; i++)
			amt_indexes_needed += (g_p_PHEN_index[indexes_offset + i].i3 == 0xFFFF) ? 3 : 6; // additional triangle

		// designations
		g_pPHEN_draw_segments[vol_id].amt_indexes = amt_indexes_needed;

		WORD* indexes_buf = (WORD*)malloc(sizeof(WORD) * amt_indexes_needed);

		UINT tri_id = 0;

		for (UINT i = 0; i < g_p_PHEN_volumes[vol_id].id1; i++)
		{
			PHEN_indexes idx = g_p_PHEN_index[indexes_offset + i];

			if (idx.i3 != 0xFFFF) {
				indexes_buf[tri_id * 3 + 0] = idx.i1;
				indexes_buf[tri_id * 3 + 1] = idx.i0;
				indexes_buf[tri_id * 3 + 2] = idx.i2;
				tri_id++;
				indexes_buf[tri_id * 3 + 0] = idx.i2;
				indexes_buf[tri_id * 3 + 1] = idx.i0;
				indexes_buf[tri_id * 3 + 2] = idx.i3;
				tri_id++;
			}
			else {
				indexes_buf[tri_id * 3 + 0] = idx.i1;
				indexes_buf[tri_id * 3 + 1] = idx.i0;
				indexes_buf[tri_id * 3 + 2] = idx.i2;
				tri_id++;
			}
		}

		// Indexes
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(WORD) * amt_indexes_needed;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		bd.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA InitData = {};
		InitData.pSysMem = indexes_buf;
		InitData.SysMemPitch = 0;

		HRESULT hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &(g_pPHEN_draw_segments[vol_id].idx_buffer));
		if (FAILED(hr))
		{
			dbgprint(__FUNCTION__, "Create index buffer failed !!! hr = %d\n", hr);
			free(indexes_buf);
			g_pPHEN_draw_segments[vol_id].idx_buffer = nullptr;
			error_at_index_create = true;
		}

		free(indexes_buf);

	}

	return !error_at_index_create;
}

int load_PHEN_chunk(wchar_t * FileName)
{
	wchar_t file_path[MAX_PATH] = { 0 };
	wsprintf(file_path, L"%ws\\PHEN_chunk\\", g_path1);
	lstrcat(file_path, FileName);

	HRESULT hr = S_OK;
	DWORD a = 0;
	//dbgprint("EMOD", "%ws\n", file_path);
	HANDLE f = CreateFileW(file_path, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (GetLastError() != ERROR_SUCCESS)
	{
		dbgprint("PHEN_Parse", "Opening file Error: %d\n", GetLastError());
		CloseHandle(f);
		return 0;
	}
	Asura40bHdr fh;
	READ(fh);
	if (fh.magic != (DWORD)'NEHP' || fh.ver1 != 6)
	{
		dbgprint("PHEN_Parse", "Opening file Error: BAD_HEADER (HEADER: %08x Type %02d BytesReaden: %d)\n", fh.magic, fh.ver1, a);
		CloseHandle(f);
		return 0;
	}

	dbgprint(__FUNCTION__, "amount volumes %d \n", fh.amt_volumes);
	dbgprint(__FUNCTION__, "amount vertexes %d \n", fh.points_arr_cnt);
	dbgprint(__FUNCTION__, "amount indexes %d \n", fh.amt_indexes);

	g_PHEN_amt_volumes = fh.amt_volumes;
	g_PHEN_amt_vertex = fh.points_arr_cnt;

	g_p_PHEN_volumes = (PHEN_struct_with_bboxes*)malloc(sizeof(PHEN_struct_with_bboxes) * fh.amt_volumes);

	READP(g_p_PHEN_volumes, sizeof(PHEN_struct_with_bboxes) * fh.amt_volumes);

	//Dump_PHEN_volumes(g_p_PHEN_volumes, fh.amt_volumes);

	g_p_PHEN_vertex = (XMFLOAT3*)malloc(sizeof(XMFLOAT3)*fh.points_arr_cnt);

	READP(g_p_PHEN_vertex, sizeof(XMFLOAT3)*fh.points_arr_cnt);

	g_p_PHEN_index = (PHEN_indexes*)malloc(sizeof(PHEN_indexes) * fh.amt_indexes);

	READP(g_p_PHEN_index, sizeof(PHEN_indexes) * fh.amt_indexes);

	SFPC((fh.amt_indexes + 3) & ~3); // some byte flags?

	g_p_PHEN_flags = (WORD*)malloc(sizeof(WORD)*fh.amt_flags);

	READP(g_p_PHEN_flags, sizeof(WORD)*fh.amt_flags);

	//for (UINT i = 0; i < fh.amt_volumes; i++)
	//{
	//	test_bound_volume_draw(i);
	//}
	//test_bound_volume_draw(0);
	
	if (!PHEN_make_vertex_buffer())
	{
		printf(__FUNCTION__, "PHEN_make_vertex_buffer() failed!\n");
		CloseHandle(f);
		return 0;
	}

	if (!PHEN_make_index_buffers())
	{
		printf(__FUNCTION__, "PHEN_make_index_buffers() failed!\n");
		CloseHandle(f);
		return 0;
	}

	CloseHandle(f);
	return 1;
}

int read_PHEN_chunk(wchar_t* PHEN_folder_path)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(PHEN_folder_path, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		dbgprint("PHEN", "FindFirstFile failed (%d) %ws\n", GetLastError(), PHEN_folder_path);
		return 0;
	}
	do {
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			continue;
		}
		else
		{
			return load_PHEN_chunk(FindFileData.cFileName);
		}
	} while (FindNextFile(hFind, &FindFileData) != 0);

	FindClose(hFind);
	return 1;
}

int PHEN_draw()
{
	if (!g_pPHEN_draw_segments || !g_pPHEN_vertex_buffer) return 0;

	g_pImmediateContext->VSSetShader(g_pVertexShader_s, nullptr, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader_s, nullptr, 0);
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout_s);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT stride = sizeof(simple_vertex);
	UINT offset = 0;

	g_pImmediateContext->RSSetState(g_Wireframe_RS);

	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pPHEN_vertex_buffer, &stride, &offset);

	for (UINT vol_id = 0; vol_id < g_PHEN_amt_volumes; vol_id++) {
		
		if (g_frustum.CheckCube(g_pPHEN_draw_segments[vol_id].pos, g_pPHEN_draw_segments[vol_id].obj_radius))
		{
			g_pImmediateContext->IASetIndexBuffer(g_pPHEN_draw_segments[vol_id].idx_buffer, DXGI_FORMAT_R16_UINT, 0);

			g_pImmediateContext->DrawIndexed(g_pPHEN_draw_segments[vol_id].amt_indexes, 0, g_pPHEN_draw_segments[vol_id].base_vertex_offset);
		}
	}

	g_pImmediateContext->RSSetState(g_Env_RS);

	return 0;
}


