#include "Entity_loader.h"
#include "Debug.h"
#include "Hash_tree.h"
#include "Models.h"
#include "Console.h"


typedef struct {
	DWORD id = 0;
	ENTITY_TYPE type;
} entity_info;

struct e_act
{
	WORD act_id;
	WORD msgid;
};

DWORD Command_find_entity(DWORD*);
DWORD Command_dump_smsg(DWORD*);

Command cmd_find_entity = {
	"find",
	CONCMD_RET_ZERO,
	CONARG_INT,
	Command_find_entity,
	nullptr
};

Command cmd_dump_smsg = {
	"dump_smsg",
	CONCMD_RET_ZERO,
	CONARG_END,
	Command_dump_smsg,
	nullptr
};

extern std::vector<bbox> g_bboxes;
extern std::vector<line> g_lines;
extern std::vector<entity_prop> g_props;
extern std::vector<entity> g_entities;
extern ID3D11Device*           g_pd3dDevice;
extern ID3D11DeviceContext*    g_pImmediateContext;
extern ID3D11Buffer*			g_dynamic_buffer_lines;
extern hash_tree_node2* model_hashs;
extern hash_tree_node2*		g_default_model;
extern int read_padded_str(HANDLE file, char* dest);
extern wchar_t* g_path2;
extern DWORD					g_selected_entity_id;
extern Console*				g_pConsole;

std::vector<entity_p> g_entity_p;
float g_default_row = 0.0f;
float g_default_column = 0.0f;
wchar_t* g_curr_file;
hash_tree_node2* g_actornames_hashs = nullptr;
bool g_actornames_loaded = false;


// SMSG table
// count / ptr / ptr2 /; ptr for id < 30000 / ptr2 for id > 30000
// ptr -> [p1, p2 ... pn]
// p1 -> SMSG_subentry array [e1, e2, ... en]

int SMSG_count1 = 0;
int SMSG_count2 = 0;
void** SMSG_ptr1;
void** SMSG_ptr2;

entity_info e_info[180] = {
	{ 0, ENTITY_UNKNOWN },
	{ 1, ENTITY_GLOBAL },
	{ 2, ENTITY_UNKNOWN },
	{ 3, ENTITY_GLOBAL },
	{ 4, ENTITY_UNKNOWN },
	{ 5, ENTITY_UNKNOWN },
	{ 6, ENTITY_UNKNOWN },
	{ 7, ENTITY_POINT },
	{ 8, ENTITY_UNKNOWN },
	{ 9, ENTITY_UNKNOWN },
	{ 10, ENTITY_UNKNOWN },
	{ 11, ENTITY_GLOBAL },
	{ 12, ENTITY_UNKNOWN },
	{ 13, ENTITY_UNKNOWN },
	{ 14, ENTITY_POINT },
	{ 15, ENTITY_UNKNOWN },
	{ 16, ENTITY_UNKNOWN },
	{ 17, ENTITY_UNKNOWN },
	{ 18, ENTITY_UNKNOWN },
	{ 19, ENTITY_UNKNOWN },
	{ 20, ENTITY_POINT },
	{ 21, ENTITY_POINT },
	{ 22, ENTITY_UNKNOWN },
	{ 23, ENTITY_UNKNOWN },
	{ 24, ENTITY_UNKNOWN },
	{ 25, ENTITY_UNKNOWN },
	{ 26, ENTITY_UNKNOWN },
	{ 27, ENTITY_UNKNOWN },
	{ 28, ENTITY_UNKNOWN },
	{ 29, ENTITY_UNKNOWN },
	{ 30, ENTITY_UNKNOWN },
	{ 31, ENTITY_UNKNOWN },
	{ 32, ENTITY_GLOBAL },
	{ 33, ENTITY_POINT },
	{ 34, ENTITY_GLOBAL },
	{ 35, ENTITY_POINT },
	{ 36, ENTITY_UNKNOWN },
	{ 37, ENTITY_UNKNOWN },
	{ 38, ENTITY_UNKNOWN },
	{ 39, ENTITY_UNKNOWN },
	{ 40, ENTITY_UNKNOWN },
	{ 41, ENTITY_UNKNOWN },
	{ 42, ENTITY_GLOBAL },
	{ 43, ENTITY_UNKNOWN },
	{ 44, ENTITY_UNKNOWN },
	{ 45, ENTITY_UNKNOWN },
	{ 46, ENTITY_UNKNOWN },
	{ 47, ENTITY_UNKNOWN },
	{ 48, ENTITY_UNKNOWN },
	{ 49, ENTITY_UNKNOWN },
	{ 50, ENTITY_UNKNOWN },
	{ 51, ENTITY_UNKNOWN },
	{ 52, ENTITY_UNKNOWN },
	{ 53, ENTITY_UNKNOWN },
	{ 54, ENTITY_UNKNOWN },
	{ 55, ENTITY_UNKNOWN },
	{ 56, ENTITY_UNKNOWN },
	{ 57, ENTITY_UNKNOWN },
	{ 58, ENTITY_UNKNOWN },
	{ 59, ENTITY_UNKNOWN },
	{ 60, ENTITY_UNKNOWN },
	{ 61, ENTITY_UNKNOWN },
	{ 62, ENTITY_UNKNOWN },
	{ 63, ENTITY_POINT },
	{ 64, ENTITY_POINT },
	{ 65, ENTITY_UNKNOWN },
	{ 66, ENTITY_UNKNOWN },
	{ 67, ENTITY_UNKNOWN },
	{ 68, ENTITY_GLOBAL },
	{ 69, ENTITY_POINT },
	{ 70, ENTITY_POINT },
	{ 71, ENTITY_POINT },
	{ 72, ENTITY_POINT },
	{ 73, ENTITY_UNKNOWN },
	{ 74, ENTITY_GLOBAL },
	{ 75, ENTITY_POINT },
	{ 76, ENTITY_UNKNOWN },
	{ 77, ENTITY_POINT },
	{ 78, ENTITY_POINT },
	{ 79, ENTITY_UNKNOWN },
	{ 80, ENTITY_UNKNOWN },
	{ 81, ENTITY_UNKNOWN },
	{ 82, ENTITY_UNKNOWN },
	{ 83, ENTITY_POINT },
	{ 84, ENTITY_UNKNOWN },
	{ 85, ENTITY_GLOBAL },
	{ 86, ENTITY_UNKNOWN },
	{ 87, ENTITY_UNKNOWN },
	{ 88, ENTITY_UNKNOWN },
	{ 89, ENTITY_UNKNOWN },
	{ 90, ENTITY_UNKNOWN },
	{ 91, ENTITY_UNKNOWN },
	{ 92, ENTITY_POINT },
	{ 93, ENTITY_UNKNOWN },
	{ 94, ENTITY_POINT },
	{ 95, ENTITY_POINT },
	{ 96, ENTITY_UNKNOWN },
	{ 97, ENTITY_POINT },
	{ 98, ENTITY_UNKNOWN },
	{ 99, ENTITY_UNKNOWN },
	{ 100, ENTITY_POINT },
	{ 101, ENTITY_UNKNOWN },
	{ 102, ENTITY_UNKNOWN },
	{ 103, ENTITY_UNKNOWN },
	{ 104, ENTITY_UNKNOWN },
	{ 105, ENTITY_UNKNOWN },
	{ 106, ENTITY_UNKNOWN },
	{ 107, ENTITY_UNKNOWN },
	{ 108, ENTITY_UNKNOWN },
	{ 109, ENTITY_UNKNOWN },
	{ 110, ENTITY_UNKNOWN },
	{ 111, ENTITY_UNKNOWN },
	{ 112, ENTITY_UNKNOWN },
	{ 113, ENTITY_UNKNOWN },
	{ 114, ENTITY_POINT },
	{ 115, ENTITY_POINT },
	{ 116, ENTITY_UNKNOWN },
	{ 117, ENTITY_UNKNOWN },
	{ 118, ENTITY_UNKNOWN },
	{ 119, ENTITY_UNKNOWN },
	{ 120, ENTITY_UNKNOWN },
	{ 121, ENTITY_UNKNOWN },
	{ 122, ENTITY_POINT },
	{ 123, ENTITY_UNKNOWN },
	{ 124, ENTITY_POINT },
	{ 125, ENTITY_UNKNOWN },
	{ 126, ENTITY_POINT },
	{ 127, ENTITY_UNKNOWN },
	{ 128, ENTITY_UNKNOWN },
	{ 129, ENTITY_UNKNOWN },
	{ 130, ENTITY_GLOBAL },
	{ 131, ENTITY_UNKNOWN },
	{ 132, ENTITY_POINT },
	{ 133, ENTITY_GLOBAL },
	{ 134, ENTITY_UNKNOWN },
	{ 135, ENTITY_POINT },
	{ 136, ENTITY_POINT },
	{ 137, ENTITY_UNKNOWN },
	{ 138, ENTITY_POINT },
	{ 139, ENTITY_UNKNOWN },
	{ 140, ENTITY_POINT },
	{ 141, ENTITY_POINT },
	{ 142, ENTITY_POINT },
	{ 143, ENTITY_UNKNOWN },
	{ 144, ENTITY_POINT },
	{ 145, ENTITY_POINT },
	{ 146, ENTITY_POINT },
	{ 147, ENTITY_GLOBAL },
	{ 148, ENTITY_POINT },
	{ 149, ENTITY_GLOBAL },
	{ 150, ENTITY_UNKNOWN },
	{ 151, ENTITY_POINT },
	{ 152, ENTITY_UNKNOWN },
	{ 153, ENTITY_UNKNOWN },
	{ 154, ENTITY_UNKNOWN },
	{ 155, ENTITY_UNKNOWN },
	{ 156, ENTITY_POINT },
	{ 157, ENTITY_POINT },
	{ 158, ENTITY_POINT },
	{ 159, ENTITY_UNKNOWN },
	{ 160, ENTITY_UNKNOWN },
	{ 161, ENTITY_UNKNOWN },
	{ 162, ENTITY_POINT },
	{ 163, ENTITY_POINT },
	{ 164, ENTITY_UNKNOWN },
	{ 165, ENTITY_POINT },
	{ 166, ENTITY_POINT },
	{ 167, ENTITY_POINT },
	{ 168, ENTITY_UNKNOWN },
	{ 169, ENTITY_POINT },
	{ 170, ENTITY_POINT },
	{ 171, ENTITY_UNKNOWN },
	{ 172, ENTITY_UNKNOWN },
	{ 173, ENTITY_POINT },
	{ 174, ENTITY_UNKNOWN },
	{ 175, ENTITY_UNKNOWN },
	{ 176, ENTITY_POINT },
	{ 177, ENTITY_UNKNOWN },
	{ 178, ENTITY_UNKNOWN },
	{ 179, ENTITY_UNKNOWN }
};


int ent_id2 = 0;

void Create_entity(char* name, char* data, DWORD ent_id, DWORD seq_id, XMFLOAT3 pos, XMVECTOR color = Colors::White, entity_prop* model = nullptr, bbox* bbox = nullptr, int connected_count = 0, DWORD* connected_list = nullptr)
{
	entity ent;
	ent.name = name;
	ent.data = data;
	ent.ent_id = ent_id;
	ent.seq_id = seq_id;
	ent.connected = false;
	ent.pos = pos;
	ent.ent_id2 = ent_id2;
	XMStoreFloat4(&(ent.color), color);
	if (model)
		ent.model = model;
	if (bbox)
		ent.bbox = bbox;
	ent.connect_count = connected_count;
	ent.connected_list = connected_list;
	entity_p ent_p;
	ent_p.id = ent_id2;
	ent_p.seq_id = seq_id;
	g_entities.push_back(ent);
	g_entity_p.push_back(ent_p);
	ent_id2++;
}

void Create_global_entity(char* name, char* data, DWORD ent_id, DWORD seq_id, XMVECTOR Color = Colors::Cyan, int connected_count = 0, DWORD* connected_list = nullptr)
{
	if (g_default_column > 10.0f)
	{
		g_default_column = 0.0f;
		g_default_row += 1.0f;
	}
	Create_entity(name, data, ent_id, seq_id, { g_default_column, 0.0f,  g_default_row}, Color, nullptr, nullptr, connected_count, connected_list);
	g_default_column += 1.0f;
}

void Entity_delete_all()
{
	for (std::vector<entity>::iterator it = g_entities.begin(); it != g_entities.end(); ++it)
	{
		//if((*it).name)		why do it crashes on it idk
		//	free((*it).name);
		if((*it).data)
			free((*it).data);
		if((*it).model)
			delete (*it).model;
		if((*it).bbox)
			delete (*it).bbox;
		if((*it).connected_list)
			free((*it).connected_list);
		std::vector<entity*>().swap((*it).connect_list);
		std::vector<entity*>().swap((*it).backward_connect_list);
	}
	std::vector<entity>().swap(g_entities);
}

bool compvar(entity_p const& one, entity_p const& two)
{
	return one.seq_id < two.seq_id;
}

bool compvar2(const entity_p& one, DWORD value)
{
	return one.seq_id < value;
}

