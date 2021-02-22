#pragma once
#include "AVP2010MapViewer.h"

struct EMOD_entry {
	bool visible;
	char* name;
	XMFLOAT3 pos;
	WORD some_flags;
	bbox bb;
};

int read_EMOD_chunk(wchar_t* FileName);
int Init_EMOD_vertex();
void EMOD_destroy();