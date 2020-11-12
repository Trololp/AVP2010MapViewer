#pragma once
#include "AVP2010MapViewer.h"

enum ENTITY_TYPE
{
	ENTITY_NONE,
	ENTITY_GLOBAL,
	ENTITY_POINT,
	ENTITY_AREA,
	ENTITY_UNKNOWN
};

struct ENTI_header
{
	DWORD Magic; // b'ENTI'
	DWORD file_size;
	DWORD file_type;
	DWORD file_type2;
	DWORD seq_id;
	DWORD ent_id;
};

struct SMSG_entry
{
	DWORD dw1;
	DWORD type;
	DWORD dw2;
	DWORD Count; //subentries
};

struct SMSG_subentry
{
	WORD w1; // type
	WORD w2; // count
	DWORD seq_id;
	float f1;
	float f2;
};

struct entity_prop
{
	XMFLOAT3 pos;
	XMFLOAT4 angle;
	DWORD group_id; // 1031 is common i think //this is flags 1025 and 1027 object is movable 0000 0100 0000 0101
	DWORD model_hash;//                                                               flags:			      ^   -not movable
	DWORD special;
	bool visible;
};

struct entity_p
{
	DWORD seq_id;
	DWORD id;
};

struct entity
{
	char* name;
	char* data;
	DWORD ent_id;
	DWORD seq_id;
	XMFLOAT3 pos;
	XMFLOAT4 color;
	int connect_count;
	bool connected;
	DWORD* connected_list; //seq ids
	std::vector<entity*> connect_list;
	std::vector<entity*> backward_connect_list;
	entity_prop* model = nullptr;
	bbox* bbox = nullptr;
};

int load_enti(const wchar_t* folder_path);
int load_smsg(const wchar_t* folder_path);
void load_entity(wchar_t* file_path);
void Entity_update_vertex_buffer(DWORD id);
void Entity_delete_all();