entity* Search_for_entity(DWORD seq_id)
{
	std::vector<entity_p>::iterator it = std::lower_bound(g_entity_p.begin(), g_entity_p.end(), seq_id, &compvar2);
	if (it == g_entity_p.end())
		return 0;
	return &(g_entities[(*it).id]);
}

DWORD Command_find_entity(DWORD* args)
{
	DWORD ent_id = args[0];
	entity* pEnt = Search_for_entity(ent_id);
	if (!pEnt)
		return 0;
	g_selected_entity_id = pEnt->ent_id2;
	Entity_update_vertex_buffer(g_selected_entity_id);
	float* args2 = (float*)malloc(16 * 4);

	args2[0] = pEnt->pos.x;
	args2[1] = pEnt->pos.y;
	args2[2] = pEnt->pos.z;
	Console_command_tp((DWORD*)args2);
	g_pConsole->is_active = false;
	free(args2);
	return 0;
}

DWORD Command_dump_smsg(DWORD* args)
{
	void* entries_ptr = nullptr;
	for (int msg_id = 0; msg_id < SMSG_count1; msg_id++)
	{
		entries_ptr = SMSG_ptr1[msg_id];
		if (entries_ptr == nullptr)
		{
			dbgprint("Dump_smsg", "MSGID: %d\n", msg_id);
			return 0;
		}
		DWORD count = *(DWORD*)entries_ptr;
		SMSG_subentry* entries = (SMSG_subentry*)((DWORD*)entries_ptr + 1);
		dbgprint("Dump_smsg", "MSGID: %d\n", msg_id);
		for (int i = 0; i < count; i++)
		{
			entity* ent_p = Search_for_entity(entries[i].seq_id);
			if (ent_p != nullptr)
			{
				dbgprint("Dump_smsg", "%d: act:(%04x %hu) %.5f ent: %d %s", i, entries[i].w1, entries[i].w2, entries[i].f1, entries[i].seq_id, ent_p->name);
			}else
			{
				dbgprint("Dump_smsg", "%d: act:(%04x %hu) %.5f ent: %d unknown \n", i, entries[i].w1, entries[i].w2, entries[i].f1, entries[i].seq_id);
			}
			if (entries[i].data != nullptr)
			{
				switch (entries[i].w1)
				{
				case 0x7B:
					Dump_hex("Dump_smsg", entries[i].data, 84);
					break;
				case 0x7C:
				case 0x7D:
				case 0x7E:
				case 0x7F:
					Dump_hex("Dump_smsg", entries[i].data, 40);
					break;
				case 0x80:
					Dump_hex("Dump_smsg", entries[i].data, 80);
					break;
				default:
					WORD w2 = (entries[i].w2 + 3) & 0xFFFFFFFC;
					if(w2)
						Dump_hex("Dump_smsg", entries[i].data, w2);
					break;
				}
			}
		}
	}
	return 0;
}

//Returns pointer to array of seq_ids of entities affected by that msgid + some additional space if needed
DWORD* search_in_SMSG(WORD msgid, int additional_ids, DWORD* founded)
{
	if (msgid == 0xFFFF)
		return nullptr;

	if (msgid < 0 || msgid > 60000)
		return nullptr;

	void* entries_ptr = nullptr;
	if (msgid < 30000)
	{
		if (msgid >= SMSG_count1)
		{
			dbgprint("SMSG", "check 1 msg_id = %d\n", msgid);
			return nullptr;
		}
		entries_ptr = SMSG_ptr1[msgid];
	}
	else
	{
		if (msgid - 30000 >= SMSG_count2)
		{
			dbgprint("SMSG", "check 2 msg_id = %d (%d)\n", msgid, msgid - 30000);
			return nullptr;
		}
		entries_ptr = SMSG_ptr2[msgid - 30000];
	}
	uint32_t count = *(DWORD*)entries_ptr;
	*founded = count; // give information about size of buffer
	if (count > 0x8000)
	{
		dbgprint("SMSG", "check  count = %d for msg_id = %d\n", count, msgid);
	}
	DWORD* seq_ids_buff = (DWORD*)malloc(count * 4 + additional_ids * 4);
	ZeroMemory(seq_ids_buff, count * 4 + additional_ids * 4);
	SMSG_subentry* entries = (SMSG_subentry*)((DWORD*)entries_ptr + 1);
	for (int i = 0; i < count; i++)
	{
		seq_ids_buff[i] = entries[i].seq_id;
	}
	return seq_ids_buff;
}

char* dump_msg(WORD msgid)
{
	//return (char*)"no dump !\n";
	if (msgid == 0xFFFF)
		return (char*)"no data \n";

	if (msgid < 0 || msgid > 60000)
		return (char*)"no data \n";

	void* entries_ptr = nullptr;
	if (msgid < 30000)
	{
		if (msgid >= SMSG_count1)
		{
			dbgprint("SMSG", "check 1 msg_id = %d\n", msgid);
			return (char*)"no data \n";
		}
		entries_ptr = SMSG_ptr1[msgid];
	}
	else
	{
		if (msgid - 30000 >= SMSG_count2)
		{
			dbgprint("SMSG", "check 2 msg_id = %d (%d)\n", msgid, msgid - 30000);
			return (char*)"no data \n";
		}
		entries_ptr = SMSG_ptr2[msgid - 30000];
	}
	uint32_t count = *(DWORD*)entries_ptr;
	int founded = count; // give information about size of buffer
	if (count > 1000)
	{
		dbgprint("SMSG", "check  count = %d for msg_id = %d\n", count, msgid);
		return (char*)"Unknown \n";
	}
	char* buffer = (char*)malloc(1024);
	ZeroMemory(buffer, 1024);
	char msg_id_line[64];
	sprintf_s(msg_id_line, sizeof(msg_id_line), "msg_id: %05d\nID Cnt ent_id delay? \n", msgid);
	strcat_s(buffer, 1024, msg_id_line);
	SMSG_subentry* entries = (SMSG_subentry*)((DWORD*)entries_ptr + 1);
	
	bool excided_limit = false;

	if (count >= 3)
	{
		count = 3;
		excided_limit = true;
	}
		

	for (int i = 0; i < count; i++)
	{
		char new_line[64];

		sprintf_s(new_line, sizeof(new_line), "0x%04x %02d %07d %.5f \n", entries[i].w1, entries[i].w2, entries[i].seq_id, entries[i].f1);
		strcat_s(buffer, 1024, new_line);
	}

	if (excided_limit)
	{
		strcat_s(buffer, 1024, "...\n");
	}

	return buffer;
}

char* BE_messages(HANDLE f)
{
	DWORD a = 0;
	int count;
	READ(count);
	DWORD* msgs = (DWORD*)malloc(count * 4);
	READP(msgs, count * 4);

	char* buffer = (char*)malloc(1024);
	ZeroMemory(buffer, 1024);
	strcat_s(buffer, 1024, "BE data\n");



	for (int i = 0; i < count; i++)
	{
		char new_line[1024];

		sprintf_s(new_line, sizeof(new_line), "On event_id : 0x%04x \n %s \n", msgs[i] & 0xFFFF, dump_msg(msgs[i] >> 16));
		strcat_s(buffer, 1024, new_line);
	}


	return buffer;
}

char* Actor_name_from_hash(DWORD hash)
{
	char* actor_name = (char*)"Unknown";
	
	if(!g_actornames_loaded)
		return actor_name;

	hash_tree_node2* hn_p = search_by_hash2(g_actornames_hashs, hash);

	if (hn_p)
		return (char*)hn_p->ptr;

	return actor_name;
}

void Perform_connections()
{
	std::sort(g_entity_p.begin(), g_entity_p.end(), &compvar);
	for (std::vector<entity>::iterator it = g_entities.begin(); it != g_entities.end(); ++it)
	{
		if ((*it).connected)
			continue;
		if (!((*it).connected_list))
			continue;
		
		int connect_count = (*it).connect_count;
		DWORD* connect_list = (*it).connected_list;
		//char seq_str[20] = {0};
		//sprintf(seq_str, "From: %d \n", (*it).seq_id);
		for (int i = 0; i < connect_count; i++)
		{
			entity* c_ent = Search_for_entity(connect_list[i]);
			if (!c_ent)
				continue;
			c_ent->backward_connect_list.push_back(&*it);
			//strcat(c_ent->data, seq_str);
			(*it).connect_list.push_back(c_ent);
		}
		(*it).connected = true;
	}
}

void Ent_to_bbox()
{
	for (std::vector<entity>::iterator it = g_entities.begin(); it != g_entities.end(); ++it)
	{
		if ((*it).bbox)
		{
			g_bboxes.push_back(*(*it).bbox);
		}
		else
		{
			g_bboxes.push_back({ XMFLOAT3((*it).pos.x + 0.1f, (*it).pos.y + 0.1f, (*it).pos.z + 0.1f),XMFLOAT3((*it).pos.x - 0.1f, (*it).pos.y - 0.1f, (*it).pos.z - 0.1f), (*it).color });
		}
	}
}

simple_vertex* Make_vertex_buffer(DWORD id, DWORD additional_size)
{
	XMFLOAT3 pos = g_entities[id].pos;
	int vtx_count = g_entities[id].connect_list.size() * 2 + g_entities[id].backward_connect_list.size() * 2 + 24; //24 is size of bbox
	simple_vertex* vertices = (simple_vertex*)_aligned_malloc((vtx_count + additional_size) * sizeof(simple_vertex), 16);
	ZeroMemory(vertices, (vtx_count + additional_size) * sizeof(simple_vertex));
	XMFLOAT4 clr = { 1.0f, 0.0f, 0.0f, 1.0f };
	XMFLOAT3 p1 = { pos.x + 0.1f, pos.y + 0.1f, pos.z + 0.1f };
	XMFLOAT3 p2 = { pos.x - 0.1f, pos.y - 0.1f, pos.z - 0.1f };
	XMFLOAT4 rot = { 0.0f, 0.0f, 0.f, 1.0f };
	XMMATRIX Q = XMMatrixRotationQuaternion(XMLoadFloat4(&(rot)));
	if (g_entities[id].bbox)
	{
		p1 = g_entities[id].bbox->p1;
		p2 = g_entities[id].bbox->p2;
		Q = XMMatrixRotationQuaternion(XMLoadFloat4(&(g_entities[id].bbox->rot)));
	}
	XMVECTOR p1_ = XMLoadFloat3(&p1);
	XMVECTOR p2_ = XMLoadFloat3(&p2);
	
	XMVECTOR p = (p1_ + p2_) / 2;
	XMFLOAT3 a = p1;
	XMFLOAT3 b = XMFLOAT3(p2.x, p1.y, p1.z);
	XMFLOAT3 c = XMFLOAT3(p2.x, p2.y, p1.z);
	XMFLOAT3 d = XMFLOAT3(p1.x, p2.y, p1.z);
	XMFLOAT3 e = XMFLOAT3(p1.x, p1.y, p2.z);
	XMFLOAT3 f = XMFLOAT3(p2.x, p1.y, p2.z);
	XMFLOAT3 g = p2;
	XMFLOAT3 h = XMFLOAT3(p1.x, p2.y, p2.z);
	int r = 0;
	vertices[0 + r] = { a, clr };  vertices[12 + r] = { g, clr };// This code get from Debug.cpp bbox_to_vertex func
	vertices[1 + r] = { b, clr };  vertices[13 + r] = { h, clr };//
	vertices[2 + r] = { b, clr };  vertices[14 + r] = { h, clr };//     h-----g
	vertices[3 + r] = { c, clr };  vertices[15 + r] = { e, clr };//    /|    /|
	vertices[4 + r] = { c, clr };  vertices[16 + r] = { e, clr };//   e-----f |
	vertices[5 + r] = { d, clr };  vertices[17 + r] = { a, clr };//   | d---|-c
	vertices[6 + r] = { d, clr };  vertices[18 + r] = { f, clr };//   |/    |/
	vertices[7 + r] = { a, clr };  vertices[19 + r] = { b, clr };//   a-----b
	vertices[8 + r] = { e, clr };  vertices[20 + r] = { g, clr };//
	vertices[9 + r] = { f, clr };  vertices[21 + r] = { c, clr };//
	vertices[10 + r] = { f, clr };  vertices[22 + r] = { h, clr };//
	vertices[11 + r] = { g, clr };  vertices[23 + r] = { d, clr };//

	for (int i = 0; i < 24; i++)
	{
		XMStoreFloat3(&(vertices[i + r].Pos), XMVector3Transform(XMLoadFloat3(&(vertices[i + r].Pos)) - p, Q) + p);
	}

	r += 24;

	for (std::vector<entity*>::iterator it = g_entities[id].backward_connect_list.begin(); it != g_entities[id].backward_connect_list.end(); ++it)
	{
		XMFLOAT4 clr = { 1.0f, 0.0f, 0.0f, 1.0f };
		XMFLOAT3 p1 = pos;
		XMFLOAT3 p2 = (*it)->pos;

		vertices[0 + r] = { p1, clr }; // (p1) ---- (p2)
		vertices[1 + r] = { p2, clr }; //
		r += 2;
	}

	for (std::vector<entity*>::iterator it = g_entities[id].connect_list.begin(); it != g_entities[id].connect_list.end(); ++it)
	{
		XMFLOAT4 clr = { 0.0f, 1.0f, 0.0f, 1.0f };
		XMFLOAT3 p1 = pos;
		XMFLOAT3 p2 = (*it)->pos;

		vertices[0 + r] = { p1, clr }; // (p1) ---- (p2)
		vertices[1 + r] = { p2, clr }; //
		r += 2;
	}
	
	//dbgprint("Make_vertex_buffer", "vtx_count: %d r = %d seq_id: %d\n", vtx_count, r, g_entities[id].seq_id);
	return vertices;
}

