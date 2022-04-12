#include "Load_map.h"
#include "Hash_tree.h"
#include "Debug.h"
#include "Console.h"
#include "EMOD_load.h"
#include <ShObjIdl.h>


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

struct Asura28Header
{
	DWORD magic;
	DWORD size_file;
	DWORD type;
	DWORD type2;
	DWORD rscf_type1;
	DWORD rscf_type2;
	DWORD size_of_data;
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

struct MMI_EMOD_part {
	std::vector<unsigned int> MMI_ids;
};

//Globals
unsigned int g_map_max_mesh_id;
unsigned int g_map_min_mesh_id;
std::vector<unsigned int> g_map_mesh_ids;
MMI_EMOD_part* g_mmi_emod_parts;



//Externs
extern ID3D11Device*           g_pd3dDevice;
extern Map_mesh_info*			g_mesh_info_structs;
extern ID3D11Buffer*           g_pVertexBuffer_env;
extern ID3D11Buffer*           g_pIndexBuffer_env;
extern hash_tree_node* hash_tree_texture;
extern wchar_t* g_path1;
extern wchar_t* g_path2;

//Testing models inside map
extern unsigned int			g_test_map_model_id;
extern bool					g_test_map_model_enabled;

//EMOD
extern EMOD_entry* g_emod_entrys;
extern unsigned int g_emod_count;
extern bool g_emod_loaded;

DWORD Command_test_map_mesh(DWORD*);
DWORD Command_dump_map(DWORD*);
DWORD Command_print_list_map_meshs(DWORD*);

Command cmd_test_map_mesh = {
	"test_map_mesh",
	CONCMD_RET_STR,
	CONARG_INT,
	Command_test_map_mesh,
	nullptr
};

Command cmd_dump_map = {
	"dump_map",
	CONCMD_RET_ZERO,
	CONARG_INT,
	Command_dump_map,
	nullptr
};




Command cmd_print_list_map_meshs = {
	"print_list_map_meshs",
	CONCMD_RET_ZERO,
	CONARG_END,
	Command_print_list_map_meshs,
	nullptr
};

int read_padded_str2(HANDLE file, char* dest)
{
	bool check = 1;
	DWORD bytes_readen = 0;
	int len = 4;
	int data;
	DWORD offset = SetFilePointer(file, NULL, NULL, FILE_CURRENT);
	ReadFile(file, &data, 4, &bytes_readen, NULL);
	for (; ((data >> 24) && ((data >> 16) & 0xFF) && ((data >> 8) & 0xFF) && (data & 0xFF));)
	{
		ReadFile(file, &data, 4, &bytes_readen, NULL);
		len += 4;
	}
	SetFilePointer(file, offset, NULL, FILE_BEGIN);
	if (!ReadFile(file, dest, len, &bytes_readen, NULL))
	{
		dbgprint("Error", "Read padded str error %d\n", GetLastError());
		return 0;
	}
	return 1;
}

DWORD Command_test_map_mesh(DWORD* args)
{
	DWORD id = args[0];

	if (g_map_min_mesh_id <= id && g_map_max_mesh_id >= id)
	{
		bool found = false;

		for (int i = 0; i < g_map_mesh_ids.size(); i++)
		{
			if (g_map_mesh_ids[i] == id)
			{
				found = true;
				break;
			}
		}

		if (found)
		{
			g_test_map_model_enabled = true;
			g_test_map_model_id = id;
			return (DWORD)(char*)"";
		}
	}

	g_test_map_model_enabled = false;
	g_test_map_model_id = 0;

	return (DWORD)(char*)"Not found";
}

DWORD Command_print_list_map_meshs(DWORD* args)
{
	for (int i = 0; i < g_map_mesh_ids.size(); i++)
	{
		dbgprint("print_list_map_meshs", "%d \n", g_map_mesh_ids[i]);
	}

	return 0;
}

int Read_map(const wchar_t* file_name, DWORD* g_total_meshes)
{
	HRESULT hr = S_OK;
	DWORD a = 0;
	HANDLE f = CreateFileW(file_name, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (GetLastError() != ERROR_SUCCESS)
	{
		dbgprint("RSCF_Parse_Enviroment", "Opening file Error: %d\n", GetLastError());
		CloseHandle(f);
		return 0;
	}
	
	Asura28Header asura_header;
	READ(asura_header);
	
	if (asura_header.magic != (DWORD)'FCSR' || asura_header.rscf_type2 != (DWORD)0xB) // 0xB == enviroment
	{
		dbgprint("RSCF_Parse_Enviroment", "Opening file Error: BAD_HEADER (HEADER: %08x Type %02d BytesReaden: %d)\n", asura_header.magic, asura_header.rscf_type2, a);
		CloseHandle(f);
		return 0;
	}

	char* name_str = (char *)malloc(260);


	read_padded_str2(f, name_str);

	if (strcmp(name_str, "Env"))
		dbgprint("RSCF_Parse_Enviroment", "Enviroment resource name different from \"Env\" : %s \n", name_str);

	Env_header env_header;
	READ(env_header);
	*g_total_meshes = env_header.Total_meshes;
	DWORD t_m = env_header.Total_meshes;

	dbgprint("RSCF_Parse_Enviroment", "env_header: total_meshes: %d total_indexes: %d total_vertexes: %d\n", 
			env_header.Total_meshes, env_header.size_index_buf, env_header.size_vertex_buf);

	g_mesh_info_structs = (Map_mesh_info*)_aligned_malloc(sizeof(Map_mesh_info) * t_m, 16);
	READP(g_mesh_info_structs, sizeof(Map_mesh_info) * t_m);

	//Parsing MMI structs to retrieve EMOD_id information, can be usefull in future
	//g_mmi_emod_parts = (MMI_EMOD_part*)malloc(sizeof(MMI_EMOD_part) * g_emod_count);
	g_mmi_emod_parts = new MMI_EMOD_part[g_emod_count];


	dbgprint("RSCF_Parse_Enviroment", "g_emod_count = %d t_m = %d\n", g_emod_count, t_m);

	for (int i = 0; i < t_m; i++)
	{
		unsigned int emod_id = g_mesh_info_structs[i].emod_id;
		g_mmi_emod_parts[emod_id].MMI_ids.push_back(i);

		
		g_map_max_mesh_id = (emod_id > g_map_max_mesh_id) ? emod_id : g_map_max_mesh_id;
		g_map_min_mesh_id = (emod_id < g_map_min_mesh_id) ? emod_id : g_map_min_mesh_id;

		bool this_id_unique = true;

		// this is made for check if its not linear ids and there is gaps somewhere
		for (int j = 0; j < g_map_mesh_ids.size(); j++)
		{
			if (g_map_mesh_ids[j] == emod_id)
			{
				this_id_unique = false;
				break;
			}
		}

		if (this_id_unique)
			g_map_mesh_ids.push_back(emod_id);
		
	}


	int points = env_header.size_vertex_buf;
	avp_vtx* vertices = (avp_vtx*)_aligned_malloc(sizeof(avp_vtx)*points, 16);
	READP(vertices, sizeof(avp_vtx)*points);


	WORD* indexes = (WORD*)_aligned_malloc(2*env_header.size_index_buf, 16);
	READP(indexes, 2 * env_header.size_index_buf);
	

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
		CloseHandle(f);
		_aligned_free(indexes);
		return 0;
	}

	_aligned_free(indexes);





	CloseHandle(f);
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
		CloseHandle(file);
		return 0;
	}
	if (file_header.magic != (DWORD)'ERAM' || (file_header.type != 21))
	{
		dbgprint("Materials", "Opening file Error: INCORRECT FILE HEADER (%ws)\n", file_path);
		CloseHandle(file);
		return 0;
	}

