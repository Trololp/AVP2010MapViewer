#include "Models.h"
#include "Debug.h"
#include "Hash_tree.h"

extern ID3D11Device*           g_pd3dDevice;
extern hash_tree_node2* model_hashs;
extern wchar_t* g_path2;

int Read_models(const wchar_t* models_folder_path)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(models_folder_path, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		dbgprint("Models", "FindFirstFile failed (%d)\n", GetLastError());
		return 0;
	}

	do {
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;
		else
			Read_model(FindFileData.cFileName, 0);
	} while (FindNextFile(hFind, &FindFileData) != 0);

	FindClose(hFind);
	return 1;
}



int read_padded_str(HANDLE file, char* dest)
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

void Read_model(const wchar_t* model_name, int skip)
{
	wchar_t file_path[MAX_PATH] = { 0 };
	if (!skip)
	{
		wsprintf(file_path, L"%ws\\", g_path2);
		lstrcat(file_path, model_name);
	}
	else
	{
		lstrcpy(file_path, model_name);
	}
	DWORD bytes_readen = 0;

	HANDLE file = CreateFileW(file_path, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (GetLastError() != ERROR_SUCCESS)
	{
		dbgprint("Models", "Opening file Error: %d\n", GetLastError());
		return;
	}
	DWORD* asura_header = (DWORD*)malloc(28);
	ReadFile(file, asura_header, 28, &bytes_readen, NULL);
	
	if (*asura_header != (DWORD)'FCSR' || *(asura_header + 5) != (DWORD)0xF)
	{
		dbgprint("Models", "Opening file Error: BAD_HEADER (HEADER: %08x Type %02d BytesReaden: %d)\n", *asura_header, *(asura_header + 5), bytes_readen);
		CloseHandle(file);
		return;
	}

	free(asura_header);

	char* name_str = (char*)malloc(260);
	read_padded_str(file, name_str);
	//dbgprint("Model", "RSCF package name: %s \n", name_str);
	model_header mdl_header;
	model_info* mdl_info = new model_info;
	mdl_info->mdl_name = name_str;
	mdl_info->empty_model = 0;
	

	if (ReadFile(file, &mdl_header, sizeof(model_header), &bytes_readen, NULL))
	{
		// 
		mdl_info->vertex_layout = mdl_header.vertex_layout;


		//dbgprint("Models", "Model header: mmi_count: %d, Vertexes: %d, Indexes: %d \n", mdl_header.model_mesh_info_count, mdl_header.vertex_count, mdl_header.index_count);
		if (mdl_header.model_mesh_info_count == 0)
		{
			dbgprint("Model", "Empty model \n");
			mdl_info->empty_model = 1;
			DWORD hash = hash_from_str(0, name_str);
			hash_tree_node2* h_n = new hash_tree_node2;
			h_n->hash = hash;
			h_n->ptr = mdl_info;
			add_hash2(model_hashs, h_n);
			CloseHandle(file);
			return;
		}
		model_mat_info* mmi_buff = (model_mat_info*)_aligned_malloc(mdl_header.model_mesh_info_count * sizeof(model_mat_info), 16);
		if (!ReadFile(file, mmi_buff, mdl_header.model_mesh_info_count * sizeof(model_mat_info), &bytes_readen, NULL))
		{
			dbgprint("Models", "Read mmi error: %d \n", GetLastError());
			free(name_str);
			CloseHandle(file);
			return;
		}
		mdl_info->mmi = mmi_buff;
		//dbgprint("Models", "mmi[0] dump : %X, %d, index_count: %d, %d, %d, %d\n", mmi_buff[0].mat_hash, mmi_buff[0].unk, mmi_buff[0].points_count, mmi_buff[0].unk2,mmi_buff[0].some_offset, mmi_buff[0].unk3);
		mdl_info->mmi_count = mdl_header.model_mesh_info_count;

		void* vertex_buff = _aligned_malloc(mdl_header.vertex_count * 64, 16);
		if (!ReadFile(file, vertex_buff, mdl_header.vertex_count * 64, &bytes_readen, NULL))
		{
			dbgprint("Models", "Read vertex error: %d \n", GetLastError());
			free(name_str);
			CloseHandle(file);
			return;
		}

		// Create vertex buffer
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = 64 * mdl_header.vertex_count;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		bd.StructureByteStride = 0;

		ID3D11Buffer* v_buff;

		D3D11_SUBRESOURCE_DATA InitData = {};
		InitData.pSysMem = vertex_buff;
		InitData.SysMemPitch = 0;
		HRESULT hr;
		hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &v_buff);
		if (FAILED(hr))
		{
			dbgprint("Model", "Create vertex buffer failed !!!\n");
			_aligned_free(vertex_buff);
			free(name_str);
			CloseHandle(file);
			return;
		}
		_aligned_free(vertex_buff);

		mdl_info->pVertexBuff = v_buff;

		void* index_buff = _aligned_malloc(mdl_header.index_count * 2, 16);
		if (!ReadFile(file, index_buff, mdl_header.index_count * 2, &bytes_readen, NULL))
		{
			dbgprint("Models", "Read indexes error: %d \n", GetLastError());
			CloseHandle(file);
			return;
		}

		// Create index buffer
		bd = {};
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = 2 * mdl_header.index_count;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		bd.StructureByteStride = 0;

		ID3D11Buffer* i_buff;

		InitData = {};
		InitData.pSysMem = index_buff;
		InitData.SysMemPitch = 0;
		hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &i_buff);
		if (FAILED(hr))
		{
			dbgprint("Model", "Create index buffer failed !!!\n");
			_aligned_free(index_buff);
			free(name_str);
			CloseHandle(file);
			return;
		}
		_aligned_free(index_buff);

		mdl_info->pIndexBuff = i_buff;
		
		DWORD hash = hash_from_str(0, name_str);
		//dbgprint("debug", "%s - %d (%x) \n", name_str, hash, hash);
		hash_tree_node2* h_n = new hash_tree_node2;
		h_n->hash = hash;
		h_n->ptr = mdl_info;
		add_hash2(model_hashs, h_n);
	}

	CloseHandle(file);
	return;
}


void Delete_models_data(hash_tree_node2* h_n)
{
	if (h_n)
	{
		if (h_n->bigger)
			Delete_models_data(h_n->bigger);
		if (h_n->smaller)
			Delete_models_data(h_n->smaller);
		model_info* mdl_info = (model_info*)h_n->ptr;
		if (mdl_info->empty_model)
		{
			if (mdl_info->mdl_name)
				free(mdl_info->mdl_name);
			return;
		}
		if (mdl_info->pIndexBuff)
			mdl_info->pIndexBuff->Release();
		if (mdl_info->pVertexBuff)
			mdl_info->pVertexBuff->Release();
		if (mdl_info->mmi)
			_aligned_free(mdl_info->mmi);
		if (mdl_info->mdl_name)
			free(mdl_info->mdl_name);
	}
	return;
}

void Delete_models()
{
	Delete_models_data(model_hashs);
	delete_nodes2(model_hashs);
	return;
}