void Ent_make_dynamic_buffer()
{
	HRESULT hr;
	int id = 0;
	int vtx_count = g_entities[id].connect_list.size() * 2 + g_entities[id].backward_connect_list.size() * 2 + 24;
	simple_vertex* vertices = Make_vertex_buffer(id, 100);

	D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
	vertexBufferDesc.ByteWidth = sizeof(simple_vertex)* (vtx_count + 100);
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;
	vertexBufferData.pSysMem = vertices;
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;

	hr = g_pd3dDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &g_dynamic_buffer_lines);
	if (FAILED(hr))
	{
		dbgprint("Entity_make_dynamic_buffer", "Failed to make dynamic buffer %x \n", hr);
		_aligned_free(vertices);
		return;
	}
	_aligned_free(vertices);
}

void Entity_update_vertex_buffer(DWORD id)
{
	int vtx_count = g_entities[id].connect_list.size() * 2 + g_entities[id].backward_connect_list.size() * 2 + 24;
	simple_vertex* vertices = Make_vertex_buffer(id, 100);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	g_pImmediateContext->Map(g_dynamic_buffer_lines, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, vertices, (vtx_count) * sizeof(simple_vertex));
	g_pImmediateContext->Unmap(g_dynamic_buffer_lines, 0);
	_aligned_free(vertices);
}

void Alloc_name_and_data_str(char** name, char** data)
{
	*data = (char*)malloc(2048);
	ZeroMemory(*data, 2048);
	*name = (char*)malloc(128);
	ZeroMemory(*name, 128);
}

bbox* bbox_from_point(XMFLOAT3 pos, float size, XMVECTOR color,XMFLOAT4 rot = { 0.0f, 0.0f, 0.0f, 1.0f })
{
	bbox* e_bbox = new bbox;
	e_bbox->p1 = XMFLOAT3(pos.x + size, pos.y + size, pos.z + size);
	e_bbox->p2 = XMFLOAT3(pos.x - size, pos.y - size, pos.z - size);
	//e_bbox->Color = color;
	XMStoreFloat4(&(e_bbox->Color), color);
	e_bbox->rot = rot;
	return e_bbox;
}

void load_enti_1(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x2C);
	DWORD num;
	READ(num);
	SFPC(num * 4 + 8);

	DWORD founded = 0;
	DWORD* seq_ids = nullptr;

	WORD* msgids = (WORD*)malloc(5 * 2);
	READP(msgids, 5 * 2);
	seq_ids = search_in_SMSG(msgids[2], 0, &founded);
	SFPC(6);
	float f1, f2;
	READ(f1);
	READ(f2);
	DWORD flags_;
	READ(flags_);


	//bool is_frozen = flags_ & 0x2;
	//bool is_enabled = flags_ & 0x1;
	//const char* is_frozen_str = is_frozen ? "frozen" : "clocks from begining";
	//is_frozen_str = is_enabled ? is_frozen_str : "dissabled";
	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	
	

	

	sprintf(name, "trigger_start ? (1)\n");
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n %s \n time: %f %f \n", seq_id, g_curr_file, dump_msg(msgids[2]), f1, f2 );

	Create_global_entity(name, data, 1, seq_id, Colors::Purple, founded, seq_ids);
}

void load_enti_3(HANDLE f, DWORD seq_id)
{
	SFPS(0x30);
	char* str1 = (char*)malloc(128);
	ZeroMemory(str1, 128);
	read_padded_str(f, str1);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "cutscene start (3)\n");
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Name: %s \n ", seq_id, g_curr_file, str1);

	Create_global_entity(name, data, 3, seq_id, Colors::LightGreen);
}

void load_prop(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	
	SFPS(0x54);
	DWORD BE_flags;
	READ(BE_flags);
	const char* BE_str = ((BE_flags & 1)) ? "enabled" : "disabled";

	SFPS(0x58);
	char* BE_data = BE_messages(f);

	entity_prop* prop = new entity_prop;
	prop->visible = 1;
	READ(prop->pos);
	READ(prop->angle);
	SFPC(0x21);
	READ(prop->special);
	SFPC(4);
	READ(prop->model_hash);
	SFPC(0x1C);
	READ(prop->group_id);

	bool movable = (prop->group_id & 0b100) ? 0 : 1;

	hash_tree_node2* node = search_by_hash2(model_hashs, prop->model_hash);

	if (!node)
		node = g_default_model; // Default mdl
	
	model_info* mi = (model_info*)node->ptr;

	const char* str = (movable) ? "true" : "false";

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Entity %s \n Movable: %s \n Flags: %d \n Model_name: \"%s\" \n %s \n", seq_id, g_curr_file, BE_str, str, prop->group_id, mi->mdl_name, BE_data);
	sprintf(name, "Entity Prop (7)\n");
	Create_entity(name, data, 7, seq_id, prop->pos, Colors::White, prop);
}

void load_enti_11(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x3C);
	DWORD count = 0;
	READ(count);

	DWORD found = 0;
	DWORD totaly_found = 0;
	DWORD current_pos = 0;
	DWORD* seq_ids = nullptr;

	for (int i = 0; i < count; i++)
	{
		WORD msgid;
		READ(msgid);
		SFPC(6);
		found = 0;
		search_in_SMSG(msgid, 0, &found);
		totaly_found += found;
	}
	//dbgprint("e_act", "count %d totaly_found %d", count, totaly_found);
	seq_ids = (DWORD*)malloc(totaly_found * 4);
	ZeroMemory(seq_ids, totaly_found * 4);
	SFPS(0x40);

	char* msg_str = (char*)malloc(256);
	ZeroMemory(msg_str, 256);
	char* temp_str = (char*)malloc(16);
	ZeroMemory(temp_str, 16);

	for (int i = 0; i < count; i++)
	{
		WORD msgid;
		READ(msgid);
		SFPC(6);

		sprintf(temp_str, " msg:%d \n", msgid);
		strcat(msg_str, temp_str);

		found = 0;
		DWORD* seq_ids2 = search_in_SMSG(msgid, 0, &found);
		if (seq_ids2 == nullptr)
			continue;
		memcpy(seq_ids + current_pos, seq_ids2, found * 4);
		current_pos += found;
	}
	found = totaly_found;

	

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Entity message (11)\n");
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n%s \n", seq_id, g_curr_file, msg_str);

	free(temp_str);
	free(msg_str);

	Create_global_entity(name, data, 11, seq_id, Colors::Purple, found, seq_ids);
}

void load_enti_14(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x38);
	DWORD* seq_ids = (DWORD*)malloc(4);
	READP(seq_ids, 4);
	XMFLOAT3 pos;
	READ(pos);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Entity . (14)\n");
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);
	Create_entity(name, data, 14, seq_id, pos, Colors::Yellow, nullptr, nullptr, 1, seq_ids);
}

void load_enti_20(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x58);
	asura_bbox bb;
	XMFLOAT4 rot;
	READ(bb);
	READ(rot);
	bbox* bbox_p = bbox_from_asura_bb(&bb, rot, Colors::Red);
	
	XMFLOAT3 pos = { ((bbox_p->p1).x + (bbox_p->p2).x) / 2,
		((bbox_p->p1).y + (bbox_p->p2).y) / 2,
		((bbox_p->p1).z + (bbox_p->p2).z) / 2 };

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Entity . (20)\n");
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);
	Create_entity(name, data, 20, seq_id, pos, Colors::Magenta, nullptr, bbox_p);
}

void load_door_entity(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;

	DWORD* p_activator_seq_id = (DWORD*)malloc(4);
	//SetFilePointer(file, 0x28, NULL, FILE_BEGIN);
	SFPS(0x28);
	//ReadFile(file, p_activator_seq_id, 4, &bytes_readen, NULL);
	READP(p_activator_seq_id, 4);
	//dbgprint("Doors", "Activator_id: %d \n", *p_activator_seq_id);
	//SetFilePointer(file, 0x54, NULL, FILE_BEGIN);
	SFPS(0x54);
	int i;
	READ(i);
	//SetFilePointer(file, 0x58 + i * 4, NULL, FILE_BEGIN);
	SFPC(i * 4);
	entity_prop* prop = new entity_prop;
	prop->visible = 1;
	READ(prop->pos);
	READ(prop->angle);
	SFPC(0x21);
	READ(prop->special);
	SFPC(4);
	READ(prop->model_hash);
	SFPC(0x1C);
	READ(prop->group_id);

	bool movable = (prop->group_id & 0b100) ? 0 : 1;
	const char* str = (movable) ? "true" : "false";

	hash_tree_node2* node = search_by_hash2(model_hashs, prop->model_hash);
	if (!node)
		node = g_default_model; // Default mdl

	model_info* mi = (model_info*)node->ptr;


	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Movable: %s \n Special: %x \n Flags: %d \n Model_name: \"%s\" \n", seq_id, g_curr_file, str, prop->special, prop->group_id, mi->mdl_name);
	sprintf(name, "Entity Movable_obj (21) \n");

	bbox* e_bbox = bbox_from_point(prop->pos, 0.2f, Colors::White);	
	Create_entity(name, data, 21, seq_id, prop->pos, Colors::White, prop, e_bbox, 1, p_activator_seq_id);
}

void load_enti_32(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x30);
	DWORD num;
	READ(num);
	SFPC(num * 4 + 16);
	float f1;
	READ(f1);
	SFPC(8);
	DWORD num2;
	READ(num2);
	for (int i = 0; i < num2; i++)
	{
		SFPC(8);
		asura_bbox bb;
		XMFLOAT4 rot;
		READ(bb);
		READ(rot);
		//bbox_p = bbox_from_asura_bb(&bb, rot, Colors::DarkBlue);
		g_bboxes.push_back({ { bb.x1,bb.y1, bb.z1 },{ bb.x2,bb.y2, bb.z2 }, XMFLOAT4(0.2f, 0.2f, 0.8f, 1.0f), rot });
	}
	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Trigger_hurt (%d)\n", seq_id);
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Damage: %f \n", seq_id, g_curr_file, f1);
	Create_global_entity(name, data, 32, seq_id);
}

void load_enti_34(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x20);
	int num1;
	READ(num1); // Two Section of HASH and some SHORT id
	SFPC(num1 * 6);
	int num2;
	READ(num2);
	SFPC(num2 * 6);
	int seq_id_count;
	READ(seq_id_count);
	DWORD* seq_ids = (DWORD*)malloc(4 * seq_id_count);
	READP(seq_ids, 4 * seq_id_count);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);
	sprintf(name, "Multimanager (34)\n");
	Create_global_entity(name, data, 34, seq_id, Colors::Purple, seq_id_count, seq_ids);
}

void load_enti_35(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x2C);
	DWORD num;
	READ(num);
	SFPC(num * 4 + 8);

	DWORD founded = 0;
	DWORD* seq_ids = nullptr;

	WORD* msgids = (WORD*)malloc(5 * 2);
	READP(msgids, 5 * 2);
	seq_ids = search_in_SMSG(msgids[2], 0, &founded);
	SFPC(6);
	float f1;
	READ(f1);
	SFPS(0x74);
	XMFLOAT3 pos;
	READ(pos);

	g_bboxes.push_back({ { pos.x + 0.2f,pos.y + 0.2f, pos.z + 0.2f },{ pos.x - 0.2f,pos.y - 0.2f, pos.z - 0.2f }, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), {0.0f, 0.0f, 0.0f, 1.0f} });
	bbox* bbox_p = bbox_from_point(pos, f1, Colors::Black);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Trigger (dynamic) (35)\n");
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n radius: %f \n", seq_id, g_curr_file, f1);
	Create_entity(name, data, 35, seq_id, pos, Colors::Red, nullptr, bbox_p, founded, seq_ids);
}

