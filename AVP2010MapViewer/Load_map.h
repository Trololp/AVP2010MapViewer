#pragma once
#include "AVP2010MapViewer.h"

int Read_map(const wchar_t* file_name, DWORD* g_total_meshes);
int Read_materials(const wchar_t* file_name, int type);