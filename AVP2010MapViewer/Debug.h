#pragma once
#include "AVP2010MapViewer.h"
using namespace DirectX;

void dbgprint(const char* debug_note, const char* fmt, ...);
DWORD Console_command_tp(DWORD* args);
int bbox_to_vertex(std::vector <bbox> &bboxes, DWORD* g_points);
int line_to_vertex(std::vector <line> &lines, DWORD* g_points);
void Dump_matrix(const char* name, XMMATRIX matrix);
void Dump_vector(const char* name, XMVECTOR vector);
void Dump_hex(const char* name, void* data, unsigned int count);