void load_enti_42(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x30);
	char* BE_data = BE_messages(f);
	
	DWORD timer_enabled; // will it work from begining ? or need activation ?
	DWORD will_cycle; // 1 if cycle
	DWORD count;
	READ(timer_enabled);
	READ(will_cycle);
	READ(count);

	DWORD found = 0;
	DWORD totaly_found = 0;
	DWORD current_pos = 0;
	DWORD* seq_ids = nullptr;

	for (int i = 0; i < count; i++)
	{
		SFPC(8);
		WORD msgid;
		READ(msgid);
		SFPC(2);
		found = 0;
		search_in_SMSG(msgid, 0, &found);
		totaly_found += found;
	}
	//dbgprint("e_act", "count %d totaly_found %d", count, totaly_found);
	seq_ids = (DWORD*)malloc(totaly_found * 4);
	ZeroMemory(seq_ids, totaly_found * 4);
	SFPS(0x40);

	char* msg_str = (char*)malloc(512);
	ZeroMemory(msg_str, 512);
	char* temp_str = (char*)malloc(32);
	ZeroMemory(temp_str, 32);

	for (int i = 0; i < count; i++)
	{
		WORD msgid;
		WORD is_entry_enabled;
		float delay;
		DWORD hash;
		READ(hash);
		READ(delay);
		READ(msgid);
		READ(is_entry_enabled);

		sprintf(temp_str, "%x %d %.5f %d\n", hash, msgid, delay, is_entry_enabled);
		strcat(msg_str, temp_str);

		found = 0;
		DWORD* seq_ids2 = search_in_SMSG(msgid, 0, &found);
		if (seq_ids2 == nullptr)
			continue;
		memcpy(seq_ids + current_pos, seq_ids2, found * 4);
		current_pos += found;
	}
	found = totaly_found;

	free(temp_str);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Timer sequence (42)\n");
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n BE_data: %s \n enabled: %d \n cycle: %d \n hash msgid delay enabled \n %s \n", \
		seq_id, g_curr_file, BE_data, timer_enabled, will_cycle, msg_str);
	free(msg_str);
	Create_global_entity(name, data, 42, seq_id, Colors::Cyan, found, seq_ids);
}

//Waypoints
void load_enti_63(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x34);
	int num1;
	READ(num1);
	SFPC(num1 * 4);
	DWORD flags;
	READ(flags);
	XMFLOAT3 pos;
	READ(pos);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Entity path_point (63)\n");

	DWORD state;
	bool reversible = flags & 0x80;
	bool start_node = flags & 0x10;
	const char* str = (reversible) ? "true" : "false";

	if (start_node)
	{
		DWORD* seq_ids = (DWORD*)malloc(4);
		READP(seq_ids, 4);
		SFPC(4);
		READ(state);
		sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n State1: %x\n State2: %x\n Reversible: %s \nStart_node \n", seq_id, g_curr_file, flags, LOWORD(state), HIWORD(state), str);
		Create_entity(name, data, 63, seq_id, pos, Colors::Cyan, nullptr, nullptr, 1, seq_ids);
	}
	else
	{
		SFPC(4);
		READ(state);
		sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n State1: %x\n State2: %x\n Reversible: %s \n End_node \n", seq_id, g_curr_file, flags, LOWORD(state), HIWORD(state), str);
		Create_entity(name, data, 63, seq_id, pos, Colors::Cyan, nullptr, nullptr, 0, nullptr);
	}
}

void load_enti_64(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x38);
	DWORD flags;
	READ(flags);
	XMFLOAT3 pos;
	READ(pos);
	SFPC(0x4);
	DWORD* seq_ids = (DWORD*)malloc(8);
	READP(seq_ids, 8);
	SFPC(0x30);
	XMFLOAT3 pos2;
	READ(pos2);
	
	g_lines.push_back({ pos, pos2, XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f) }); // blue line

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Entity path_controller (64)\n");

	bool reversible = flags & 0x80;
	const char* str = (reversible) ? "true" : "false";

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n Reversible: %s \n", seq_id, g_curr_file, flags, str);
	Create_entity(name, data, 64, seq_id, pos, Colors::Magenta, nullptr, nullptr, 2, seq_ids);

}

void load_enti_68(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x2C);
	int num1;
	READ(num1);
	SFPC(num1 * 4 + 20);
	DWORD* seq_ids = (DWORD*)malloc(4);
	READP(seq_ids, 4);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);
	sprintf(name, "Entity Teleport property (68)\n");
	Create_global_entity(name, data, 68, seq_id, Colors::Cyan ,1, seq_ids);
}

void load_enti_69(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x2C);
	int num1;
	READ(num1);
	SFPC(num1 * 4);
	XMFLOAT3 pos;
	READ(pos);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);
	sprintf(name, "Entity Teleport locations (69)\n");
	Create_entity(name, data, 69, seq_id, pos, Colors::Purple);
}

void load_enti_70(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x38);
	DWORD founded = 0;
	DWORD* seq_ids = nullptr;

	WORD* msgids = (WORD*)malloc(5 * 2);
	READP(msgids, 5 * 2);
	seq_ids = search_in_SMSG(msgids[2], 0, &founded);

	SFPS(0x58);
	XMFLOAT3 pos;
	READ(pos);
	char* str1 = (char*)malloc(256);
	ZeroMemory(str1, 256);
	SFPS(0x88);
	read_padded_str(f, str1);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Entity debug note (70)\n");
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n note: %s \n", seq_id, g_curr_file, str1);

	Create_entity(name, data, 70, seq_id, pos, Colors::Magenta, nullptr, nullptr, founded, seq_ids);
}

void load_enti_71(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x34);
	DWORD flags;
	READ(flags);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	READ(pos);
	READ(rot);
	SFPS(0x180);
	char* str1 = (char*)malloc(128);
	ZeroMemory(str1, 128);
	char* str2 = (char*)malloc(128);
	ZeroMemory(str2, 128);
	char* str3 = (char*)malloc(128);
	ZeroMemory(str3, 128);
	read_padded_str(f, str1);
	read_padded_str(f, str2);
	read_padded_str(f, str3);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Entity special fx (71)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n 1: %s \n 2: %s \n 3: %s \n", seq_id, g_curr_file, flags, str1, str2, str3);
	XMFLOAT3 dir = { pos.x + rot.x, pos.y + rot.y, pos.z + rot.z };
	g_lines.push_back({ pos, dir, XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f) }); // blue line

	Create_entity(name, data, 71, seq_id, pos, Colors::Yellow);
}

void load_enti_72(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x38);
	char* BE_data = BE_messages(f);
	XMFLOAT3 pos;
	READ(pos);
	
	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Entity waypoint (72)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);
	Create_entity(name, data, 72, seq_id, pos, Colors::Cyan);
}

void load_enti_74(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x2C);
	DWORD num = 0;
	DWORD founded = 0;
	DWORD* seq_ids = nullptr;
	READ(num);
	if (num != 0)
	{
		e_act* acts = (e_act*)malloc(num * 4);
		READP(acts, num * 4);
		seq_ids = search_in_SMSG(acts[0].msgid, 0, &founded);
	}
	
	char* str1 = (char*)malloc(256);
	ZeroMemory(str1, 256);
	SFPC(8);
	read_padded_str(f, str1);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "console var (74)\n");
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n command: %s \n", seq_id, g_curr_file, str1);

	Create_global_entity( name, data, 74, seq_id, Colors::LightGreen, founded, seq_ids);
}

void load_enti_75(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x30);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	READ(pos);
	READ(rot);
	SFPS(0x98);
	char* str1 = (char*)malloc(128);
	ZeroMemory(str1, 128);
	char* str2 = (char*)malloc(128);
	ZeroMemory(str2, 128);
	read_padded_str(f, str1);
	SFPC(0xC);
	read_padded_str(f, str2);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Entity special fx (75)\n");
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n 1: %s \n 2: %s \n", seq_id, g_curr_file, str1, str2);
	XMFLOAT3 dir = { pos.x + rot.x, pos.y + rot.y, pos.z + rot.z };
	g_lines.push_back({ pos, dir, XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f) }); // blue line

	Create_entity(name, data, 75, seq_id, pos, Colors::Yellow);
}

void load_enti_77(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x38);
	XMFLOAT3 pos;
	READ(pos);
	SFPS(0x74);
	asura_bbox bb;
	XMFLOAT4 rot;
	READ(bb);
	READ(rot);
	g_bboxes.push_back({ { bb.x1,bb.y1, bb.z1 },{ bb.x2,bb.y2, bb.z2 }, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), rot });

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Entity special fx (77)\n");
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);
	Create_entity(name, data, 77, seq_id, pos, Colors::Red);
}

void load_enti_78(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;

	//SFPS(0x30); // BE flags (need to create BE parsing function)
	//DWORD BE_flags;
	//READ(BE_flags);
	//const char* str = ((BE_flags & 1)) ? "enabled" : "disabled";
	SFPS(0x38);
	DWORD trigger_enabled;
	DWORD flags;
	READ(trigger_enabled);
	READ(flags);
	const char* str = ((trigger_enabled & 1)) ? "enabled" : "disabled";
	SFPS(0x40);

	DWORD founded = 0;
	DWORD* seq_ids = nullptr;

	WORD* msgids = (WORD*)malloc(5 * 2);
	READP(msgids, 5 * 2);
	seq_ids = search_in_SMSG(msgids[2], 0, &founded);

	SFPS(0x7D);
	DWORD num1;
	READ(num1);
	for (int i = 0; i < num1; i++)
	{
		DWORD type;
		READ(type);
		if (type == 1)
		{
			SFPC(4);
			asura_bbox bb;
			XMFLOAT4 rot;
			READ(bb);
			READ(rot);
			XMFLOAT3 pos = { (bb.x1 + bb.x2) / 2,(bb.y1 + bb.y2) / 2, (bb.z1 + bb.z2) / 2 };
			g_bboxes.push_back({ {pos.x + 0.2f,pos.y + 0.2f, pos.z + 0.2f },{ pos.x - 0.2f,pos.y - 0.2f, pos.z - 0.2f }, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), rot });
			bbox* bbox_p = bbox_from_asura_bb(&bb, rot, Colors::Black);
			SFPC(16);

			char* name = nullptr; char* data = nullptr;
			Alloc_name_and_data_str(&name, &data);
			sprintf(name, "Trigger (78)\n");
			sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Trigger %s \n flags: %x\n Type 1 \n  %s \n", seq_id, g_curr_file, str, flags, dump_msg(msgids[2]));

			Create_entity(name, data, 78, seq_id, pos, Colors::Red, nullptr, bbox_p, founded, seq_ids);
			return;
		}
		if (type == 2)
		{
			SFPC(4);
			XMFLOAT3 pos;
			float value;
			XMFLOAT4 rot;
			READ(pos);
			READ(value);
			READ(rot);
			g_bboxes.push_back({ { pos.x + 0.2f,pos.y + 0.2f, pos.z + 0.2f },{ pos.x - 0.2f,pos.y - 0.2f, pos.z - 0.2f }, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), rot });
			bbox* bbox_p = bbox_from_point(pos, value, Colors::Black);

			char* name = nullptr; char* data = nullptr;
			Alloc_name_and_data_str(&name, &data);
			sprintf(name, "Trigger (78)\n");
			sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Trigger %s \n flags: %x\n Type 2 \n radius: %f \n %s \n", seq_id, g_curr_file, str, flags, value, dump_msg(msgids[2]));

			Create_entity(name, data, 78, seq_id, pos, Colors::Red, nullptr, bbox_p, founded, seq_ids);
			return;
		}
		if (type == 3) //unk yet
		{
			SFPC(4);
			XMFLOAT3 pos;
			READ(pos);
			SFPC(24);

			char* name = nullptr; char* data = nullptr;
			Alloc_name_and_data_str(&name, &data);
			sprintf(name, "Trigger (78)\n");
			sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Trigger %s \n flags: %x\n Type 3 \n %s \n", seq_id, g_curr_file, str, flags, dump_msg(msgids[2]));

			Create_entity(name, data, 78, seq_id, pos, Colors::Red, nullptr, nullptr, founded, seq_ids);
			return;
		}
	}
}

//Light obj
void load_enti_83(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x30);
	DWORD num1;
	READ(num1);
	SFPC(num1 * 4);
	DWORD hash;  // this is hash from CONA chunk
	READ(hash);

	XMFLOAT3 pos;
	XMFLOAT4 rot;
	READ(pos);
	READ(rot);



	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Entity Light obj (83)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);

	bbox* bbox_p = bbox_from_point(pos, 0.1f, Colors::Yellow, rot);
	// no prop for now
	Create_entity(name, data, 83, seq_id, pos, Colors::Yellow, nullptr, bbox_p);
}

void load_enti_85(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x2C);
	char* BE_data = BE_messages(f);
	DWORD DLLN_counts; //dialogu lines count
	SFPC(0x22);
	READ(DLLN_counts);
	SFPC(0x13 * DLLN_counts); // dont interessting right now
	DWORD amount;
	READ(amount);
	DWORD* seq_ids = (DWORD*)malloc(amount * 4);
	for (int i = 0; i < amount; i++)
	{
		SFPC(4);//skip DLLN hash
		READ(seq_ids[i]);
	}

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Dialogue sequence (85)\n");
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n BE_data: %s \n Lines: %d \n", seq_id, g_curr_file, BE_data, amount);
	Create_global_entity(name, data, 85, seq_id, Colors::Cyan, amount, seq_ids);
}

