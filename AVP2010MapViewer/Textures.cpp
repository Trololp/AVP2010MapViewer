#include "Textures.h"
#include "Debug.h"
#include "Hash_tree.h"
#include "DDSLoader.h"



extern avp_texture AVP_TEXTURES[1500];
extern hash_tree_node* hash_tree_texture;
extern ID3D11Device*           g_pd3dDevice;
extern wchar_t* g_path1;
extern wchar_t* g_path2;
int g_i = 0;

void load_texture(const wchar_t* file_name, int type)
{
	wchar_t* texture_path = (wchar_t*)malloc(MAX_PATH);
	lstrcpy(texture_path, file_name);


	HRESULT hr = CreateDDSTextureFromFile(g_pd3dDevice, file_name, nullptr, &(AVP_TEXTURES[g_i].pSRView));
	wchar_t file_name2[260] = { 0 };
	if (type == 0)
	{
		int len1 = lstrlen(g_path1);
		lstrcpyW(file_name2, &(file_name[len1]));
	}
	else
	{
		int len2 = lstrlen(g_path2);
		lstrcpyW(file_name2, &(file_name[len2]));
	}
	wchar_t* str1 = std::wcschr(file_name2, L'.');
	int len3 = lstrlen(str1);
	wchar_t file_name3[260] = { 0 };
	lstrcpyn(file_name3, &(file_name2[9]), lstrlen(file_name2) - len3 + 1 - 9);
	char file_name4[260] = { 0 };
	wcstombs(file_name4, file_name3, lstrlen(file_name3)); // im so sorry

	DWORD hash = hash_from_str(0, file_name4);
	if (FAILED(hr))
	{
		dbgprint("Textures", "Failed Load texture %ws : hr =  %x : id = %d : hash = %x \n", file_name, hr, g_i, hash);
	}
	else
	{
		AVP_TEXTURES[g_i].hash = hash;
		AVP_TEXTURES[g_i].id = g_i;
		AVP_TEXTURES[g_i].path = texture_path;
		//dbgprint("Textures", "Successful load texture %ws : %x : id = %d : hash = %x \n", file_name, &(AVP_TEXTURES[g_i].pSRView), g_i, AVP_TEXTURES[g_i].hash);
		hash_tree_node* hn = new hash_tree_node;
		hn->Hash = hash;
		hn->texture = &(AVP_TEXTURES[g_i]);
		add_hash(hash_tree_texture, hn);
	}
	g_i++;
}



int Read_textures(const wchar_t* start_folder, int type)
{
	wchar_t search_path[260] = { 0 };
	lstrcpy(search_path, start_folder);
	lstrcat(search_path, L"*.*");
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(search_path, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		dbgprint("Textures", "FindFirstFile failed (%d)\n", GetLastError());
		return 0;
	}

	do {
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (!lstrcmp(FindFileData.cFileName, L".") || !lstrcmp(FindFileData.cFileName, L".."))
				continue;
			wchar_t next_path[260] = { 0 };
			lstrcpy(next_path, start_folder);
			lstrcat(next_path, FindFileData.cFileName);
			lstrcat(next_path, L"\\");
			Read_textures(next_path, type);
		}
		else
		{
			wchar_t path[260] = { 0 };
			lstrcpy(path, start_folder);
			lstrcat(path, FindFileData.cFileName);
			load_texture(path, type);
		}
	} while (FindNextFile(hFind, &FindFileData) != 0);

	FindClose(hFind);
	return 1;
}

