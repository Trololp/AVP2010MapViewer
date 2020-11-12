#include "Load_map.h"
#include "Hash_tree.h"
#include "Debug.h"

struct Env_header
{
	DWORD num_vertex_buf;
	DWORD size_vertex_buf; // Can be size_vertex_buf[num] maybe idk need check !!!
	DWORD num_index_buf;
	DWORD size_index_buf;
	DWORD Total_meshes;
};

struct Asura16header
{
	DWORD magic;
	DWORD size_file;
	DWORD type;
	DWORD type2;
};

struct MARE_ENTRY
{
	DWORD mat_hash;
	unsigned long int num;
	DWORD Hash1;
	DWORD Hash2;
	DWORD Hash3;
	DWORD Hash4;
	DWORD Hash5;
	float a;
	float b[4];
	DWORD c[3];
	DWORD Hash6;
	DWORD d;
	DWORD e[4];
	DWORD f[2];
	DWORD h[3][4];
	DWORD g;
	DWORD i[6];
	DWORD j[3];
	DWORD l[6];
	DWORD Default_mat_id;
	DWORD Hash7;
	DWORD m;
	DWORD Hash8;
	DWORD n;
	DWORD Hash9;
	DWORD Hash10;
	DWORD k[4];
	DWORD pad[16];
};


extern ID3D11Device*           g_pd3dDevice;
extern Map_mesh_info*			Mesh_info_structs;
extern ID3D11Buffer*           g_pVertexBuffer_env;
extern ID3D11Buffer*           g_pIndexBuffer_env;
extern hash_tree_node* hash_tree_texture;
extern wchar_t* g_path1;
extern wchar_t* g_path2;

int Read_map(const wchar_t* file_name, DWORD* g_total_meshes)
{
	HRESULT hr = S_OK;
	DWORD bytes_readen = 0;
	HANDLE file = CreateFileW(file_name, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD* asura_header = (DWORD*)malloc(28);
	ReadFile(file, asura_header, 28, &bytes_readen, NULL);
	if (GetLastError() != ERROR_SUCCESS)
	{
		dbgprint("RSCF_Parse_Enviroment", "Opening file Error: %d\n", GetLastError());
		return 0;
	}
	if (*asura_header != (DWORD)'FCSR' || *(asura_header + 5) != (DWORD)0xB)
	{
		dbgprint("RSCF_Parse_Enviroment", "Opening file Error: BAD_HEADER (HEADER: %08x Type %02d BytesReaden: %d)\n", *asura_header, *(asura_header + 5), bytes_readen);
		return 0;
	}
	free(asura_header);

	DWORD resource_name = NULL;

	ReadFile(file, &resource_name, 4, &bytes_readen, NULL);

	Env_header env_header;
	ReadFile(file, &env_header, sizeof(Env_header), &bytes_readen, NULL);
	*g_total_meshes = env_header.Total_meshes;
	DWORD t_m = env_header.Total_meshes;

	dbgprint("RSCF_Parse_Enviroment", "env_header: total_meshes: %d total_indexes: %d total_vertexes: %d\n", 
			env_header.Total_meshes, env_header.size_index_buf, env_header.size_vertex_buf);

	Mesh_info_structs = (Map_mesh_info*)_aligned_malloc(sizeof(Map_mesh_info) * t_m, 16);

	ReadFile(file, Mesh_info_structs, sizeof(Map_mesh_info) * t_m, &bytes_readen, NULL); //Mesh infos

	int points = env_header.size_vertex_buf;

	avp_vtx* vertices = (avp_vtx*)_aligned_malloc(sizeof(avp_vtx)*points, 16);
	ReadFile(file, vertices, sizeof(avp_vtx)*points, &bytes_readen, NULL);

	WORD* indexes = (WORD*)_aligned_malloc(2*env_header.size_index_buf, 16);
	ReadFile(file, indexes, 2 * env_header.size_index_buf, &bytes_readen, NULL);

	// Create vertex buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(avp_vtx) * points;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertices;
	InitData.SysMemPitch = 0;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer_env);
	if (FAILED(hr))
	{
		dbgprint("RSCF_Parse_Enviroment", "Create vertex buffer failed !!!\n");
		_aligned_free(vertices);
		return 0;
	}
	_aligned_free(vertices);

	// Indexes
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = 2*env_header.size_index_buf;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;

	InitData.pSysMem = indexes;
	InitData.SysMemPitch = 0;

	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pIndexBuffer_env);
	if (FAILED(hr))
	{
		dbgprint("RSCF_Parse_Enviroment", "Create index buffer failed !!!\n");
		return 0;
		_aligned_free(indexes);
	}

	_aligned_free(indexes);
	CloseHandle(file);
	return 1;
}