void load_enti_92(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x30);
	DWORD hash;
	READ(hash);
	SFPC(8);
	char* BE_data = BE_messages(f);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	READ(pos);
	READ(rot);
	SFPC(25);
	XMFLOAT2 hp;
	READ(hp);
	SFPC(116);
	DWORD* seq_ids = (DWORD*)malloc(8);
	READP(seq_ids, 8);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "alien (%d)\n", seq_id);

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Actor: %s\n Curr_health: %f \n Max_health: %f \n %s \n", seq_id, g_curr_file, Actor_name_from_hash(hash), hp.x, hp.y, BE_data);
	bbox* bbox_p = new bbox;
	bbox_p->p1 = { pos.x + 0.4f, pos.y - 1.0f, pos.z + 0.4f };
	bbox_p->p2 = { pos.x - 0.4f, pos.y - 0.0f, pos.z - 0.4f };
	bbox_p->rot = rot;
	bbox_p->Color = { 1.0f, 0.0f, 1.0f, 1.0f };

	entity_prop* prop = new entity_prop;
	ZeroMemory(prop, sizeof(entity_prop));
	prop->visible = 1;
	prop->model_hash = hash_from_str(0, (char*)"Alien_Warrior");
	prop->pos = pos; 
	prop->angle = rot;

	Create_entity(name, data, 92, seq_id, pos, Colors::Yellow, prop, bbox_p, 2, seq_ids);
}

void load_enti_94(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	SFPS(0x30);
	DWORD hash;
	READ(hash);
	SFPC(8);
	char* BE_data = BE_messages(f);
	READ(pos);
	READ(rot);
	SFPC(25);
	XMFLOAT2 hp;
	READ(hp);
	SFPC(104);
	DWORD* seq_ids = (DWORD*)malloc(8);
	READP(seq_ids, 8);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Facehugger (%d)\n", seq_id);

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Actor: %s\n Curr_health: %f \n Max_health: %f \n %s \n", seq_id, g_curr_file, Actor_name_from_hash(hash), hp.x, hp.y, BE_data);
	bbox* bbox_p = new bbox;
	bbox_p->p1 = { pos.x + 0.3f, pos.y - 0.2f, pos.z + 0.3f };
	bbox_p->p2 = { pos.x - 0.3f, pos.y - 0.0f, pos.z - 0.3f };
	bbox_p->rot = rot;
	bbox_p->Color = { 1.0f, 0.0f, 1.0f, 1.0f };

	entity_prop* prop = new entity_prop;
	ZeroMemory(prop, sizeof(entity_prop));
	prop->visible = 1;
	prop->model_hash = hash_from_str(0, (char*)"Facehugger");
	prop->pos = pos; 
	prop->angle = rot;

	Create_entity(name, data, 94, seq_id, pos, Colors::Yellow, prop, bbox_p, 2, seq_ids);
}

void load_enti_95(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x30);
	DWORD hash;
	READ(hash);
	SFPC(8);
	char* BE_data = BE_messages(f);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	READ(pos);
	READ(rot);
	SFPC(25);
	XMFLOAT2 hp;
	READ(hp);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "alien_queen (%d)\n", seq_id);

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Actor: %s\n Curr_health: %f \n Max_health: %f \n %s \n", seq_id, g_curr_file, Actor_name_from_hash(hash), hp.x, hp.y, BE_data);
	bbox* bbox_p = new bbox;
	bbox_p->p1 = { pos.x + 1.0f, pos.y - 3.0f, pos.z + 1.0f };
	bbox_p->p2 = { pos.x - 1.0f, pos.y - 0.0f, pos.z - 1.0f };
	bbox_p->rot = rot;
	bbox_p->Color = { 1.0f, 0.0f, 1.0f, 1.0f };

	entity_prop* prop = new entity_prop;
	ZeroMemory(prop, sizeof(entity_prop));
	prop->visible = 1;
	prop->model_hash = hash_from_str(0, (char*)"Hag_Queen");
	prop->pos = pos;
	prop->angle = rot;

	Create_entity(name, data, 95, seq_id, pos, Colors::Yellow, prop, bbox_p);
}

void load_enti_97(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x30);

	DWORD actor_name_hash;
	READ(actor_name_hash);
	SFPC(8);
	char* BE_data = BE_messages(f);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	READ(pos);
	READ(rot);
	SFPC(25);
	XMFLOAT2 some_float_data;
	READ(some_float_data);
	SFPC(112);
	DWORD* seq_ids = (DWORD*)malloc(8);
	READP(seq_ids, 8);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Predator (%d)\n", seq_id);

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Actor: %s \n Curr_health: %f \n Max_health: %f \n %s \n", seq_id, g_curr_file, \
		Actor_name_from_hash(actor_name_hash), some_float_data.x, some_float_data.y, BE_data);
	//XMFLOAT3 dir = { pos.x + rot.x, pos.y + rot.y, pos.z + rot.z };
	//g_lines.push_back({ pos, dir, XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f) }); // blue line
	bbox* bbox_p = new bbox;
	bbox_p->p1 = { pos.x + 0.3f, pos.y - 1.2f, pos.z + 0.3f };
	bbox_p->p2 = { pos.x - 0.3f, pos.y + 0.0f, pos.z - 0.3f };
	bbox_p->rot = rot;
	bbox_p->Color = { 1.0f, 0.0f, 1.0f, 1.0f };

	entity_prop* prop = new entity_prop;
	ZeroMemory(prop, sizeof(entity_prop));
	prop->visible = 1;
	prop->model_hash = hash_from_str(0, (char*)"Predator");
	prop->pos = pos;
	prop->angle = rot;

	Create_entity(name, data, 97, seq_id, pos, Colors::Yellow, prop, bbox_p, 2, seq_ids);
}

void load_enti_100(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x3C);
	DWORD flags;
	READ(flags);
	XMFLOAT3 pos;
	READ(pos);
	SFPC(0x4);
	DWORD* seq_ids = (DWORD*)malloc(8);
	READP(seq_ids, 8);
	SFPC(0x30);
	XMFLOAT3 pos2;
	READ(pos2);

	g_lines.push_back({ pos, pos2, XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f) }); // blue line

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Entity waypoint_controller (100)\n");

	bool reversible = flags & 0x80;
	const char* str = (reversible) ? "true" : "false";

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n Reversible: %s \n", seq_id, g_curr_file, flags, str);
	Create_entity(name, data, 64, seq_id, pos, Colors::Cyan, nullptr, nullptr, 2, seq_ids);
}

void load_enti_114(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x38);
	DWORD num;
	READ(num);
	SFPC(num * 4);
	DWORD flags;
	XMFLOAT3 pos;
	READ(flags);
	READ(pos);
	DWORD* seq_ids = (DWORD*)malloc(4);
	READP(seq_ids, 4);
	if (*seq_ids == -1)
		seq_ids = nullptr;

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Entity waypoint (114)\n");

	bool reversible = flags & 0x80;
	bool start_node = flags & 0x10;
	const char* str = (reversible) ? "true" : "false";
	const char* str2 = (start_node) ? "true" : "false";

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n Reversible: %s \nStart_node: %s \n", seq_id, g_curr_file, flags, str, str2);
	Create_entity(name, data, 114, seq_id, pos, Colors::Cyan, nullptr, nullptr, 1, seq_ids);
}

void load_enti_115(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x5C);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	char* BE_data = BE_messages(f);
	READ(pos);
	READ(rot);
	SFPC(25);
	XMFLOAT2 hp;
	READ(hp);
	DWORD hash;
	SFPC(8);
	READ(hash);
	
	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "115 (%d)\n", seq_id);

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Curr_health: %f \n Max_health: %f \n %s \n", seq_id, g_curr_file, hp.x, hp.y, BE_data);
	entity_prop* prop = new entity_prop;
	ZeroMemory(prop, sizeof(entity_prop));
	prop->visible = 1;
	prop->model_hash = hash;
	prop->pos = pos;
	prop->angle = rot;

	Create_entity(name, data, 115, seq_id, pos, Colors::Purple, prop, nullptr);
}

void load_enti_122(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x34);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	char* BE_data = BE_messages(f);
	READ(pos);
	READ(rot);
	SFPC(25);
	XMFLOAT2 hp;
	READ(hp);
	DWORD hash;
	SFPC(8);
	READ(hash);
	SFPC(0x43);
	asura_bbox bb;
	XMFLOAT4 rot2;
	READ(bb);
	READ(rot2);
	bbox* bbox_p = bbox_from_asura_bb(&bb, rot, Colors::Purple); // Activation zone

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "retinal_scanner (%d)\n", seq_id);

	hash_tree_node2* node = search_by_hash2(model_hashs, hash);
	if (!node)
		node = g_default_model; // Default mdl

	model_info* mi = (model_info*)node->ptr;

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Curr_health: %f \n Max_health: %f \n Model_name: %s \n %s \n", seq_id, g_curr_file, hp.x, hp.y, mi->mdl_name, BE_data);
	entity_prop* prop = new entity_prop;
	ZeroMemory(prop, sizeof(entity_prop));
	prop->visible = 1;
	prop->model_hash = hash;
	prop->pos = pos;
	prop->angle = rot;

	Create_entity(name, data, 122, seq_id, pos, Colors::Purple, prop, bbox_p);
}

void load_enti_124(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x3C);
	char* BE_data = BE_messages(f);

	DWORD flags;
	READ(flags);
	XMFLOAT3 pos;
	READ(pos);
	SFPC(0x52);
	DWORD* seq_ids = (DWORD*)malloc(4);
	READP(seq_ids, 4);
	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n %s \n", seq_id, g_curr_file, flags, BE_data);
	sprintf(name, "124 (%d)\n", seq_id);

	Create_entity(name, data, 124, seq_id, pos, Colors::Cyan, nullptr, nullptr, 1, seq_ids);
}

void load_enti_126(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x2C);
	DWORD num;
	READ(num);
	SFPC(num * 4);
	XMFLOAT3 pos;
	READ(pos);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Checkpoint (126)\n");
	sprintf(data, "SEQ_ID: %d \n FILE: %ws\n", seq_id, g_curr_file);

	Create_entity(name, data, 126, seq_id, pos, Colors::Purple);
}

void load_enti_130(HANDLE f, DWORD seq_id)
{
	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "GameScores? (130)\n");
	sprintf(data, "SEQ_ID: %d \n FILE: %ws\n", seq_id, g_curr_file);
	Create_global_entity(name, data, 130, seq_id);
}

void load_enti_132(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x2C);
	XMFLOAT3 pos;
	DWORD num1;
	READ(num1);
	SFPC(num1 * 4);
	READ(pos);
	SFPC(24);

	asura_bbox ar;
	XMFLOAT4 rot;
	READ(ar);
	READ(rot);

	DWORD num2;
	READ(num2);
	DWORD* seq_ids = (DWORD*)malloc(num2 * 4);
	READP(seq_ids, num2 * 4);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Entity ??? (132)\n");
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);

	bbox* bbox_p = bbox_from_asura_bb(&ar, rot, Colors::Black);
	
	Create_entity(name, data, 132, seq_id, pos, Colors::White, nullptr, bbox_p, num2, seq_ids);
}

void load_enti_135(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x34);
	DWORD actor_name_hash;
	READ(actor_name_hash);

	SFPS(0x40);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	DWORD num1;
	READ(num1);
	SFPC(num1 * 4);
	READ(pos);
	READ(rot);
	SFPC(25);
	XMFLOAT2 some_float_data;
	READ(some_float_data);
	SFPC(0x24);
	DWORD type;
	READ(type);
	int skip_bytes = type == 1 ? 0x23 : 0xD8;
	SFPC(skip_bytes);
	DWORD* seq_ids = (DWORD*)malloc(8);
	READP(seq_ids, 8);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Combat_droid (%d)\n", seq_id);

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Actor: %s \n Curr_health: %f \n Max_health: %f \n", seq_id, g_curr_file, \
		Actor_name_from_hash(actor_name_hash), some_float_data.x, some_float_data.y);
	bbox* bbox_p = new bbox;
	bbox_p->p1 = { pos.x + 0.3f, pos.y - 1.2f, pos.z + 0.3f };
	bbox_p->p2 = { pos.x - 0.3f, pos.y + 0.0f, pos.z - 0.3f };
	bbox_p->rot = rot;
	bbox_p->Color = { 1.0f, 0.0f, 1.0f, 1.0f };

	entity_prop* prop = new entity_prop;
	ZeroMemory(prop, sizeof(entity_prop));
	prop->visible = 1;
	prop->model_hash = hash_from_str(0, (char*)"combat_android");
	prop->pos = pos;
	prop->angle = rot;

	Create_entity(name, data, 135, seq_id, pos, Colors::Yellow, prop, bbox_p, 2, seq_ids);
}

void load_enti_133(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x30);
	DWORD count = 0;
	READ(count);

	DWORD found = 0;
	DWORD totaly_found = 0;
	DWORD current_pos = 0;
	DWORD* seq_ids = nullptr;

	for (int i = 0; i < count; i++)
	{
		WORD msgid;
		SFPC(2);
		READ(msgid);
		found = 0;
		search_in_SMSG(msgid, 0, &found);
		totaly_found += found;
	}
	seq_ids = (DWORD*)malloc(totaly_found * 4);
	ZeroMemory(seq_ids, totaly_found * 4);
	SFPS(0x34);

	for (int i = 0; i < count; i++)
	{
		WORD msgid;
		SFPC(2);
		READ(msgid);
		found = 0;
		DWORD* seq_ids2 = search_in_SMSG(msgid, 0, &found);
		if (seq_ids2 == nullptr)
			continue;
		memcpy(seq_ids + current_pos, seq_ids2, found * 4);
		current_pos += found;
	}
	found = totaly_found;

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "some event (133)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);

	Create_global_entity(name, data, 133, seq_id, Colors::Cyan, found, seq_ids);
}

