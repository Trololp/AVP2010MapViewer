#include "DLEV_load.h"
#include "Debug.h"

//Dialogue events file (not need now but in future maybe i will add support to it)
//Structs

struct sub_dlev
{
	char* name;
	DWORD hash;
	//etc...
};

struct dlev_file
{
	char* EV_name;
	DWORD hash;
	uint32_t sub_event_count;
	sub_dlev** sub_dlev_list;
};


extern wchar_t* g_path2;
extern int read_padded_str(HANDLE file, char* dest);

void load_dlev_file(wchar_t* file_name)
{
	dbgprint("DLEV", "!!!!!!!!!!!!!!!!!!!!!!!!! DIALOG EVENT LIST !!!!!!!!!!!!!!!!!!!!!!!!! \n");
	wchar_t file_path[MAX_PATH] = { 0 };
	wsprintf(file_path, L"%ws\\DLEV_chunk\\", g_path2);
	lstrcat(file_path, file_name);
	dbgprint("DLEV", "Trying load %ws \n", file_path);
	DWORD a = 0;
	HANDLE f = CreateFileW(file_path, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (GetLastError() != ERROR_SUCCESS)
	{
		dbgprint("SMSG", "Opening file Error: %d\n", GetLastError());
		return;
	}

	struct asura_header16 {
		DWORD magic;
		DWORD size;
		DWORD type1;
		DWORD type2;
	};
	asura_header16 header;
	READ(header);

	if (header.magic != (DWORD)'VELD' || header.type1 > 5)
	{
		dbgprint("DLEV", "Opening file Error: INCORRECT FILE HEADER\n");
		return;
	}

	char* EV_name = (char*)malloc(128);
	read_padded_str(f, EV_name);
	dbgprint("DLEV", "%s\n", EV_name);
}

int load_DLEVs(const wchar_t * folder_path)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(folder_path, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		dbgprint("SMSG", "FindFirstFile failed (%d) %ws\n", GetLastError(), folder_path);
		return 0;
	}

	do {
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;
		else
			load_dlev_file(FindFileData.cFileName);
	} while (FindNextFile(hFind, &FindFileData) != 0);

	FindClose(hFind);
	return 1;
}