	unsigned int mat_entries;
	ReadFile(file, &mat_entries, 4, &bytes_readen, NULL);
	int i = 0;
	MARE_ENTRY* entries_mem = (MARE_ENTRY*)malloc(sizeof(MARE_ENTRY) * mat_entries);
	ReadFile(file, entries_mem, sizeof(MARE_ENTRY) * mat_entries, &bytes_readen, NULL);

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

	free(entries_mem);
	CloseHandle(file);
	return 1;
}

bool parse_MARE_for_dump_map(wchar_t* MARE_file_name, wchar_t* folder_where_to_dump, const wchar_t* mtl_file_name)
{
	wchar_t file_path[MAX_PATH] = { 0 };

	wsprintf(file_path, L"%ws\\MARE_chunk\\", g_path1);
	lstrcat(file_path, MARE_file_name);


	DWORD a = 0;
	HANDLE f = CreateFileW(file_path, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	Asura16header file_header;
	READ(file_header);

	if (GetLastError() != ERROR_SUCCESS)
	{
		dbgprint("Dump_map", "Opening file Error: %d (%ws)\n", GetLastError(), file_path);
		CloseHandle(f);
		return false;
	}
	if (file_header.magic != (DWORD)'ERAM' || (file_header.type != 21))
	{
		dbgprint("Dump_map", "Opening file Error: INCORRECT FILE HEADER (%ws)\n", file_path);
		CloseHandle(f);
		return false;
	}

	unsigned int mat_entries;

	READ(mat_entries);
	
	int i = 0;
	MARE_ENTRY* entries_mem = (MARE_ENTRY*)malloc(sizeof(MARE_ENTRY) * mat_entries);
	READP(entries_mem, sizeof(MARE_ENTRY) * mat_entries);

	dbgprint("Dump_map", "MARE Header: %d materials in file\n", mat_entries);

	HANDLE f2 = CreateFileW(mtl_file_name, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if ((GetLastError() != ERROR_SUCCESS) && (GetLastError() != ERROR_ALREADY_EXISTS))
	{
		dbgprint("Dump_map", "File create error: %d (%ws)\n", GetLastError(), mtl_file_name);
		CloseHandle(f2);
		CloseHandle(f);
		return false;
	}

	//Completly copy all folder with textures ?
	// not now
	wchar_t material_folder_name[50] = { 0 };
	char material_name_s[200] = { 0 };
	wchar_t material_folder[MAX_PATH];
	wchar_t new_texture_file_name[MAX_PATH];

	hash_tree_node* h_n;

	while (i <= mat_entries)
	{
		sprintf(material_name_s, "newmtl material_%08x \n", entries_mem[i].mat_hash);
		WRITEP(material_name_s, strlen(material_name_s));
		WRITEP("Ka 1.000000 1.000000 1.000000 \nKd 1.000000 1.000000 1.000000 \nKs 0.000000 0.000000 0.000000 \nTr 1.000000 \nillum 1 \nNs 0.000000 \n", 128);


		wsprintf(material_folder_name, L"material_%08x\\", entries_mem[i].mat_hash);
		lstrcpy(material_folder, folder_where_to_dump);
		lstrcat(material_folder, material_folder_name);
		CreateDirectory(material_folder, NULL);

		if (GetLastError() && (GetLastError() != ERROR_ALREADY_EXISTS))
		{
			dbgprint("Dump_map", "CreateDirectory Error: %d (%ws)\n", GetLastError(), material_folder);
			WRITEP("map_Kd default.dds \n \n", 22);
			i++;
			continue;
		}

		h_n = search_by_hash(hash_tree_texture, entries_mem[i].Hash1);

		if (h_n)
		{
			//Set texture
			if (h_n->texture->path == nullptr)
			{
				WRITEP("map_Kd default.dds \n \n", 22);
				i++;
				continue;
			}

			wchar_t* file_name = wcsrchr(h_n->texture->path, L'\\');
			lstrcpy(new_texture_file_name, material_folder);
			lstrcat(new_texture_file_name, file_name + 1);
			CopyFile(h_n->texture->path, new_texture_file_name, NULL);

			if (GetLastError() && (GetLastError() != ERROR_ALREADY_EXISTS))
			{
				dbgprint("Dump_map", "CopyFile Error: %d (%ws to %ws) \n", GetLastError(), h_n->texture->path, material_folder);
				WRITEP("map_Kd default.dds \n \n", 22);
				i++;
				continue;
			}

			char asd2[260];
			sprintf(asd2, "map_Kd Materials\\%ws%ws \n", material_folder_name, file_name + 1);
			WRITEP(asd2, strlen(asd2));
		}
		else
		{
			// This some specific material or textures not loaded for it yet
			dbgprint("Dump_map", "Not found match with texture hash %x and material hash %x \n", entries_mem[i].Hash1, entries_mem[i].mat_hash);
			// set default texture
			WRITEP("map_Kd default.dds \n \n", 22);
		}

		// normal texture
		h_n = search_by_hash(hash_tree_texture, entries_mem[i].Hash2);

		if (h_n)
		{
			//Set texture
			if (h_n->texture->path == nullptr)
			{
				i++;
				continue;
			}

			wchar_t* file_name = wcsrchr(h_n->texture->path, L'\\');
			lstrcpy(new_texture_file_name, material_folder);
			lstrcat(new_texture_file_name, file_name + 1);
			CopyFile(h_n->texture->path, new_texture_file_name, NULL);

			if (GetLastError() && (GetLastError() != ERROR_ALREADY_EXISTS))
			{
				dbgprint("Dump_map", "CopyFile Error: %d (%ws to %ws) \n", GetLastError(), h_n->texture->path, material_folder);
				i++;
				continue;
			}

			char asd2[260];
			sprintf(asd2, "map_Bump Materials\\%ws%ws \n \n", material_folder_name, file_name + 1);
			WRITEP(asd2, strlen(asd2));
		}
		else
		{
			// This some specific material or textures not loaded for it yet
			//dbgprint("Dump_map", "Not found match with texture hash %x and material hash %x \n", entries_mem[i].Hash1, entries_mem[i].mat_hash);
		}

		// alpha texture 
		h_n = search_by_hash(hash_tree_texture, entries_mem[i].Hash4);

		if (h_n)
		{
			//Set texture
			if (h_n->texture->path == nullptr)
			{
				i++;
				continue;
			}

			wchar_t* file_name = wcsrchr(h_n->texture->path, L'\\');
			lstrcpy(new_texture_file_name, material_folder);
			lstrcat(new_texture_file_name, file_name + 1);
			CopyFile(h_n->texture->path, new_texture_file_name, NULL);

			if (GetLastError() && (GetLastError() != ERROR_ALREADY_EXISTS))
			{
				dbgprint("Dump_map", "CopyFile Error: %d (%ws to %ws) \n", GetLastError(), h_n->texture->path, material_folder);
				i++;
				continue;
			}

			char asd2[260];
			sprintf(asd2, "map_d Materials\\%ws%ws \n \n", material_folder_name, file_name + 1);
			WRITEP(asd2, strlen(asd2));
		}
		else
		{
			// This some specific material or textures not loaded for it yet
			//dbgprint("Dump_map", "Not found match with texture hash %x and material hash %x \n", entries_mem[i].Hash1, entries_mem[i].mat_hash);
		}

		i++;
	}


	free(entries_mem);
	CloseHandle(f2);
	CloseHandle(f);
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



bool Dump_Entire_Map_to_Models(unsigned int type)
{
	wchar_t map_file_name[MAX_PATH];
	lstrcpy(map_file_name, g_path1);
	lstrcat(map_file_name, L"\\Env.RSCF");

	wchar_t emod_file_path[MAX_PATH];
	lstrcpy(emod_file_path, g_path1);
	lstrcat(emod_file_path, L"\\EMOD_chunk\\");
	//wchar_t path_where_to_save[MAX_PATH] = { 0 };
	wchar_t* path_where_to_save = nullptr;

	if (!g_emod_loaded)
		read_EMOD_chunk(emod_file_path);

	if (!g_emod_loaded)
	{
		dbgprint("Dump_map", "Cannot load EMOD chunk");
		return false; // at now we exit but there is way to dump map without having clue on object names.
	}

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
		COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileOpenDialog *pFileOpen;

		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

		if (SUCCEEDED(hr))
		{

			FILEOPENDIALOGOPTIONS* pfos = new FILEOPENDIALOGOPTIONS;
			ZeroMemory(pfos, sizeof(pfos));
			pFileOpen->GetOptions(pfos);
			*pfos = *pfos |= FOS_PICKFOLDERS;
			pFileOpen->SetOptions(*pfos);
			pFileOpen->SetTitle(L"Select folder to map dump");
			// Show the Open dialog box.
			hr = pFileOpen->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItem *pItem;
				hr = pFileOpen->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &path_where_to_save);
					if (FAILED(hr))
					{
						dbgprint("Dump_map", "failed to choose folder with error hr = %x \n", hr);
						//MessageBoxW(NULL, L"Bad folder or else ...", L"File Path", MB_OK);
					}
					pItem->Release();
				}
			}

			pFileOpen->Release();
		}
		CoUninitialize();
	}

	wchar_t materials_folder[MAX_PATH];
	lstrcpy(materials_folder, path_where_to_save);
	lstrcat(materials_folder, L"\\Materials\\");
	CreateDirectory(materials_folder, NULL);

	wchar_t mtl_filename[MAX_PATH];
	lstrcpy(mtl_filename, materials_folder);
	lstrcat(mtl_filename, L"\\Materials.mtl");

	//Copy default texture from cwd.
	wchar_t* cwd = _wgetcwd(0, 0);
	wchar_t default_tex_file_path[MAX_PATH];
	wchar_t new_tex_file_path[MAX_PATH];

	lstrcpy(new_tex_file_path, materials_folder);
	lstrcpy(default_tex_file_path, cwd);
	lstrcat(default_tex_file_path, L"\\default.dds");
	lstrcat(new_tex_file_path, L"\\default.dds");
	CopyFile(default_tex_file_path, new_tex_file_path, NULL);

	if (GetLastError() && (GetLastError() != ERROR_ALREADY_EXISTS))
	{
		dbgprint("Dump_map", "CopyFile Error: %d (%ws to %ws) \n", GetLastError(), default_tex_file_path, new_tex_file_path);
	}


	//First dump materials
	wchar_t MARE_folder_path[MAX_PATH];
	lstrcpy(MARE_folder_path, g_path1);
	lstrcat(MARE_folder_path, L"\\MARE_chunk\\*.MARE");

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(MARE_folder_path, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		dbgprint("Dump_map", "FindFirstFile failed (%d) %ws\n", GetLastError(), MARE_folder_path);
		return 0;
	}
	do {
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			continue;
		}
		else
		{
			parse_MARE_for_dump_map(FindFileData.cFileName, materials_folder, mtl_filename);
			break;
		}
	} while (FindNextFile(hFind, &FindFileData) != 0);

	FindClose(hFind);

	//Now open map again and parse it

	DWORD a = 0;
	HANDLE f = CreateFileW(map_file_name, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (GetLastError() != ERROR_SUCCESS)
	{
		dbgprint("Dump_map", "Opening map Error: %d\n", GetLastError());
		CloseHandle(f);
		return 0;
	}

	Asura28Header asura_header;
	READ(asura_header);

	if (asura_header.magic != (DWORD)'FCSR' || asura_header.rscf_type2 != (DWORD)0xB) // 0xB == enviroment
	{
		dbgprint("Dump_map", "Opening map Error: BAD_HEADER (HEADER: %08x Type %02d BytesReaden: %d)\n", asura_header.magic, asura_header.rscf_type2, a);
		CloseHandle(f);
		return 0;
	}

	char* name_str = (char *)malloc(260);


	read_padded_str2(f, name_str);

	if (strcmp(name_str, "Env"))
		dbgprint("Dump_map", "Enviroment resource name different from \"Env\" : %s \n", name_str);

	Env_header env_header;
	READ(env_header);
	DWORD t_m = env_header.Total_meshes;

	dbgprint("Dump_map", "env_header: total_meshes: %d total_indexes: %d total_vertexes: %d\n",
		env_header.Total_meshes, env_header.size_index_buf, env_header.size_vertex_buf);

	Map_mesh_info* mmi_arr = (Map_mesh_info*)_aligned_malloc(sizeof(Map_mesh_info) * t_m, 16);
	READP(mmi_arr, sizeof(Map_mesh_info) * t_m);

	int points = env_header.size_vertex_buf;
	avp_vtx* vertices = (avp_vtx*)_aligned_malloc(sizeof(avp_vtx)*points, 16);
	READP(vertices, sizeof(avp_vtx)*points);


	WORD* indexes = (WORD*)_aligned_malloc(2 * env_header.size_index_buf, 16);
	READP(indexes, 2 * env_header.size_index_buf);

	unsigned int unk_file_name_counter = 0;

	//For testing
	//g_emod_count = 100;

	for (unsigned int emod_id = 0; emod_id < g_emod_count; emod_id++)
	{
		if (g_emod_entrys[emod_id].some_flags == 0x2A) // Dont dump collision, not now
			continue;

		if (g_emod_entrys[emod_id].some_flags == 0xC)  // Guess it is water, dont dumping it, it anyway a plane without texture
			continue;

		if (g_mmi_emod_parts[emod_id].MMI_ids.size() == 0) // Seems something wrong
			continue;

		//Create .obj File with name of emod chunk
		wchar_t obj_model_name[MAX_PATH];

		if (!g_emod_entrys[emod_id].name)
		{
			wsprintf(obj_model_name, L"%ws\\unk_%d.obj", path_where_to_save, unk_file_name_counter++);
		}
		else
		{
			wsprintf(obj_model_name, L"%ws\\%S.obj", path_where_to_save, g_emod_entrys[emod_id].name);
		}

		HANDLE f2 = CreateFileW(obj_model_name, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if ((GetLastError() != ERROR_SUCCESS) && (GetLastError() != ERROR_ALREADY_EXISTS))
		{
			printf("File create error: %d (%ws)\n", GetLastError(), obj_model_name);
			CloseHandle(f2);
			continue;
		}

		WRITEP("# imported from AVP2010MapViewer \n#Map Dump test \n \n \n", 54);
		WRITEP("mtllib Materials\\Materials.mtl \n \n", 34);
		WRITEP("# vertexes \ng \n", 15);
		

		char vtx_str[200] = { 0 };

		XMVECTOR emod_middle_point = g_XMZero;

		// this mean we need calculate middle point for emod
		// to make all 
		if (type == 2)
		{
			float count_of_chunks_emod = (float)g_mmi_emod_parts[emod_id].MMI_ids.size();

			for (auto k = 0; k < g_mmi_emod_parts[emod_id].MMI_ids.size(); k++)
			{
				DWORD mmi_part_id = g_mmi_emod_parts[emod_id].MMI_ids[k];

				float x_ = mmi_arr[mmi_part_id].bound_x_;
				float x = mmi_arr[mmi_part_id].bound_x;
				float y_ = mmi_arr[mmi_part_id].bound_y_;
				float y = mmi_arr[mmi_part_id].bound_y;
				float z_ = mmi_arr[mmi_part_id].bound_z_;
				float z = mmi_arr[mmi_part_id].bound_z;

				XMVECTOR p1 = { x, y, z, 0.0f };
				XMVECTOR p2 = { x_, y_, z_, 0.0f };

				//middle point
				XMVECTOR p = (p1 + p2) / 2;

				emod_middle_point += p;
			}

			emod_middle_point /= count_of_chunks_emod;
		}

		// type 1 dump with saving global coordinates of objects
		for (auto k = 0; k < g_mmi_emod_parts[emod_id].MMI_ids.size(); k++)
		{
			DWORD mmi_part_id = g_mmi_emod_parts[emod_id].MMI_ids[k];

			unsigned int start_vtx = mmi_arr[mmi_part_id].Min_vertex_index;
			unsigned int end_vtx;
			if (mmi_part_id < t_m - 1)
				end_vtx = mmi_arr[mmi_part_id + 1].Min_vertex_index;
			else
				end_vtx = env_header.size_vertex_buf;

			for (unsigned int i = start_vtx; i < end_vtx; i++)
			{
				sprintf(vtx_str, "v %f %f %f \n", (vertices[i].Pos.x - emod_middle_point.m128_f32[0]),
					-(vertices[i].Pos.y - emod_middle_point.m128_f32[1]), -(vertices[i].Pos.z - emod_middle_point.m128_f32[2])); // Z is negative
				WRITEP(vtx_str, strlen(vtx_str));
				float U = (float)(vertices[i].TexCoord1.x);
				float V = (float)(vertices[i].TexCoord1.y);
				sprintf(vtx_str, "vt %f %f \n", U, 1.0f - V); // U, V texture V again is neg somewhy
				WRITEP(vtx_str, strlen(vtx_str));
				float X = (float)((float)vertices[i].Normal[0] / 32768.0f) - 1.0f;
				float Y = (float)((float)vertices[i].Normal[1] / 32768.0f) - 1.0f;
				float Z = (float)((float)vertices[i].Normal[2] / 32768.0f) - 1.0f;
				sprintf(vtx_str, "vn %f %f %f \n", X, Y, Z); // Normal
				WRITEP(vtx_str, strlen(vtx_str));
			}
		}

		char face_str[200] = { 0 };
		char part_num_str[200] = { 0 };
		char usemtl_str[100] = { 0 };

		WRITEP("# faces \n\n\n\n", 12);

		int start_index_2 = 0;
		int part_num = 0;

		// vertex layout is TRIANGLELIST
		for (auto k = 0; k < g_mmi_emod_parts[emod_id].MMI_ids.size(); k++)
		{
			DWORD mmi_part_id = g_mmi_emod_parts[emod_id].MMI_ids[k];

			sprintf(part_num_str, "g %s.part%d \n", g_emod_entrys[emod_id].name, part_num++);
			WRITEP(part_num_str, strlen(part_num_str));

			DWORD mat_hash = mmi_arr[mmi_part_id].mat1_hash;

			sprintf(usemtl_str, "usemtl material_%08x \n", mat_hash);
			WRITEP(usemtl_str, strlen(usemtl_str));

			int start_index = mmi_arr[mmi_part_id].start_index;

			int end_index = start_index + mmi_arr[mmi_part_id].triangles_count * 3;

			for (int j = start_index; j <= end_index; j += 3)
			{
				WORD _1 = indexes[j + 0] + 1 + start_index_2;
				WORD _2 = indexes[j + 1] + 1 + start_index_2;
				WORD _3 = indexes[j + 2] + 1 + start_index_2;


				//sprintf(face_str, "f %d/%d/%d %d/%d/%d %d/%d/%d \n", _2, _2, _2, _1, _1, _1, _3, _3, _3);
				sprintf(face_str, "f %d/%d/%d %d/%d/%d %d/%d/%d \n", _1, _1, _1, _2, _2, _2, _3, _3, _3);
				WRITEP(face_str, strlen(face_str));
			}

			start_index_2 += mmi_arr[mmi_part_id].Num_vertices;
			//start_index_2 += mmi_arr[mmi_part_id].triangles_count * 3;
		}

		CloseHandle(f2);
		
	}

	_aligned_free(mmi_arr);
	_aligned_free(vertices);
	_aligned_free(indexes);

	dbgprint("Map_dump", "exporting done !!! \n");

	CloseHandle(f);
	return true;
}


DWORD Command_dump_map(DWORD* args)
{
	DWORD type_of_dump = args[0];

	if (type_of_dump != 1 && type_of_dump != 2)
	{
		dbgprint("Command_dump_map", "Error argument can be 1 or 2 only !\n");
		return 0;
	}


	if (!Dump_Entire_Map_to_Models(type_of_dump))
	{
		dbgprint("Command_dump_map", "Error !\n");
	}

	return 0;
}