void load_enti_136(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x34);
	DWORD flags;
	READ(flags);
	XMFLOAT3 pos;
	READ(pos);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Entity obstacle (136)\n");

	DWORD state;
	bool reversible = flags & 0x80;
	bool start_node = flags & 0x10;
	const char* str = (reversible) ? "true" : "false";


	if (start_node)
	{
		DWORD* seq_ids = (DWORD*)malloc(4);
		READP(seq_ids, 4);
		SFPC(4);
		READ(state);
		sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n State1: %x\n State2: %x\n Reversible: %s \nStart_node \n", seq_id, g_curr_file, flags, LOWORD(state), HIWORD(state), str);
		Create_entity(name, data, 136, seq_id, pos, Colors::Cyan, nullptr, nullptr, 0, nullptr);
	}
	else
	{
		SFPC(4);
		READ(state);
		sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n State1: %x\n State2: %x\n Reversible: %s \n End_node \n", seq_id, g_curr_file, flags, LOWORD(state), HIWORD(state), str);
		Create_entity(name, data, 136, seq_id, pos, Colors::Cyan, nullptr, nullptr, 0, nullptr);
	}
}

void load_enti_138(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x38);
	DWORD num;
	READ(num);
	SFPC(num * 4);
	DWORD num1;
	XMFLOAT3 pos;
	READ(num1);
	READ(pos);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Predator spots? (138)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n flags: %x \n", seq_id, g_curr_file, num1);

	Create_entity(name, data, 138, seq_id, pos, Colors::Cyan);
}

void load_enti_140(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x40);
	asura_bbox ar;
	XMFLOAT4 rot;
	READ(ar);
	READ(rot);
	SFPS(0x7C);
	DWORD num;
	READ(num);
	SFPC(num * 4);
	DWORD num1;
	XMFLOAT3 pos;
	READ(num1);
	READ(pos);
	DWORD num2;
	DWORD num3;
	SFPC(20);
	READ(num2);
	DWORD* seq_ids = (DWORD*)malloc(num2 * 4);
	READP(seq_ids, num2 * 4);
	READ(num3);
	DWORD* seq_ids2 = (DWORD*)malloc((num3 + num2) * 4);
	memcpy(seq_ids2, seq_ids, num2 * 4);
	READP(seq_ids2 + num2, num3 * 4);

	bbox* bbox_p = bbox_from_asura_bb(&ar, rot, Colors::Black);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "patrol area ? (140)\n");

	g_bboxes.push_back({ { pos.x + 0.2f, pos.y + 0.2f, pos.z + 0.2f }, { pos.x - 0.2f, pos.y - 0.2f, pos.z - 0.2f }, { 1.0f, 0.0f, 1.0f, 1.0f } });

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n flags: %x \n", seq_id, g_curr_file, num1);

	Create_entity(name, data, 140, seq_id, pos, Colors::Magenta, nullptr, bbox_p, num2 + num3, seq_ids2);
}

void load_enti_141(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x34);
	DWORD hash;
	READ(hash);
	SFPC(8);
	char* BE_data = BE_messages(f);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	READ(pos);
	READ(rot);
	SFPC(25);
	XMFLOAT2 hp;
	READ(hp);
	SFPC(0x24);
	DWORD type;
	READ(type);
	int skip_bytes = type == 1 ? 0x23 : 0x115;
	SFPC(skip_bytes);
	DWORD* seq_ids = (DWORD*)malloc(8);
	READP(seq_ids, 8);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Colonist (141)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Actor: %s\n Curr_health: %f \n Max_health: %f \n %s \n", seq_id, g_curr_file, Actor_name_from_hash(hash), hp.x, hp.y, BE_data);
	//XMFLOAT3 dir = { pos.x + rot.x, pos.y + rot.y, pos.z + rot.z };
	//g_lines.push_back({ pos, dir, XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f) }); // blue line
	bbox* bbox_p = new bbox;
	bbox_p->p1 = { pos.x + 0.3f, pos.y - 1.2f, pos.z + 0.3f };
	bbox_p->p2 = { pos.x - 0.3f, pos.y + 0.0f, pos.z - 0.3f };
	bbox_p->rot = rot;
	bbox_p->Color = { 1.0f, 0.0f, 1.0f, 1.0f };

	entity_prop* prop = new entity_prop;
	ZeroMemory(prop, sizeof(entity_prop));
	prop->visible = 1;
	prop->model_hash = hash_from_str(0, (char*)"Colonist");
	prop->pos = pos;
	prop->angle = rot;

	Create_entity(name, data, 141, seq_id, pos, Colors::Yellow, prop, bbox_p, 2, seq_ids);
}

void load_enti_142(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x34);
	DWORD hash;
	READ(hash);
	SFPC(8);
	char* BE_data = BE_messages(f);

	XMFLOAT3 pos;
	XMFLOAT4 rot;
	READ(pos);
	READ(rot);
	SFPC(25);
	XMFLOAT2 hp;
	READ(hp);
	SFPC(0x24);
	DWORD type;
	READ(type);
	int skip_bytes = type == 1 ? 0x23 : 0xD8;
	SFPC(skip_bytes);
	DWORD* seq_ids = (DWORD*)malloc(8);
	READP(seq_ids, 8);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Marine (%d)\n", seq_id);

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Actor: %s\n Curr_health: %f \n Max_health: %f \n %s \n", seq_id, g_curr_file, Actor_name_from_hash(hash), hp.x, hp.y, BE_data);
	//XMFLOAT3 dir = { pos.x + rot.x, pos.y + rot.y, pos.z + rot.z };
	//g_lines.push_back({ pos, dir, XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f) }); // blue line
	bbox* bbox_p = new bbox;
	bbox_p->p1 = { pos.x + 0.3f, pos.y - 1.2f, pos.z + 0.3f };
	bbox_p->p2 = { pos.x - 0.3f, pos.y + 0.0f, pos.z - 0.3f };
	bbox_p->rot = rot;
	bbox_p->Color = { 1.0f, 0.0f, 1.0f, 1.0f };

	entity_prop* prop = new entity_prop;
	ZeroMemory(prop, sizeof(entity_prop));
	prop->visible = 1;
	prop->model_hash = hash_from_str(0, (char*)"MarineBody01");
	prop->pos = pos;
	prop->angle = rot;

	Create_entity(name, data, 142, seq_id, pos, Colors::Yellow, prop, bbox_p, 2, seq_ids);
}

void load_enti_144(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x3C);
	DWORD flags;
	READ(flags);
	XMFLOAT3 pos;
	READ(pos);
	

	
	DWORD state;
	SFPC(4);
	READ(state);
	XMFLOAT3 angles;
	READ(angles);
	DWORD check;
	READ(check);
	DWORD type = check & 0xff; // first byte

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n angles?: %f %f %f", seq_id, g_curr_file, flags, angles.x, angles.y, angles.z);
	sprintf(name, "Entity obstacle (144) type %d\n", type);

	DWORD count;
	DWORD* seq_ids;
	XMFLOAT3 pos2;

	switch (type)
	{
	case 1:
		READ(count);
		seq_ids = (DWORD*)malloc(count * 4);
		READP(seq_ids, count * 4);
		Create_entity(name, data, 144, seq_id, pos, Colors::Cyan, nullptr, nullptr, count, seq_ids);
		break;
	case 2:
	case 3:
		SFPC(4);
		READ(pos2);
		g_lines.push_back({ pos, pos2, XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f) });
		SFPC(12);
		READ(count);
		seq_ids = (DWORD*)malloc(count * 4);
		READP(seq_ids, count * 4);
		Create_entity(name, data, 144, seq_id, pos, Colors::Cyan, nullptr, nullptr, count, seq_ids);
		break;
	case 4:
		SFPC(4);
		READ(pos2);
		g_lines.push_back({ pos, pos2, XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f) });
		SFPC(12);
		XMFLOAT3 pos3;
		READ(pos3);
		g_lines.push_back({ pos, pos3, XMFLOAT4(0.2f, 0.7f, 0.7f, 1.0f) });
		SFPC(12);
		READ(count);
		seq_ids = (DWORD*)malloc(count * 4);
		READP(seq_ids, count * 4);
		Create_entity(name, data, 144, seq_id, pos, Colors::Cyan, nullptr, nullptr, count, seq_ids);
	}
}

void load_enti_145(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x30);
	DWORD num;
	READ(num);
	SFPC(num * 4);

	DWORD flags;
	READ(flags);
	XMFLOAT3 pos;
	READ(pos);
	
	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n f", seq_id, g_curr_file, flags);
	sprintf(name, "some waypoint? (145)\n");

	Create_entity(name, data, 145, seq_id, pos, Colors::Cyan);
}

void load_enti_146(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x2C);
	DWORD num;
	READ(num);
	SFPC(num * 4);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	READ(pos);
	READ(rot);
	SFPC(10);
	DWORD num1;
	READ(num1);
	for (int i = 0; i < num1; i++)
	{
		XMFLOAT3 pos2;
		READ(pos2);
		SFPC(16); // rotator
		g_lines.push_back({ pos, pos2, {0.0f, 0.0f, 1.0f, 1.0f} });
	}

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "alien hidding spot (146)\n");
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);

	Create_entity(name, data, 146, seq_id, pos, Colors::Magenta);
}

void load_enti_147(HANDLE f, DWORD seq_id)
{
	dbgprint("ENTI", "Trying load 147 spawn_property (%ws)\n", g_curr_file);
	DWORD a = 0;
	SFPS(0x2C);
	char* BE_dump = (char*)BE_messages(f);
	int d1;
	int d2;
	float f1;
	float f2;
	DWORD seq_id1;
	DWORD actor_count;
	int num;
	READ(d1);
	READ(d2);
	READ(f1);
	READ(f2);
	READ(seq_id1);
	READ(actor_count);

	struct actor_data
	{
		DWORD actor_name_hash;
		DWORD actor_init_hash;
		float prob; // maybe probability to spawn
	};

	actor_data* actors = (actor_data*)malloc(actor_count * sizeof(actor_data));

	READP(actors, actor_count * sizeof(actor_data));

	char actors_str[1024] = { 0 };
	char tmp_str[20] = { 0 };


	for (int i = 0; i < actor_count; i++)
	{
		strcat_s(actors_str, sizeof(actors_str), (const char*)Actor_name_from_hash(actors[i].actor_name_hash));
		sprintf(tmp_str, " (prob: %f)", actors[i].prob);
		strcat_s(actors_str, sizeof(actors_str), tmp_str);
		strcat_s(actors_str, sizeof(actors_str), "\n");
	}

	SFPC(8);
	READ(num);
	DWORD* seq_ids = (DWORD*)malloc((1 + num) * 4);
	seq_ids[0] = seq_id1;
	for (int i = 0; i < num; i++)
	{
		READP(seq_ids + 1 + i, 4);
		SFPC(4);
	}

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Actors: %s \n BE_data: %s \n d1: %d \n d2: %d \n f1: %f \n f2: %f", seq_id, g_curr_file, actors_str, BE_dump, d1, d2, f1, f2);
	sprintf(name, "spawn_property? (147)\n");

	Create_global_entity(name, data, 147, seq_id, Colors::Magenta, num + 1, seq_ids);
}

void load_enti_148(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x34);
	DWORD num;
	READ(num);
	SFPC(num * 4);

	DWORD flags;
	READ(flags);
	XMFLOAT3 pos;
	READ(pos);
	SFPC(0x20);
	DWORD* seq_ids = (DWORD*)malloc(4);
	READP(seq_ids, 4);
	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n", seq_id, g_curr_file, flags);
	sprintf(name, "some waypoint? (148)\n");

	Create_entity(name, data, 145, seq_id, pos, Colors::Cyan, nullptr, nullptr, 1, seq_ids);
}

void load_enti_149(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x30);
	int num1;
	READ(num1);
	SFPC(num1 * 4 + 12);
	DWORD id[3];
	READP(id, 12);
	float f1;
	READ(f1);


	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n id1: %d \n id2: %d \n id3: %d \n sense_radius: %f \n", seq_id, g_curr_file, id[0], id[1], id[2], f1);
	sprintf(name, "Monster idle_state? (149)\n");
	Create_global_entity(name, data, 149, seq_id);
}

