#pragma once
#include "AVP2010MapViewer.h"

struct EMOD_data
{
	XMFLOAT3 pos;
	bbox bb;
	char* name;
	DWORD unk0;
	DWORD unk1;
	DWORD unk2;
	DWORD unk3;
	DWORD unk4;
	DWORD unk5;
	DWORD unk6;
	DWORD unk7;
	DWORD unk8;
	DWORD unk9;
	DWORD unk10;
	DWORD unk11;
};

int Read_map(const wchar_t* file_name, DWORD* g_total_meshes);
int Read_materials(const wchar_t* file_name, int type);