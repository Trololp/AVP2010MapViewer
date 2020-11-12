#pragma once
#include "AVP2010MapViewer.h"
#pragma pack(push, 1)
struct model_header
{
	DWORD model_mesh_info_count;
	DWORD vertex_count;
	DWORD index_count;
	DWORD unk;
	DWORD count_unk_chunks;
	bool  unk_bool;
};
#pragma pack(pop)

struct model_mat_info
{
	DWORD mat_hash;
	DWORD unk;
	DWORD points_count;
	DWORD some_offset;
	DWORD unk2;
	DWORD unk3;
};

struct model_info
{
	ID3D11Buffer *pVertexBuff;
	ID3D11Buffer *pIndexBuff;
	model_mat_info* mmi;
	UINT		mmi_count;
	UINT		unk; //unk bool value (maybe displays that current model can be ragdol)
	char*		mdl_name;
	int			empty_model;
};

int Read_models(const wchar_t* models_folder_path);
void Read_model(const wchar_t* model_name, int skip);
void Delete_models();