void load_enti_151(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x30);
	DWORD hash;
	READ(hash);
	//skip 'Taav' and flags
	SFPC(8);
	char* BE_data = BE_messages(f);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	READ(pos);
	READ(rot);
	SFPC(25);
	XMFLOAT2 hp;
	READ(hp);
	SFPC(0x24);
	DWORD type;
	READ(type);
	int skip_bytes = type == 8 ? 0x44 : 0x44;
	SFPC(skip_bytes);
	DWORD* seq_ids = (DWORD*)malloc(8);
	READP(seq_ids, 8);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Sentry Gun (151)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Curr_health: %f \n Max_health: %f \n Actor: %s \n %s \n", seq_id, g_curr_file, hp.x, hp.y, Actor_name_from_hash(hash), BE_data);
	//XMFLOAT3 dir = { pos.x + rot.x, pos.y + rot.y, pos.z + rot.z };
	//g_lines.push_back({ pos, dir, XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f) }); // blue line
	bbox* bbox_p = new bbox;
	bbox_p->p1 = { pos.x + 0.3f, pos.y - 1.2f, pos.z + 0.3f };
	bbox_p->p2 = { pos.x - 0.3f, pos.y + 0.0f, pos.z - 0.3f };
	bbox_p->rot = rot;
	bbox_p->Color = { 1.0f, 0.0f, 1.0f, 1.0f };

	entity_prop* prop = new entity_prop;
	ZeroMemory(prop, sizeof(entity_prop));
	prop->visible = 1;
	prop->model_hash = hash_from_str(0, (char*)"sentry_gun");
	prop->pos = pos;
	prop->angle = rot;

	Create_entity(name, data, 151, seq_id, pos, Colors::Yellow, prop, bbox_p, 2, seq_ids);
}

void load_enti_156(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x34);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	DWORD num1;
	READ(num1);
	SFPC(num1 * 4);
	READ(pos);
	READ(rot);
	SFPC(0x29);
	DWORD hash;
	READ(hash);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Collectable (156)\n");

	entity_prop* prop = new entity_prop;
	ZeroMemory(prop, sizeof(entity_prop));
	prop->visible = 1;
	prop->model_hash = hash;
	prop->pos = pos;
	prop->angle = rot;

	hash_tree_node2* node = search_by_hash2(model_hashs, prop->model_hash);
	if (!node)
		node = g_default_model; // Default mdl

	model_info* mi = (model_info*)node->ptr;

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Model_name: %s \n", seq_id, g_curr_file, mi->mdl_name);

	Create_entity(name, data, 156, seq_id, pos, Colors::Magenta, prop);
}

void load_enti_157(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x30);
	XMFLOAT3 pos;
	DWORD flags;
	DWORD num1;
	READ(num1);
	SFPC(num1 * 4);
	READ(flags);
	READ(pos);
	SFPC(12);
	DWORD* seq_ids = (DWORD*)malloc(4);
	READP(seq_ids, 4);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Entity Target point (157)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n", seq_id, g_curr_file, flags);

	Create_entity(name, data, 157, seq_id, pos, Colors::Magenta, nullptr, nullptr, 1, seq_ids);
}

void load_enti_158(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x4C);
	XMFLOAT3 pos;
	DWORD flags;
	DWORD num1;
	READ(num1);
	SFPC(num1 * 4);
	READ(flags);
	READ(pos);
	SFPC(12);
	DWORD* seq_ids = (DWORD*)malloc(4);
	READP(seq_ids, 4);
	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Entity Mission Target (158)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n", seq_id, g_curr_file, flags);

	Create_entity(name, data, 158, seq_id, pos, Colors::Purple, nullptr, nullptr, 1, seq_ids);
}

void load_enti_162(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x3C);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	DWORD num1;
	READ(num1);
	SFPC(num1 * 4);
	READ(pos);
	READ(rot);
	SFPC(25);
	XMFLOAT2 some_float_data;
	READ(some_float_data);
	DWORD hash;
	SFPC(8);
	READ(hash);
	SFPC(0xBF);
	DWORD* seq_ids = (DWORD*)malloc(4);
	READP(seq_ids, 4);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Entity battery_station (162)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Curr_health: %f \n Max_health: %f \n", seq_id, g_curr_file, some_float_data.x, some_float_data.y);
	entity_prop* prop = new entity_prop;
	ZeroMemory(prop, sizeof(entity_prop));
	prop->visible = 1;
	prop->model_hash = hash;
	prop->pos = pos;
	prop->angle = rot;

	Create_entity(name, data, 162, seq_id, pos, Colors::Purple, prop, nullptr, 1, seq_ids);
}

void load_enti_163(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x3C);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	DWORD num1;
	READ(num1);
	SFPC(num1 * 4);
	READ(pos);
	READ(rot);
	SFPC(25);
	XMFLOAT2 some_float_data;
	READ(some_float_data);
	DWORD hash;
	SFPC(8);
	READ(hash);
	SFPC(0xBF);
	DWORD* seq_ids = (DWORD*)malloc(4);
	READP(seq_ids, 4);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Entity ??? (163)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Curr_health: %f \n Max_health: %f \n", seq_id, g_curr_file, some_float_data.x, some_float_data.y);
	entity_prop* prop = new entity_prop;
	ZeroMemory(prop, sizeof(entity_prop));
	prop->visible = 1;
	prop->model_hash = hash;
	prop->pos = pos;
	prop->angle = rot;

	Create_entity(name, data, 163, seq_id, pos, Colors::Red, prop, nullptr, 1, seq_ids);
}

void load_enti_165(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x3C);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	char* BE_data = BE_messages(f);
	READ(pos);
	READ(rot);
	SFPC(25);
	XMFLOAT2 hp;
	READ(hp);
	DWORD hash;
	SFPC(8);
	READ(hash);
	SFPC(0x43);
	asura_bbox bb;
	XMFLOAT4 rot2;
	READ(bb);
	READ(rot2);
	bbox* bbox_p = bbox_from_asura_bb(&bb, rot, Colors::Purple); // Activation zone

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "retina_scanner? (%d)\n", seq_id);

	hash_tree_node2* node = search_by_hash2(model_hashs, hash);
	if (!node)
		node = g_default_model; // Default mdl

	model_info* mi = (model_info*)node->ptr;

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Curr_health: %f \n Max_health: %f \n Model_name: %s \n %s \n", seq_id, g_curr_file, hp.x, hp.y, mi->mdl_name, BE_data);
	entity_prop* prop = new entity_prop;
	ZeroMemory(prop, sizeof(entity_prop));
	prop->visible = 1;
	prop->model_hash = hash;
	prop->pos = pos;
	prop->angle = rot;

	Create_entity(name, data, 165, seq_id, pos, Colors::Purple, prop, bbox_p);
}


void load_enti_166(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x2C);
	XMFLOAT3 pos;
	DWORD num1;
	READ(num1);
	SFPC(num1 * 4);
	READ(pos);
	SFPC(8);
	DWORD num2;
	READ(num2);
	for (int i = 0; i < num2; i++)
	{
		SFPC(8);
		asura_bbox ar;
		XMFLOAT4 rot;
		READ(ar);
		READ(rot);
		//g_bboxes.push_back({ {ar.x1, ar.y1, ar.z1 }, {ar.x2, ar.y2, ar.z2 }, {1.0f, 0.0f, 0.0f, 1.0f}, rot }); make for futher less messy
	}

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Entity Zone_area (166)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);

	char* data2 = (char*)malloc(512);
	ZeroMemory(data2, 512);


	DWORD num3;
	READ(num3);
	DWORD* seq_ids = (DWORD*)malloc(num3 * 4);
	for (int i = 0; i < num3; i++)
	{
		DWORD id1;
		DWORD id2;
		XMFLOAT3 pos_;
		DWORD id3;
	
		READ(id1);
		READ(id2);
		READP(seq_ids + i, 4);
		READ(pos_);

		//g_bboxes.push_back({ {pos_.x + 0.2f, pos_.y + 0.2f, pos_.z + 0.2f },{ pos_.x - 0.2f, pos_.y - 0.2f, pos_.z - 0.2f },{ 1.0f, 0.0f, 0.0f, 1.0f }}); // translation point ?

		READ(id3);
		//sprintf(data2, "%d -> { %d, %d, %d } \n", i, id1, id2, id3);
		strcat(data, data2);
	}

	Create_entity(name, data, 166, seq_id, pos, Colors::Cyan, nullptr, nullptr, num3, seq_ids);
}

void load_enti_167(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x30);
	DWORD num;
	READ(num);
	SFPC(num * 4);

	DWORD flags;
	READ(flags);
	XMFLOAT3 pos;
	READ(pos);
	SFPC(0x4);
	DWORD* seq_ids = (DWORD*)malloc(4);
	READP(seq_ids, 4);
	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n", seq_id, g_curr_file, flags);
	sprintf(name, "167 (%d)\n", seq_id);

	Create_entity(name, data, 167, seq_id, pos, Colors::Cyan, nullptr, nullptr, 1, seq_ids);
}

void load_enti_169(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x2C);
	DWORD num;
	READ(num);
	SFPC(num * 4);
	DWORD* seq_ids = (DWORD*)malloc(4);
	READP(seq_ids, 4);
	XMFLOAT3 pos;
	READ(pos);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);
	sprintf(name, "specific point (169)\n");

	Create_entity(name, data, 169, seq_id, pos, Colors::Magenta, nullptr, nullptr, 1, seq_ids);
}

void load_enti_170(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x38);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	DWORD num1;
	READ(num1);
	SFPC(num1 * 4);
	READ(pos);
	SFPC(0x30);
	asura_bbox bb;
	READ(bb);
	READ(rot);

	g_bboxes.push_back({ { bb.x1, bb.y1, bb.z1 },{ bb.x2, bb.y2, bb.z2 },{ 1.0f, 0.0f, 0.0f, 1.0f }, rot }); 

	// There so many connections followed by this CODE: Short (id) byte (id2)

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Entity In door area (170)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);

	Create_entity(name, data, 170, seq_id, pos, Colors::Red);
}

void load_enti_173(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x38);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	char* BE_data = BE_messages(f);
	READ(pos);
	READ(rot);
	SFPC(25);
	XMFLOAT2 hp;
	READ(hp);
	DWORD hash;
	SFPC(8);
	READ(hash);
	SFPC(0x43);
	asura_bbox bb;
	XMFLOAT4 rot2;
	READ(bb);
	READ(rot2);
	bbox* bbox_p = bbox_from_asura_bb(&bb, rot, Colors::Purple); // Activation zone

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Computer (%d)\n", seq_id);

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Curr_health: %f \n Max_health: %f \n %s \n", seq_id, g_curr_file, hp.x, hp.y, BE_data);
	entity_prop* prop = new entity_prop;
	ZeroMemory(prop, sizeof(entity_prop));
	prop->visible = 1;
	prop->model_hash = hash;
	prop->pos = pos;
	prop->angle = rot;

	Create_entity(name, data, 173, seq_id, pos, Colors::Purple, prop, bbox_p);
}

void load_enti_176(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(0x30);
	DWORD hash;
	READ(hash);
	SFPC(8);
	char* BE_data = BE_messages(f);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	READ(pos);
	READ(rot);
	SFPC(25);
	XMFLOAT2 hp;
	READ(hp);

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(name, "Preatorian (%d)\n", seq_id);

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Actor: %s\n Curr_health: %f \n Max_health: %f \n %s \n", seq_id, g_curr_file, Actor_name_from_hash(hash), hp.x, hp.y, BE_data);
	bbox* bbox_p = new bbox;
	bbox_p->p1 = { pos.x + 0.6f, pos.y - 2.0f, pos.z + 0.6f };
	bbox_p->p2 = { pos.x - 0.6f, pos.y - 0.0f, pos.z - 0.6f };
	bbox_p->rot = rot;
	bbox_p->Color = { 1.0f, 0.0f, 1.0f, 1.0f };

	entity_prop* prop = new entity_prop;
	ZeroMemory(prop, sizeof(entity_prop));
	prop->visible = 1;
	prop->model_hash = 0xEE060EC5;
	prop->pos = pos;
	prop->angle = rot;

	Create_entity(name, data, 176, seq_id, pos, Colors::Yellow, prop, bbox_p);
}

void load_effect(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	//SetFilePointer(file, 0x34, NULL, FILE_BEGIN); // effect name HASH
	//SetFilePointer(file, 0x38, NULL, FILE_BEGIN);
	SFPS(0x38);
	XMFLOAT3 pos;
	XMFLOAT3 dir;
	//ReadFile(file, &pos, sizeof(XMFLOAT3), &bytes_readen, NULL);
	READ(pos);
	//SetFilePointer(file, 0x48, NULL, FILE_BEGIN);
	SFPS(0x48);
	//ReadFile(file, &dir, sizeof(XMFLOAT3), &bytes_readen, NULL);
	READ(dir); // i dont think its direction.
	g_bboxes.push_back({ XMFLOAT3(pos.x + 0.1f, pos.y + 0.1f, pos.z + 0.1f),XMFLOAT3(pos.x - 0.1f, pos.y - 0.1f, pos.z - 0.1f), XMFLOAT4(1.0f,1.0f,0.0f,1.0f) }); // small yellow bbox
	XMVECTOR norm_dir = XMLoadFloat3(&dir);
	XMVECTOR pos_v = XMLoadFloat3(&pos);
	norm_dir = XMVector3Normalize(norm_dir);
	
	XMStoreFloat3(&dir, pos_v + norm_dir);
	g_lines.push_back({ pos, dir, XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f)}); // blue line
}

int load_enti(const wchar_t* folder_path)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(folder_path, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		dbgprint("Entity", "FindFirstFile failed (%d) %ws\n", GetLastError(), folder_path);
		return 0;
	}

	do {
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;
		else
			load_entity(FindFileData.cFileName);
	} while (FindNextFile(hFind, &FindFileData) != 0);

	Perform_connections();
	Ent_to_bbox();
	Ent_make_dynamic_buffer();

	FindClose(hFind);
	return 1;
}