int Read_MARE(const wchar_t* file_name, int type)
{
	wchar_t file_path[MAX_PATH] = { 0 };
	if (type == 0)
	{
		wsprintf(file_path, L"%ws\\MARE_chunk\\", g_path1);
		lstrcat(file_path, file_name);
	}
	else
	{
		wsprintf(file_path, L"%ws\\MARE_chunk\\", g_path2);
		lstrcat(file_path, file_name);
	}

	DWORD bytes_readen = 0;
	HANDLE file = CreateFileW(file_path, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Asura16header file_header;
	ReadFile(file, &file_header, 16, &bytes_readen, NULL);
	if (int i = GetLastError() != ERROR_SUCCESS)
	{
		dbgprint("Materials", "Opening file Error: %d\n", i);
		return 0;
	}
	if (file_header.magic != (DWORD)'ERAM' || (file_header.type != 21))
	{
		dbgprint("Materials", "Opening file Error: INCORRECT FILE HEADER (%ws)\n", file_path);
		return 0;
	}

	unsigned int mat_entries;
	ReadFile(file, &mat_entries, 4, &bytes_readen, NULL);
	int i = 0;
	MARE_ENTRY* entries_mem = (MARE_ENTRY*)malloc(312 * mat_entries);
	ReadFile(file, entries_mem, 312 * mat_entries, &bytes_readen, NULL);

	dbgprint("Materials", "MARE Header: %d materials in file\n", mat_entries);

	hash_tree_node* h_n;
	
	while (i <= mat_entries)
	{
		if (search_mat_by_hash(entries_mem[i].mat_hash))
		{
			i++;
			continue;
		}
		Material* curr_mat = new Material;
		curr_mat->mat_hash = entries_mem[i].mat_hash;
		curr_mat->mat_num = entries_mem[i].num;

		h_n = search_by_hash(hash_tree_texture, entries_mem[i].Hash1);
		//dbgprint("Materials", "%x = search_by_hash( %x , %x ) \n", h_n, hash_tree_texture, entries_mem[i].Hash1);
		if (h_n)
		{
			//dbgprint("Materials", "Found match with texture id %d and material hash %x \n", h_n->texture->id, curr_mat->mat_hash);
			curr_mat->texture_diff_id = h_n->texture->id;
		}
		else
		{
			dbgprint("Materials", "Not found match with texture hash %x and material hash %x \n", entries_mem[i].Hash1, curr_mat->mat_hash);
			curr_mat->texture_diff_id = 0;
		}

		add_hash_and_mat(curr_mat, curr_mat->mat_hash);
		i++;
	}

	CloseHandle(file);
	return 1;
}


int Read_materials(const wchar_t* MARE_folder_path, int type)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(MARE_folder_path, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		dbgprint("Materials", "FindFirstFile failed (%d) %ws\n", GetLastError(), MARE_folder_path);
		return 0;
	}
	do {
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			continue;
		}
		else
		{
			Read_MARE(FindFileData.cFileName, type);
		}
	} while (FindNextFile(hFind, &FindFileData) != 0);

	FindClose(hFind);
	return 1;
}

