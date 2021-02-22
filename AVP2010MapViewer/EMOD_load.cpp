#include "EMOD_load.h"
#include "AVP2010MapViewer.h"
#include "Console.h"
#include "Debug.h"

struct file_header {
	DWORD Magic; // 'EMOD'
	DWORD Size;
	DWORD ver1;
	DWORD ver2;
	DWORD count;
};

DWORD Command_dump_emod(DWORD*);

Command cmd_dump_emod = {
	"dump_emod",
	CONCMD_RET_ZERO,
	CONARG_END,
	Command_dump_emod,
	nullptr
};

EMOD_entry* g_emod_entrys;
unsigned int g_emod_count;
bool g_emod_loaded = false;

extern ID3D11Buffer*			g_pVertexBuffer_emod_bbox; // EMOD
extern ID3D11Device*           g_pd3dDevice;
extern std::vector <bbox>		g_bboxes;

extern int read_padded_str(HANDLE file, char* dest);
extern wchar_t* g_path1;

DWORD Command_dump_emod(DWORD* args)
{
	if (!g_emod_loaded)
	{
		dbgprint("EMOD_dump", "No emod loaded \n");
		return 0;
	}
		
	for (int i = 0; i < g_emod_count; i++)
	{
		EMOD_entry* emod_e = g_emod_entrys + i;
		dbgprint("EMOD_dump", "name: %s pos: %05f %05f %05f flags: %d \n", emod_e->name, emod_e->pos.x, emod_e->pos.y, emod_e->pos.z, emod_e->some_flags);
	}
	return 0;
}

int Init_EMOD_vertex()
{
	HRESULT hr = S_OK;

	if (!g_emod_loaded)
	{
		dbgprint("EMOD", "Init EMOD bboxes failed: no EMOD loaded ! \n");
		return 0;
	}

	int size_of_vertex_data = 12 * 2 * sizeof(simple_vertex) * g_emod_count;
	simple_vertex* vertices = (simple_vertex*)malloc(size_of_vertex_data);
	int r = 0;

	dbgprint("EMOD", "Bbox: count: %d\n", g_emod_count);

	for (int i = 0; i < g_emod_count; i++)
	{
		EMOD_entry* emod_e = g_emod_entrys + i;
		XMFLOAT4 clr_w = (XMFLOAT4)Colors::White;
		XMFLOAT4 clr_lg = (XMFLOAT4)Colors::LightGray;
		XMFLOAT4 clr_dg = (XMFLOAT4)Colors::Gray;
		XMFLOAT3 p1 = emod_e->bb.p1;
		XMFLOAT3 p2 = emod_e->bb.p2;
		XMFLOAT3 a = p1;
		XMFLOAT3 b = XMFLOAT3(p2.x, p1.y, p1.z);
		XMFLOAT3 c = XMFLOAT3(p2.x, p2.y, p1.z);
		XMFLOAT3 d = XMFLOAT3(p1.x, p2.y, p1.z);
		XMFLOAT3 e = XMFLOAT3(p1.x, p1.y, p2.z);
		XMFLOAT3 f = XMFLOAT3(p2.x, p1.y, p2.z);
		XMFLOAT3 g = p2;
		XMFLOAT3 h = XMFLOAT3(p1.x, p2.y, p2.z);

		vertices[0 + r] = { a, clr_dg };  vertices[12 + r] = { g, clr_w }; //
		vertices[1 + r] = { b, clr_dg };  vertices[13 + r] = { h, clr_w }; //
		vertices[2 + r] = { b, clr_dg };  vertices[14 + r] = { h, clr_w }; //     h-----g
		vertices[3 + r] = { c, clr_dg };  vertices[15 + r] = { e, clr_w }; //    /|    /|
		vertices[4 + r] = { c, clr_dg };  vertices[16 + r] = { e, clr_lg };//   e-----f |
		vertices[5 + r] = { d, clr_dg };  vertices[17 + r] = { a, clr_lg };//   | d---|-c
		vertices[6 + r] = { d, clr_dg };  vertices[18 + r] = { f, clr_lg };//   |/    |/
		vertices[7 + r] = { a, clr_dg };  vertices[19 + r] = { b, clr_lg };//   a-----b
		vertices[8 + r] = { e, clr_w };   vertices[20 + r] = { g, clr_lg };//
		vertices[9 + r] = { f, clr_w };   vertices[21 + r] = { c, clr_lg };//
		vertices[10 + r] = { f, clr_w };  vertices[22 + r] = { h, clr_lg };//
		vertices[11 + r] = { g, clr_w };  vertices[23 + r] = { d, clr_lg };//

		r += 24;
	}


	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = size_of_vertex_data;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertices;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer_emod_bbox);
	if (FAILED(hr))
	{
		dbgprint("EMOD", "Bbox CreateBuffer failed ! \n");
		g_emod_loaded = false; // user should not use it now
		return 0;
	}
	free(vertices);
	return 1;
}