void load_smsg_file(wchar_t* file_name)
{
	//dbgprint("SMSG", "!!!!!!!!!!!!!!!!!!!!!!!!! MESSAGE LIST !!!!!!!!!!!!!!!!!!!!!!!!! \n");
	wchar_t file_path[MAX_PATH] = { 0 };
	wsprintf(file_path, L"%ws\\SMSG_chunk\\", g_path2);
	lstrcat(file_path, file_name);
	dbgprint("SMSG", "Trying load %ws \n", file_path);
	DWORD a = 0;
	HANDLE f = CreateFileW(file_path, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (GetLastError() != ERROR_SUCCESS)
	{
		dbgprint("SMSG", "Opening file Error: %d\n", GetLastError());
		return;
	}

	struct asura_header24 {
		DWORD magic;
		DWORD size;
		DWORD type1;
		DWORD type2;
		DWORD Count;
		DWORD upper;
	};
	asura_header24 header;
	READ(header);


	if (header.magic != (DWORD)'GSMS' || header.upper >= 2 || header.upper < 0 || header.type1 != 7)
	{
		dbgprint("SMSG", "Opening file Error: INCORRECT FILE HEADER\n");
		return;
	}

	if (header.upper)
	{
		SMSG_ptr2 = (void**)malloc(header.Count * 4);
		ZeroMemory(SMSG_ptr2, header.Count * 4);
		SMSG_count2 = header.Count;
	}
	else
	{
		SMSG_ptr1 = (void**)malloc(header.Count * 4);
		ZeroMemory(SMSG_ptr1, header.Count * 4);
		SMSG_count1 = header.Count;
	}

	for (int i = 0; i < header.Count; i++)
	{
		uint8_t b1;
		READ(b1);
		if (b1)
		{
			SMSG_entry entry;
			READ(entry);
			//dbgprint("SMSG", "entry: %d %d %d (%d) -> \n",entry.dw1, entry.type, entry.dw2, entry.Count);
			void* mem = malloc(sizeof(SMSG_subentry) * entry.Count + 4);
			*(DWORD*)mem = entry.Count;
			for (int j = 0; j < entry.Count; j++)
			{
				SMSG_subentry subentry;
				READ(subentry.w1);
				READ(subentry.w2);
				READ(subentry.seq_id);
				READ(subentry.f1);
				READ(subentry.f2);
				//dbgprint("SMSG", "%d : {%hu, %hu -> %d} \n", i, subentry.w1, subentry.w2, subentry.seq_id);
				if (entry.type >= 6)
					SFPC(4); //unk dword
				void* mem_data = nullptr;
				switch (subentry.w1)
				{
				case 0x7B:
					mem_data = malloc(84);
					READP(mem_data, 84);
					subentry.data = mem_data;
					//SFPC(84);
					break;
				case 0x7C:
				case 0x7D:
				case 0x7E:
				case 0x7F:
					mem_data = malloc(40);
					READP(mem_data, 40);
					subentry.data = mem_data; // 32 + 8 unk
					//SFPC(40);
					break;
				case 0x80:
					mem_data = malloc(80);
					READP(mem_data, 80);
					subentry.data = mem_data;
					//SFPC(80);
					break;
				default:
					WORD w2 = (subentry.w2 + 3) & 0xFFFFFFFC;
					if (w2)
					{
						mem_data = malloc(w2);
						READP(mem_data, w2);
						subentry.data = mem_data;
					}
					//SFPC(w2);
					break;
				}
				memcpy((DWORD*)mem + 1 + j*(sizeof(subentry)/4), &subentry, sizeof(subentry));
			}

			if (header.upper)
			{
				SMSG_ptr2[i] = mem;
			}
			else
			{
				SMSG_ptr1[i] = mem;
			}
		}
	}
	CloseHandle(f);
}

int load_smsg(const wchar_t* folder_path)
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
			load_smsg_file(FindFileData.cFileName);
	} while (FindNextFile(hFind, &FindFileData) != 0);

	FindClose(hFind);
	return 1;
}

int load_actor_names()
{
	FILE* f = fopen("ActorNames.txt", "rt");

	dbgprint("actor names", "Init hashs\n");

	if (f == NULL)
	{
		dbgprint("actor names", "Error opening names list !!!\n");
		return 0;
	}

	g_actornames_hashs = new hash_tree_node2;
	g_actornames_hashs->hash = 0;
	g_actornames_hashs->bigger = nullptr;
	g_actornames_hashs->smaller = nullptr;
	g_actornames_hashs->ptr = nullptr;

	char _name[512];

	while (!feof(f))
	{
		char* name = new char[512];
		ZeroMemory(name, 512);
		fgets(_name, 512, f);
		strcpy(name, _name);
		name[strlen(name) - 1] = '\0';

		if (strlen(name) == 0)
			break;

		DWORD hash = hash_from_str(0, name);

		if (search_by_hash2(g_actornames_hashs, hash))
		{
			delete[] name;
			continue;
		}

		hash_tree_node2* hn = new hash_tree_node2;
		hn->hash = hash;
		hn->ptr = name;
		add_hash2(g_actornames_hashs, hn);
	}

	g_actornames_loaded = true;

	fclose(f);
	return 1;
}



void load_point_entity(HANDLE file, DWORD entity_id, DWORD seq_id)
{


	switch (entity_id)
	{
	case 7:
		load_prop(file, seq_id);
		return;
	case 14:
		//load_enti_14(file, seq_id); // some trigger 
		return;
	case 20:
		//load_enti_20(file, seq_id);// some trigger near doors
		return;
	case 21:
		load_door_entity(file, seq_id); // movable obj
		return;
	case 33:
		//load_effect(file, seq_id);
		return;
	case 35:
		load_enti_35(file, seq_id);// some trigger near doors
		return;
	case 63:
		load_enti_63(file, seq_id); //path node
		return;
	case 64:
		load_enti_64(file, seq_id); //path controller
		return;
	case 69:
		load_enti_69(file, seq_id);
		return;
	case 70:
		load_enti_70(file, seq_id); // Debug note
		return;
	case 71:
		//load_enti_71(file, seq_id); // special fx projector
		return;
	case 72:
		load_enti_72(file, seq_id); // some predator waypoints
		return;
	case 75:
		//load_enti_75(file, seq_id); // special fx light shafts
		return;
	case 77:
		//load_enti_77(file, seq_id); Trigger with so many ids
		return;
	case 78:
		load_enti_78(file, seq_id); // Triggers
		return;
	case 83:
		load_enti_83(file, seq_id); // Light obj
		return;
	case 92:
		load_enti_92(file, seq_id); //Alien
		return;
	case 94:
		load_enti_94(file, seq_id); //Facehugger
		return;
	case 95:
		load_enti_95(file, seq_id); // Alien Queen 
		return;
	case 97:
		load_enti_97(file, seq_id); // Predator
		return;
	case 100:
		load_enti_100(file, seq_id);// Waypoint activator
		return;
	case 114:
		load_enti_114(file, seq_id);// NPC waypoints
		return;
	case 115:
		load_enti_115(file, seq_id);
		return;
	case 122:
		load_enti_122(file, seq_id); // Computer console
		return;
	case 124:
		load_enti_124(file, seq_id); // Some waypoint controller
		return;
	case 126:
		load_enti_126(file, seq_id);
		return;
	case 132:
		//load_enti_132(file, seq_id);
		return;
	case 135:
		load_enti_135(file, seq_id);// Combat_androids
		return;
	case 136:
		load_enti_136(file, seq_id);
		return;
	case 138:
		load_enti_138(file, seq_id); // Predator spots
		return;
	case 140:
		load_enti_140(file, seq_id); // patrol area
		return;
	case 141:
		load_enti_141(file, seq_id);// Colonist
		return;
	case 142:
		load_enti_142(file, seq_id);// Marine
		return;
	case 144:
		load_enti_144(file, seq_id);// Obstacle
		return;
	case 145:
		load_enti_145(file, seq_id);
		return;
	case 146:
		load_enti_146(file, seq_id);// alien hidding spots
		return;
	case 148:
		load_enti_148(file, seq_id);// Some waypoint controller
		return;
	case 151:
		load_enti_151(file, seq_id);// Sentry Gun
		return;
	case 156:
		load_enti_156(file, seq_id); // Collectable
		return;
	case 157:
		load_enti_157(file, seq_id); // Target point (for NPC aim ajustments)
		return;
	case 158:
		load_enti_158(file, seq_id); // Mission target
		return;
	case 162:
		load_enti_162(file, seq_id); // Battery station
		return;
	case 163:
		load_enti_163(file, seq_id); // 
		return;
	case 165:
		load_enti_165(file, seq_id); // Retina scaner or button pad.
		return;
	case 166:
		load_enti_166(file, seq_id); // Areas
		return;
	case 167:
		load_enti_167(file, seq_id); // Some waypoint
		return;
	case 169:
		load_enti_169(file, seq_id); // 
		return;
	case 170:
		//load_enti_170(file, seq_id);// indoor area
		return;
	case 173:
		load_enti_173(file, seq_id); // Computer
		return;
	case 176:
		load_enti_176(file, seq_id); // Praetorian
		return;

	default:
		break;
	}

	return;
}

void load_area_entity(HANDLE file, DWORD entity_id)
{

	switch (entity_id)
	{
	
	default:
		break;
	}
	return;
}

void load_default_entity(DWORD ent_id, DWORD seq_id, wchar_t* file_name)
{
	DWORD bytes_readen = 0;
	if (g_default_column > 10.0f)
	{
		g_default_column = 0.0f;
		g_default_row += 1.0f;
	}

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, file_name);
	sprintf(name, "Entity %d \n", ent_id);
	Create_entity(name, data, ent_id, seq_id, { g_default_column, 0.0f,  g_default_row});
	g_default_column += 1.0f;
}

void load_global_entity(HANDLE file, DWORD entity_id, DWORD seq_id)
{

	DWORD bytes_readen = 0;
	switch (entity_id)
	{
	case 1:
		load_enti_1(file, seq_id); // trigger start ?
		return;
	case 3:
		load_enti_3(file, seq_id); // cutscene
		return;
	case 11:
		load_enti_11(file, seq_id); // call message
		return;
	case 32:
		load_enti_32(file, seq_id); // trigger hurt
		return;
	case 34:
		load_enti_34(file, seq_id); // multimanager
		return;
	case 42:
		load_enti_42(file, seq_id); // Timer sequence
		return;
	case 68:
		load_enti_68(file, seq_id); // Properties of 69
		return;
	case 74:
		load_enti_74(file, seq_id); // console command
		return;
	case 85:
		load_enti_85(file, seq_id); // Dialogue sequence
		return;
	case 130:
		load_enti_130(file, seq_id); // gamescores ?
		return;
	case 133:
		load_enti_133(file, seq_id); // some event
		return;
	case 147:
		load_enti_147(file, seq_id); // spawner
		return;
	case 149:
		load_enti_149(file, seq_id);
		return;
	default:
		break;
	}

	char* name = nullptr; char* data = nullptr;
	Alloc_name_and_data_str(&name, &data);
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);
	sprintf(name, "Global %d \n", entity_id);
	Create_global_entity(name, data, entity_id, seq_id);

	return;
}

void load_entity(wchar_t* file_name)
{
	wchar_t file_path[MAX_PATH] = { 0 };
	wsprintf(file_path, L"%ws\\ENTI_chunk\\", g_path2);
	lstrcat(file_path, file_name);
	g_curr_file = file_name;
	//dbgprint("Entity", "Trying load %ws \n", file_path);
	DWORD a = 0;
	HANDLE f = CreateFileW(file_path, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	ENTI_header file_header;
	READ(file_header);
	if (GetLastError() != ERROR_SUCCESS)
	{
		dbgprint("Entity", "Opening file Error: %d\n", GetLastError());
		return;
	}
	if (file_header.Magic != (DWORD)'ITNE')
	{
		dbgprint("Entity", "Opening file Error: INCORRECT FILE HEADER\n");
		return;
	}
	DWORD ent_id = file_header.ent_id;
	ent_id = file_header.ent_id > 0x8000 ? ent_id - 0x7FA5 : ent_id;
	if (ent_id > 178 || ent_id <= 0)
	{
		dbgprint("Entity", "Ent_id > 173 or <= 0: %ws \n", file_name);
		return;
	}
	ENTITY_TYPE type = e_info[ent_id].type;

	//dbgprint("enti_debug", "Trying load entity %d (%d) (%ws)\n", ent_id, file_header.seq_id, file_name);

	switch (type)
	{
	case ENTITY_NONE:
	case ENTITY_UNKNOWN:
		load_default_entity(ent_id, file_header.seq_id, file_name);
		break;
	case ENTITY_GLOBAL:
		load_global_entity(f, ent_id, file_header.seq_id);
		break;
	case ENTITY_AREA: // actualy not used yet
		load_area_entity(f, ent_id);
		break;
	case ENTITY_POINT:
		load_point_entity(f, ent_id, file_header.seq_id);
		break;
	default:
		dbgprint("Entity", "Error in switch case condition: type = %d \n", type);
	}

	CloseHandle(f);
	return;
}