int load_EMOD_chunk(wchar_t * FileName)
{
	wchar_t file_path[MAX_PATH] = { 0 };
	wsprintf(file_path, L"%ws\\EMOD_chunk\\", g_path1);
	lstrcat(file_path, FileName);

	HRESULT hr = S_OK;
	DWORD a = 0;
	//dbgprint("EMOD", "%ws\n", file_path);
	HANDLE f = CreateFileW(file_path, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (GetLastError() != ERROR_SUCCESS)
	{
		dbgprint("EMOD_Parse", "Opening file Error: %d\n", GetLastError());
		CloseHandle(f);
		return 0;
	}
	file_header fh;
	READ(fh);
	if (fh.Magic != (DWORD)'DOME' || fh.ver1 != 6)
	{
		dbgprint("EMOD_Parse", "Opening file Error: BAD_HEADER (HEADER: %08x Type %02d BytesReaden: %d)\n", fh.Magic, fh.ver1, a);
		CloseHandle(f);
		return 0;
	}

	char EMOD_package_name[256] = { 0 };
	read_padded_str(f, EMOD_package_name);
	dbgprint("EMOD", "Package name: %s\n", EMOD_package_name);

	g_emod_count = fh.count;
	g_emod_entrys = (EMOD_entry*)malloc(sizeof(EMOD_entry)*fh.count);

	for (int i = 0; i < fh.count; i++)
	{
		char* emod_name = (char*)malloc(256);
		read_padded_str(f, emod_name);
		EMOD_entry* emod_entry = g_emod_entrys + i;
		emod_entry->name = emod_name;
		READP(&(emod_entry->pos), 12);
		SFPC(0x28);
		READP(&(emod_entry->some_flags), 2);
		char temp_str[16] = { 0 };
		sprintf(temp_str, " 0x%x\n", emod_entry->some_flags);
		strcat(emod_name, temp_str);
		//dbgprint("EMOD", "name: %s pos: %05f %05f %05f flags: %d \n", emod_name, emod_entry->pos.x, emod_entry->pos.y, emod_entry->pos.z, emod_entry->some_flags);
		SFPC(6);
		asura_bbox asura_bb;
		READ(asura_bb); 
		emod_entry->bb.p1 = { asura_bb.x1 + emod_entry->pos.x, asura_bb.y1 \
			+ emod_entry->pos.y, asura_bb.z1 + emod_entry->pos.z };
		emod_entry->bb.p2 = { asura_bb.x2 + emod_entry->pos.x, asura_bb.y2 \
			+ emod_entry->pos.y, asura_bb.z2 + emod_entry->pos.z };
		emod_entry->bb.Color = {1.0f, 1.0f, 1.0f, 1.0f};
		emod_entry->bb.rot = NULL_ROT;
		//g_bboxes.push_back({ emod_entry->bb.p1,emod_entry->bb.p2, XMFLOAT4(1.0f,1.0f,1.0f,1.0f) });
		SFPC(12);

	}

	CloseHandle(f);
	g_emod_loaded = true;
	return 1;
}

int read_EMOD_chunk(wchar_t* EMOD_folder_path)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(EMOD_folder_path, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		dbgprint("EMOD", "FindFirstFile failed (%d) %ws\n", GetLastError(), EMOD_folder_path);
		return 0;
	}
	do {
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			continue;
		}
		else
		{
			return load_EMOD_chunk(FindFileData.cFileName);
		}
	} while (FindNextFile(hFind, &FindFileData) != 0);

	FindClose(hFind);
	return 1;
}

void EMOD_destroy()
{
	for (unsigned int i = 0; i < g_emod_count; i++)
	{
		EMOD_entry* emod_entry = g_emod_entrys + i;
		free(emod_entry->name);
	}
	free(g_emod_entrys);
}
