#include "Entity_loader.h"
#include "Debug.h"
#include "Hash_tree.h"
#include "Models.h"

//Macro
#define SFPS(file, pos) SetFilePointer(file, pos, 0, FILE_BEGIN)
#define SFPC(file, pos) SetFilePointer(file, pos, 0, FILE_CURRENT)
#define READ(file, buffer, size, a) ReadFile(file, buffer, size, a, NULL)


typedef struct {
	DWORD id = 0;
	ENTITY_TYPE type;
	DWORD pos_offset;
	DWORD pos2_offset;
	//add support of text messages and custom loaders !
} entity_info;

struct e_act
{
	WORD act_id;
	WORD msgid;
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
std::vector<entity_p> g_entity_p;

float g_default_row = 0.0f;
float g_default_column = 0.0f;
wchar_t* g_curr_file;

// SMSG table
// count / ptr / ptr2 /; ptr for id < 30000 / ptr2 for id > 30000
// ptr -> [p1, p2 ... pn]
// p1 -> SMSG_subentry array [e1, e2, ... en]

int SMSG_count1 = 0;
int SMSG_count2 = 0;
void** SMSG_ptr1;
void** SMSG_ptr2;

entity_info e_info[174] = {
	{ 0, ENTITY_UNKNOWN, 0, 0},
	{ 1, ENTITY_GLOBAL, 0, 0 },
	{ 2, ENTITY_UNKNOWN, 0, 0 },
	{ 3, ENTITY_GLOBAL, 0, 0 },
	{ 4, ENTITY_UNKNOWN, 0, 0 },
	{ 5, ENTITY_UNKNOWN, 0, 0 },
	{ 6, ENTITY_UNKNOWN, 0, 0 },
	{ 7, ENTITY_POINT, 0x5C, 0 },
	{ 8, ENTITY_UNKNOWN, 0, 0 },
	{ 9, ENTITY_UNKNOWN, 0, 0 },
	{ 10, ENTITY_UNKNOWN, 0, 0 },
	{ 11, ENTITY_GLOBAL, 0, 0 },
	{ 12, ENTITY_UNKNOWN, 0, 0 },
	{ 13, ENTITY_UNKNOWN, 0, 0 },
	{ 14, ENTITY_POINT, 0, 0 },
	{ 15, ENTITY_UNKNOWN, 0, 0 },
	{ 16, ENTITY_UNKNOWN, 0, 0 },
	{ 17, ENTITY_UNKNOWN, 0, 0 },
	{ 18, ENTITY_UNKNOWN, 0, 0 },
	{ 19, ENTITY_UNKNOWN, 0, 0 },
	{ 20, ENTITY_POINT, 0x58, 0x64 },
	{ 21, ENTITY_POINT, 0, 0 },
	{ 22, ENTITY_UNKNOWN, 0, 0 },
	{ 23, ENTITY_UNKNOWN, 0, 0 },
	{ 24, ENTITY_UNKNOWN, 0, 0 },
	{ 25, ENTITY_UNKNOWN, 0, 0 },
	{ 26, ENTITY_UNKNOWN, 0, 0 },
	{ 27, ENTITY_UNKNOWN, 0, 0 },
	{ 28, ENTITY_UNKNOWN, 0, 0 },	
	{ 29, ENTITY_UNKNOWN, 0, 0 },
	{ 30, ENTITY_UNKNOWN, 0, 0 },
	{ 31, ENTITY_UNKNOWN, 0, 0 },
	{ 32, ENTITY_GLOBAL, 0, 0 },
	{ 33, ENTITY_POINT, 0x38, 0x48 },
	{ 34, ENTITY_GLOBAL, 0, 0 },
	{ 35, ENTITY_POINT, 0, 0 },
	{ 36, ENTITY_UNKNOWN, 0, 0 },
	{ 37, ENTITY_UNKNOWN, 0, 0 },
	{ 38, ENTITY_UNKNOWN, 0, 0 },
	{ 39, ENTITY_UNKNOWN, 0, 0 },
	{ 40, ENTITY_UNKNOWN, 0, 0 },
	{ 41, ENTITY_UNKNOWN, 0, 0 },
	{ 42, ENTITY_UNKNOWN, 0, 0 },
	{ 43, ENTITY_UNKNOWN, 0, 0 },
	{ 44, ENTITY_UNKNOWN, 0, 0 },
	{ 45, ENTITY_UNKNOWN, 0, 0 },
	{ 46, ENTITY_UNKNOWN, 0, 0 },
	{ 47, ENTITY_UNKNOWN, 0, 0 },
	{ 48, ENTITY_UNKNOWN, 0, 0 },
	{ 49, ENTITY_UNKNOWN, 0, 0 },
	{ 50, ENTITY_UNKNOWN, 0, 0 },
	{ 51, ENTITY_UNKNOWN, 0, 0 },
	{ 52, ENTITY_UNKNOWN, 0, 0 },
	{ 53, ENTITY_UNKNOWN, 0, 0 },
	{ 54, ENTITY_UNKNOWN, 0, 0 },
	{ 55, ENTITY_UNKNOWN, 0, 0 },
	{ 56, ENTITY_UNKNOWN, 0, 0 },
	{ 57, ENTITY_UNKNOWN, 0, 0 },
	{ 58, ENTITY_UNKNOWN, 0, 0 },
	{ 59, ENTITY_UNKNOWN, 0, 0 },
	{ 60, ENTITY_UNKNOWN, 0, 0 },
	{ 61, ENTITY_UNKNOWN, 0, 0 },
	{ 62, ENTITY_UNKNOWN, 0, 0 },
	{ 63, ENTITY_POINT, 0, 0 },
	{ 64, ENTITY_POINT, 0, 0 },
	{ 65, ENTITY_UNKNOWN, 0, 0 },
	{ 66, ENTITY_UNKNOWN, 0, 0 },
	{ 67, ENTITY_UNKNOWN, 0, 0 },
	{ 68, ENTITY_GLOBAL, 0, 0 },
	{ 69, ENTITY_POINT, 0, 0 },
	{ 70, ENTITY_POINT, 0, 0 },
	{ 71, ENTITY_POINT, 0, 0 },
	{ 72, ENTITY_UNKNOWN, 0, 0 },
	{ 73, ENTITY_UNKNOWN, 0, 0 },
	{ 74, ENTITY_GLOBAL, 0, 0 },
	{ 75, ENTITY_POINT, 0, 0 },
	{ 76, ENTITY_UNKNOWN, 0, 0 },
	{ 77, ENTITY_POINT, 0, 0 },
	{ 78, ENTITY_POINT, 0, 0 },
	{ 79, ENTITY_UNKNOWN, 0, 0 },
	{ 80, ENTITY_UNKNOWN, 0, 0 },
	{ 81, ENTITY_UNKNOWN, 0, 0 },
	{ 82, ENTITY_UNKNOWN, 0, 0 },
	{ 83, ENTITY_POINT, 0, 0 },
	{ 84, ENTITY_UNKNOWN, 0, 0 },
	{ 85, ENTITY_UNKNOWN, 0, 0 },
	{ 86, ENTITY_UNKNOWN, 0, 0 },
	{ 87, ENTITY_UNKNOWN, 0, 0 },
	{ 88, ENTITY_UNKNOWN, 0, 0 },
	{ 89, ENTITY_UNKNOWN, 0, 0 },
	{ 90, ENTITY_UNKNOWN, 0, 0 },
	{ 91, ENTITY_UNKNOWN, 0, 0 },
	{ 92, ENTITY_POINT, 0, 0 },
	{ 93, ENTITY_UNKNOWN, 0, 0 },
	{ 94, ENTITY_POINT, 0, 0 },
	{ 95, ENTITY_UNKNOWN, 0, 0 },
	{ 96, ENTITY_UNKNOWN, 0, 0 },
	{ 97, ENTITY_POINT, 0, 0 },
	{ 98, ENTITY_UNKNOWN, 0, 0 },
	{ 99, ENTITY_UNKNOWN, 0, 0 },
	{ 100, ENTITY_POINT, 0, 0 },
	{ 101, ENTITY_UNKNOWN, 0, 0 },
	{ 102, ENTITY_UNKNOWN, 0, 0 },
	{ 103, ENTITY_UNKNOWN, 0, 0 },
	{ 104, ENTITY_UNKNOWN, 0, 0 },
	{ 105, ENTITY_UNKNOWN, 0, 0 },
	{ 106, ENTITY_UNKNOWN, 0, 0 },
	{ 107, ENTITY_UNKNOWN, 0, 0 },
	{ 108, ENTITY_UNKNOWN, 0, 0 },
	{ 109, ENTITY_UNKNOWN, 0, 0 },
	{ 110, ENTITY_UNKNOWN, 0, 0 },
	{ 111, ENTITY_UNKNOWN, 0, 0 },
	{ 112, ENTITY_UNKNOWN, 0, 0 },
	{ 113, ENTITY_UNKNOWN, 0, 0 },
	{ 114, ENTITY_POINT, 0, 0 },
	{ 115, ENTITY_UNKNOWN, 0, 0 },
	{ 116, ENTITY_UNKNOWN, 0, 0 },
	{ 117, ENTITY_UNKNOWN, 0, 0 },
	{ 118, ENTITY_UNKNOWN, 0, 0 },
	{ 119, ENTITY_UNKNOWN, 0, 0 },
	{ 120, ENTITY_UNKNOWN, 0, 0 },
	{ 121, ENTITY_UNKNOWN, 0, 0 },
	{ 122, ENTITY_UNKNOWN, 0, 0 },
	{ 123, ENTITY_UNKNOWN, 0, 0 },
	{ 124, ENTITY_UNKNOWN, 0, 0 },
	{ 125, ENTITY_UNKNOWN, 0, 0 },
	{ 126, ENTITY_POINT, 0, 0 },
	{ 127, ENTITY_UNKNOWN, 0, 0 },
	{ 128, ENTITY_UNKNOWN, 0, 0 },
	{ 129, ENTITY_UNKNOWN, 0, 0 },
	{ 130, ENTITY_GLOBAL, 0, 0 },
	{ 131, ENTITY_UNKNOWN, 0, 0 },
	{ 132, ENTITY_POINT, 0, 0 },
	{ 133, ENTITY_GLOBAL, 0, 0 },
	{ 134, ENTITY_UNKNOWN, 0, 0 },
	{ 135, ENTITY_POINT, 0, 0 },
	{ 136, ENTITY_POINT, 0, 0 },
	{ 137, ENTITY_UNKNOWN, 0, 0 },
	{ 138, ENTITY_POINT, 0, 0 },
	{ 139, ENTITY_UNKNOWN, 0, 0 },
	{ 140, ENTITY_POINT, 0, 0 },
	{ 141, ENTITY_POINT, 0, 0 },
	{ 142, ENTITY_POINT, 0, 0 },
	{ 143, ENTITY_UNKNOWN, 0, 0 },
	{ 144, ENTITY_POINT, 0, 0 },
	{ 145, ENTITY_POINT, 0, 0 },
	{ 146, ENTITY_POINT, 0, 0 },
	{ 147, ENTITY_UNKNOWN, 0, 0 },
	{ 148, ENTITY_UNKNOWN, 0, 0 },
	{ 149, ENTITY_GLOBAL, 0, 0 },
	{ 150, ENTITY_UNKNOWN, 0, 0 },
	{ 151, ENTITY_UNKNOWN, 0, 0 },
	{ 152, ENTITY_UNKNOWN, 0, 0 },
	{ 153, ENTITY_UNKNOWN, 0, 0 },
	{ 154, ENTITY_UNKNOWN, 0, 0 },
	{ 155, ENTITY_UNKNOWN, 0, 0 },
	{ 156, ENTITY_POINT, 0, 0 },
	{ 157, ENTITY_POINT, 0, 0 },
	{ 158, ENTITY_POINT, 0, 0 },
	{ 159, ENTITY_UNKNOWN, 0, 0 },
	{ 160, ENTITY_UNKNOWN, 0, 0 },
	{ 161, ENTITY_UNKNOWN, 0, 0 },
	{ 162, ENTITY_POINT, 0, 0 },
	{ 163, ENTITY_POINT, 0, 0 },
	{ 164, ENTITY_UNKNOWN, 0, 0 },
	{ 165, ENTITY_UNKNOWN, 0, 0 },
	{ 166, ENTITY_POINT, 0, 0 },
	{ 167, ENTITY_UNKNOWN, 0, 0 },
	{ 168, ENTITY_UNKNOWN, 0, 0 },
	{ 169, ENTITY_POINT, 0, 0 },
	{ 170, ENTITY_POINT, 0, 0 },
	{ 171, ENTITY_UNKNOWN, 0, 0 },
	{ 172, ENTITY_UNKNOWN, 0, 0 },
	{ 173, ENTITY_UNKNOWN, 0, 0 },
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

void Create_global_entity(char* name, char* data, DWORD ent_id, DWORD seq_id, int connected_count = 0, DWORD* connected_list = nullptr)
{
	if (g_default_column > 10.0f)
	{
		g_default_column = 0.0f;
		g_default_row += 1.0f;
	}
	Create_entity(name, data, ent_id, seq_id, { g_default_column, 0.0f,  g_default_row}, Colors::Cyan, nullptr, nullptr, connected_count, connected_list);
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

void load_enti_1(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x2C);
	DWORD num;
	READ(f, &num, 4, &a);
	SFPC(f, num * 4 + 8);

	DWORD founded = 0;
	DWORD* seq_ids = nullptr;

	WORD* msgids = (WORD*)malloc(5 * 2);
	READ(f, msgids, 5 * 2, &a);
	seq_ids = search_in_SMSG(msgids[2], 0, &founded);
	SFPC(f, 6);
	float f1, f2;
	READ(f, &f1, 4, &a);
	READ(f, &f2, 4, &a);

	char* data = (char*)malloc(256);
	ZeroMemory(data, 256);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "Entity message_like_trigger (1)\n");
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n msgid: %d \n time?: %f %f \n ", seq_id, g_curr_file, msgids[2], f1, f2);

	Create_global_entity(name, data, 1, seq_id, founded, seq_ids);
}

void load_enti_3(HANDLE f, DWORD seq_id)
{
	SFPS(f, 0x30);
	char* str1 = (char*)malloc(128);
	ZeroMemory(str1, 128);
	read_padded_str(f, str1);

	char* data = (char*)malloc(256);
	ZeroMemory(data, 256);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "Entity cutscene start (3)\n");
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Name: %s \n ", seq_id, g_curr_file, str1);

	Create_global_entity(name, data, 3, seq_id);
}

void load_prop(HANDLE file, DWORD seq_id)
{
	DWORD bytes_readen = 0;
	
	SetFilePointer(file, 0x58, NULL, FILE_BEGIN);
	int i;
	ReadFile(file, &i, 4, &bytes_readen, NULL);
	SetFilePointer(file, 0x5C + i*4, NULL, FILE_BEGIN);
	
	entity_prop* prop = new entity_prop;
	prop->visible = 1;
	ReadFile(file, &(prop->pos), sizeof(XMFLOAT3), &bytes_readen, NULL);
	ReadFile(file, &(prop->angle), sizeof(XMFLOAT4), &bytes_readen, NULL);
	SetFilePointer(file, 0x21, NULL, FILE_CURRENT);
	ReadFile(file, &(prop->special), sizeof(DWORD), &bytes_readen, NULL);
	SetFilePointer(file, 4, NULL, FILE_CURRENT);
	ReadFile(file, &(prop->model_hash), sizeof(DWORD), &bytes_readen, NULL);
	SetFilePointer(file, 0x1c, NULL, FILE_CURRENT);
	ReadFile(file, &(prop->group_id), sizeof(DWORD), &bytes_readen, NULL);
	
	bool movable = 1;
	if (prop->group_id & 0b100)
	{
		movable = 0;
	}

	hash_tree_node2* node = search_by_hash2(model_hashs, prop->model_hash);
	if (!node)
	{
		node = g_default_model; // Default mdl
	}
	model_info* mi = (model_info*)node->ptr;

	char* data = (char*)malloc(256);
	ZeroMemory(data, 256);
	const char* str = (movable) ? "true" : "false";
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Movable: %s \n Special: %x \n Flags: %d \n Model_name: \"%s\" \n", seq_id, g_curr_file, str, prop->special, prop->group_id, mi->mdl_name);
	char* name = (char*)malloc(24);
	ZeroMemory(name, 24);
	sprintf(name, "Entity Prop (7)\n");
	Create_entity(name, data, 7, seq_id, prop->pos, {1.0f, 1.0f, 1.0f, 1.0f}, prop);
	//g_props.push_back(prop);
	//dbgprint("Entity", "Loaded prop entity: (pos: %f %f %f) (angle: %f %f %f %f) (hash: %X) group id %d\n", prop.pos.x, prop.pos.y, prop.pos.z, prop.angle.x, prop.angle.y, prop.angle.z, prop.angle.w, prop.model_hash, prop.group_id);
}

void load_enti_11(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x3C);
	DWORD count = 0;
	READ(f, &count, 4, &a);

	DWORD found = 0;
	DWORD totaly_found = 0;
	DWORD current_pos = 0;
	DWORD* seq_ids = nullptr;

	for (int i = 0; i < count; i++)
	{
		WORD msgid;
		READ(f, &msgid, 2, &a);
		SFPC(f, 6);
		found = 0;
		search_in_SMSG(msgid, 0, &found);
		totaly_found += found;
	}
	//dbgprint("e_act", "count %d totaly_found %d", count, totaly_found);
	seq_ids = (DWORD*)malloc(totaly_found * 4);
	ZeroMemory(seq_ids, totaly_found * 4);
	SFPS(f, 0x40);

	for (int i = 0; i < count; i++)
	{
		WORD msgid;
		READ(f, &msgid, 2, &a);
		SFPC(f, 6);
		found = 0;
		DWORD* seq_ids2 = search_in_SMSG(msgid, 0, &found);
		if (seq_ids2 == nullptr)
			continue;
		memcpy(seq_ids + current_pos, seq_ids2, found * 4);
		current_pos += found;
	}
	found = totaly_found;
	//dbgprint("e_act", "ent 11 %d \n", seq_id);
	/*
	for (int i = 0; i < found; i++)
	{
	dbgprint("e_act", "seq_id: %d \n", seq_ids[i]);
	}
	dbgprint("e_act", "msgid %d\n", msgids[2]);
	for (int i = 0; i < founded; i++)
	{
	dbgprint("e_act", "seq_id: %d \n", seq_ids[i]);
	}*/

	char* data = (char*)malloc(512);
	ZeroMemory(data, 512);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "Entity message (11)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);

	Create_global_entity(name, data, 11, seq_id, found, seq_ids);
}

void load_enti_14(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x38);
	DWORD* seq_ids = (DWORD*)malloc(4);
	READ(f, seq_ids, 4, &a);
	XMFLOAT3 pos;
	READ(f, &pos, 12, &a);

	char* data = (char*)malloc(256);
	ZeroMemory(data, 256);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "Entity . (14)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);
	Create_entity(name, data, 14, seq_id, pos, Colors::Yellow, nullptr, nullptr, 1, seq_ids);
}

void load_enti_20(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x58);
	asura_bbox bb;
	XMFLOAT4 rot;
	READ(f, &bb, 24, &a);
	READ(f, &rot, 16, &a);
	//g_lines.push_back({ pos, pos2, XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f) }); // blue line
	bbox* bbox_p = new bbox;
	bbox_p->p1 = {bb.x1, bb.y1, bb.z1 };
	bbox_p->p2 = {bb.x2, bb.y2, bb.z2 };
	bbox_p->rot = rot;
	bbox_p->Color = { 1.0f, 0.0f, 0.0f, 1.0f };
	char* data = (char*)malloc(256);
	ZeroMemory(data, 256);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "Entity . (20)\n");

	XMVECTOR p1, p2;
	p1 = XMLoadFloat3(&(bbox_p->p1));
	p2 = XMLoadFloat3(&(bbox_p->p2));
	XMFLOAT3 pos;
	XMStoreFloat3(&pos, (p1 + p2) / 2);

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);
	Create_entity(name, data, 20, seq_id, pos, Colors::Magenta, nullptr, bbox_p);
}

void load_door_entity(HANDLE file, DWORD seq_id)
{
	DWORD bytes_readen = 0;

	DWORD* p_activator_seq_id = (DWORD*)malloc(4);
	SetFilePointer(file, 0x28, NULL, FILE_BEGIN);
	ReadFile(file, p_activator_seq_id, 4, &bytes_readen, NULL);
	//dbgprint("Doors", "Activator_id: %d \n", *p_activator_seq_id);
	SetFilePointer(file, 0x54, NULL, FILE_BEGIN);
	int i;
	ReadFile(file, &i, 4, &bytes_readen, NULL);
	SetFilePointer(file, 0x58 + i * 4, NULL, FILE_BEGIN);

	entity_prop* prop = new entity_prop;
	prop->visible = 1;
	ReadFile(file, &(prop->pos), sizeof(XMFLOAT3), &bytes_readen, NULL);
	ReadFile(file, &(prop->angle), sizeof(XMFLOAT4), &bytes_readen, NULL);
	SetFilePointer(file, 0x21, NULL, FILE_CURRENT);
	ReadFile(file, &(prop->special), sizeof(DWORD), &bytes_readen, NULL);
	SetFilePointer(file, 4, NULL, FILE_CURRENT);
	ReadFile(file, &(prop->model_hash), sizeof(DWORD), &bytes_readen, NULL);
	SetFilePointer(file, 0x1c, NULL, FILE_CURRENT);
	ReadFile(file, &(prop->group_id), sizeof(DWORD), &bytes_readen, NULL);

	bool movable = 1;
	if (prop->group_id & 0b100)
	{
		movable = 0;
	}
	hash_tree_node2* node = search_by_hash2(model_hashs, prop->model_hash);
	if (!node)
	{
		node = g_default_model; // Default mdl
	}
	model_info* mi = (model_info*)node->ptr;

	char* data = (char*)malloc(256);
	ZeroMemory(data, 256);
	const char* str = (movable) ? "true" : "false";
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Movable: %s \n Special: %x \n Flags: %d \n Model_name: \"%s\" \n", seq_id, g_curr_file, str, prop->special, prop->group_id, mi->mdl_name);
	char* name = (char*)malloc(24);
	ZeroMemory(name, 24);
	sprintf(name, "Entity Movable_obj (21) \n");
	bbox* e_bbox = new bbox;
	e_bbox->p1 = XMFLOAT3(prop->pos.x + 0.2f, prop->pos.y + 0.2f, prop->pos.z + 0.2f);
	e_bbox->p2 = XMFLOAT3(prop->pos.x - 0.2f, prop->pos.y - 0.2f, prop->pos.z - 0.2f);
	e_bbox->Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Create_entity(name, data, 21, seq_id, prop->pos, Colors::White, prop, e_bbox, 1, p_activator_seq_id);
	//g_props.push_back(prop);
	//g_bboxes.push_back({ XMFLOAT3(prop.pos.x + 0.2f, prop.pos.y + 0.2f, prop.pos.z + 0.2f),XMFLOAT3(prop.pos.x - 0.2f, prop.pos.y - 0.2f, prop.pos.z - 0.2f), XMFLOAT4(1.0f,0.0f,1.0f,1.0f) }); // small cyan bbox
}

void load_enti_32(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x30);
	DWORD num;
	READ(f, &num, 4, &a);
	SFPC(f, num * 4 + 16);
	float f1;
	READ(f, &f1, 4, &a);
	SFPC(f, 8);
	DWORD num2;
	READ(f, &num2, 4, &a);
	for (int i = 0; i < num2; i++)
	{
		SFPC(f, 8);
		asura_bbox bb;
		XMFLOAT4 rot;
		READ(f, &bb, 24, &a);
		READ(f, &rot, 16, &a);
		g_bboxes.push_back({ { bb.x1,bb.y1, bb.z1 },{ bb.x2,bb.y2, bb.z2 }, XMFLOAT4(0.2f, 0.2f, 0.8f, 1.0f), rot });
	}
	char* data = (char*)malloc(256);
	ZeroMemory(data, 256);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "Trigger Hurt (32)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Damage: %f \n", seq_id, g_curr_file, f1);
	
	Create_global_entity( name, data, 32, seq_id);
}

void load_enti_34(HANDLE file, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(file, 0x20);
	int num1;
	READ(file, &num1, 4, &a); // Two Section of HASH and some SHORT id
	SFPC(file, num1 * 6);
	int num2;
	READ(file, &num2, 4, &a);
	SFPC(file, num2 * 6);
	int seq_id_count;
	READ(file, &seq_id_count, 4, &a);
	DWORD* seq_ids = (DWORD*)malloc(4 * seq_id_count);
	READ(file, seq_ids, 4 * seq_id_count, &a);

	char* data = (char*)malloc(128);
	ZeroMemory(data, 128);
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);
	char* name = (char*)malloc(24);
	ZeroMemory(name, 24);
	sprintf(name, "Entity Multimanager ? (34)\n");
	Create_global_entity(name, data, 34, seq_id, seq_id_count, seq_ids);
}

void load_enti_35(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x2C);
	DWORD num;
	READ(f, &num, 4, &a);
	SFPC(f, num * 4 + 8);

	DWORD founded = 0;
	DWORD* seq_ids = nullptr;

	WORD* msgids = (WORD*)malloc(5 * 2);
	READ(f, msgids, 5 * 2, &a);
	seq_ids = search_in_SMSG(msgids[2], 0, &founded);
	SFPC(f, 6);
	float f1;
	READ(f, &f1, 4, &a);
	SFPS(f, 0x74);
	XMFLOAT3 pos;
	READ(f, &pos, 12, &a);

	g_bboxes.push_back({ { pos.x + 0.2f,pos.y + 0.2f, pos.z + 0.2f },{ pos.x - 0.2f,pos.y - 0.2f, pos.z - 0.2f }, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), {0.0f, 0.0f, 0.0f, 1.0f} });
	asura_bbox bb = { pos.x + f1, pos.x - f1, pos.y + f1, pos.y - f1 , pos.z + f1, pos.z - f1 };
	bbox* bbox_p = new bbox;
	bbox_p->Color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	bbox_p->rot = { 0.0f, 0.0f, 0.0f, 1.0f };
	bbox_p->p1 = { bb.x1,bb.y1, bb.z1 };
	bbox_p->p2 = { bb.x2,bb.y2, bb.z2 };

	char* data = (char*)malloc(256);
	ZeroMemory(data, 256);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "Trigger (dynamic) (35)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n radius: %f \n", seq_id, g_curr_file, f1);
	Create_entity(name, data, 35, seq_id, pos, Colors::Red, nullptr, bbox_p, founded, seq_ids);
}



//Waypoints
void load_enti_63(HANDLE file, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(file, 0x34);
	int num1;
	READ(file, &num1, 4, &a);
	SFPC(file, num1 * 4);
	DWORD flags;
	READ(file, &flags, 4, &a);
	XMFLOAT3 pos;
	READ(file, &pos, 12, &a);
	char* data = (char*)malloc(256);
	ZeroMemory(data, 128);
	char* name = (char*)malloc(24);
	ZeroMemory(name, 24);
	sprintf(name, "Entity path_point (63)\n");

	DWORD state;
	bool reversible = flags & 0x80;
	bool start_node = flags & 0x10;
	const char* str = (reversible) ? "true" : "false";


	if (start_node)
	{
		DWORD* seq_ids = (DWORD*)malloc(4);
		READ(file, seq_ids, 4, &a);
		SFPC(file, 4);
		READ(file, &state, 4, &a);
		sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n State1: %x\n State2: %x\n Reversible: %s \nStart_node \n", seq_id, g_curr_file, flags, LOWORD(state), HIWORD(state), str);
		Create_entity(name, data, 63, seq_id, pos, Colors::Cyan, nullptr, nullptr, 1, seq_ids);
	}
	else
	{
		SFPC(file, 4);
		READ(file, &state, 4, &a);
		sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n State1: %x\n State2: %x\n Reversible: %s \n End_node \n", seq_id, g_curr_file, flags, LOWORD(state), HIWORD(state), str);
		Create_entity(name, data, 63, seq_id, pos, Colors::Cyan, nullptr, nullptr, 0, nullptr);
	}
}

void load_enti_64(HANDLE file, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(file, 0x38);
	DWORD flags;
	READ(file, &flags, 4, &a);
	XMFLOAT3 pos;
	READ(file, &pos, 12, &a);
	SFPC(file, 0x4);
	DWORD* seq_ids = (DWORD*)malloc(8);
	READ(file, seq_ids, 8, &a);
	SFPC(file, 0x30);
	XMFLOAT3 pos2;
	READ(file, &pos2, 12, &a);
	
	g_lines.push_back({ pos, pos2, XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f) }); // blue line

	char* data = (char*)malloc(256);
	ZeroMemory(data, 256);
	char* name = (char*)malloc(24);
	ZeroMemory(name, 24);
	sprintf(name, "Entity path_controller (64)\n");

	bool reversible = flags & 0x80;
	const char* str = (reversible) ? "true" : "false";

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n Reversible: %s \n", seq_id, g_curr_file, flags, str);
	Create_entity(name, data, 64, seq_id, pos, Colors::Magenta, nullptr, nullptr, 2, seq_ids);

}

void load_enti_68(HANDLE file, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(file, 0x2C);
	int num1;
	READ(file, &num1, 4, &a);
	SFPC(file, num1 * 4 + 20);
	DWORD* seq_ids = (DWORD*)malloc(4);
	READ(file, seq_ids, 4, &a);

	char* data = (char*)malloc(128);
	ZeroMemory(data, 128);
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);
	char* name = (char*)malloc(24);
	ZeroMemory(name, 32);
	sprintf(name, "Entity Teleport property (68)\n");
	Create_global_entity(name, data, 68, seq_id, 1, seq_ids);
}

void load_enti_69(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x2C);
	int num1;
	READ(f, &num1, 4, &a);
	SFPC(f, num1 * 4);
	XMFLOAT3 pos;
	READ(f, &pos, 12, &a);

	char* data = (char*)malloc(128);
	ZeroMemory(data, 128);
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "Entity Teleport locations (69)\n");
	Create_entity(name, data, 69, seq_id, pos, Colors::Purple);
}

void load_enti_70(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x38);
	DWORD founded = 0;
	DWORD* seq_ids = nullptr;

	WORD* msgids = (WORD*)malloc(5 * 2);
	READ(f, msgids, 5 * 2, &a);
	seq_ids = search_in_SMSG(msgids[2], 0, &founded);
	/*
	dbgprint("e_act", "msgid %d\n", msgids[2]);
	for (int i = 0; i < founded; i++)
	{
		dbgprint("e_act", "seq_id: %d \n", seq_ids[i]);
	}*/

	SFPS(f, 0x58);
	XMFLOAT3 pos;
	READ(f, &pos, 12, &a);
	char* str1 = (char*)malloc(256);
	ZeroMemory(str1, 256);
	SFPS(f, 0x88);
	read_padded_str(f, str1);

	char* data = (char*)malloc(512);
	ZeroMemory(data, 512);
	char* name = (char*)malloc(24);
	ZeroMemory(name, 24);
	sprintf(name, "Entity debug node (70)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n note: %s \n", seq_id, g_curr_file, str1);

	Create_entity(name, data, 71, seq_id, pos, Colors::Magenta, nullptr, nullptr, founded, seq_ids);
}

void load_enti_71(HANDLE file, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(file, 0x34);
	DWORD flags;
	READ(file, &flags, 4, &a);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	READ(file, &pos, 12, &a);
	READ(file, &rot, 16, &a);
	SFPS(file, 0x180);
	char* str1 = (char*)malloc(128);
	ZeroMemory(str1, 128);
	char* str2 = (char*)malloc(128);
	ZeroMemory(str2, 128);
	char* str3 = (char*)malloc(128);
	ZeroMemory(str3, 128);
	read_padded_str(file, str1);
	read_padded_str(file, str2);
	read_padded_str(file, str3);

	char* data = (char*)malloc(512);
	ZeroMemory(data, 512);
	char* name = (char*)malloc(24);
	ZeroMemory(name, 24);
	sprintf(name, "Entity special fx (71)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n 1: %s \n 2: %s \n 3: %s \n", seq_id, g_curr_file, flags, str1, str2, str3);
	XMFLOAT3 dir = { pos.x + rot.x, pos.y + rot.y, pos.z + rot.z };
	g_lines.push_back({ pos, dir, XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f) }); // blue line

	Create_entity(name, data, 71, seq_id, pos, Colors::Yellow);
}

void load_enti_74(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x2C);
	DWORD num = 0;
	DWORD founded = 0;
	DWORD* seq_ids = nullptr;
	READ(f, &num, 4, &a);
	if (num != 0)
	{
		e_act* acts = (e_act*)malloc(num * 4);
		READ(f, acts, num * 4, &a);
		seq_ids = search_in_SMSG(acts[0].msgid, 0, &founded);
	}
	
	char* str1 = (char*)malloc(256);
	ZeroMemory(str1, 256);
	SFPC(f, 8);
	read_padded_str(f, str1);

	char* data = (char*)malloc(512);
	ZeroMemory(data, 512);
	char* name = (char*)malloc(24);
	ZeroMemory(name, 24);
	sprintf(name, "Entity console var (74)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n command: %s \n", seq_id, g_curr_file, str1);

	Create_global_entity( name, data, 74, seq_id, founded, seq_ids);
}

void load_enti_75(HANDLE file, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(file, 0x30);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	READ(file, &pos, 12, &a);
	READ(file, &rot, 16, &a);
	SFPS(file, 0x98);
	char* str1 = (char*)malloc(128);
	ZeroMemory(str1, 128);
	char* str2 = (char*)malloc(128);
	ZeroMemory(str2, 128);
	read_padded_str(file, str1);
	SFPC(file, 0xC);
	read_padded_str(file, str2);

	char* data = (char*)malloc(512);
	ZeroMemory(data, 128);
	char* name = (char*)malloc(24);
	ZeroMemory(name, 24);
	sprintf(name, "Entity special fx (75)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n 1: %s \n 2: %s \n", seq_id, g_curr_file, str1, str2);
	XMFLOAT3 dir = { pos.x + rot.x, pos.y + rot.y, pos.z + rot.z };
	g_lines.push_back({ pos, dir, XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f) }); // blue line

	Create_entity(name, data, 75, seq_id, pos, Colors::Yellow);
}

void load_enti_77(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x38);
	XMFLOAT3 pos;
	READ(f, &pos, 12, &a);
	SFPS(f, 0x74);
	asura_bbox bb;
	XMFLOAT4 rot;
	READ(f, &bb, 24, &a);
	READ(f, &rot, 16, &a);
	g_bboxes.push_back({ { bb.x1,bb.y1, bb.z1 },{ bb.x2,bb.y2, bb.z2 }, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), rot });

	char* data = (char*)malloc(512);
	ZeroMemory(data, 128);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "Entity special fx (77)\n");
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);
	Create_entity(name, data, 77, seq_id, pos, Colors::Red);
}

void load_enti_78(HANDLE file, DWORD seq_id)
{
	DWORD a = 0;

	SFPS(file, 0x40);

	DWORD founded = 0;
	DWORD* seq_ids = nullptr;

	WORD* msgids = (WORD*)malloc(5 * 2);
	READ(file, msgids, 5 * 2, &a);
	seq_ids = search_in_SMSG(msgids[2], 0, &founded);

	SFPS(file, 0x7D);
	DWORD num1;
	READ(file, &num1, 4, &a);
	for (int i = 0; i < num1; i++)
	{
		DWORD type;
		READ(file, &type, 4, &a);
		if (type == 1)
		{
			SFPC(file, 4);
			asura_bbox bb;
			XMFLOAT4 rot;
			READ(file, &bb, 24, &a);
			READ(file, &rot, 16, &a);
			XMFLOAT3 pos = { (bb.x1 + bb.x2) / 2,(bb.y1 + bb.y2) / 2, (bb.z1 + bb.z2) / 2 };
			g_bboxes.push_back({ {pos.x + 0.2f,pos.y + 0.2f, pos.z + 0.2f },{ pos.x - 0.2f,pos.y - 0.2f, pos.z - 0.2f }, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), rot });
			bbox* bbox_p = new bbox;
			bbox_p->Color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
			bbox_p->rot = rot;
			bbox_p->p1 = { bb.x1,bb.y1, bb.z1 };
			bbox_p->p2 = { bb.x2,bb.y2, bb.z2 };
			SFPC(file, 16);

			char* data = (char*)malloc(128);
			ZeroMemory(data, 128);
			char* name = (char*)malloc(24);
			ZeroMemory(name, 24);
			sprintf(name, "Entity (78)\n");
			sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Type 1 \n msgid: %d \n", seq_id, g_curr_file, msgids[2]);

			Create_entity(name, data, 78, seq_id, pos, Colors::Red, nullptr, bbox_p, founded, seq_ids);
			return;
		}
		if (type == 2)
		{
			SFPC(file, 4);
			XMFLOAT3 pos;
			float value;
			XMFLOAT4 rot;
			READ(file, &pos, 12, &a);
			READ(file, &value, 4, &a);
			READ(file, &rot, 16, &a);
			//dbgprint("dbg 78", "%f %f %f %f \n", rot.x, rot.y, rot.z, rot.w);
			g_bboxes.push_back({ { pos.x + 0.2f,pos.y + 0.2f, pos.z + 0.2f },{ pos.x - 0.2f,pos.y - 0.2f, pos.z - 0.2f }, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), rot });
			asura_bbox bb = { pos.x + value, pos.x - value, pos.y + value, pos.y - value , pos.z + value, pos.z - value };
			bbox* bbox_p = new bbox;
			bbox_p->Color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
			bbox_p->rot = rot;
			bbox_p->p1 = { bb.x1,bb.y1, bb.z1 };
			bbox_p->p2 = { bb.x2,bb.y2, bb.z2 };
			char* data = (char*)malloc(128);
			ZeroMemory(data, 128);
			char* name = (char*)malloc(24);
			ZeroMemory(name, 24);
			sprintf(name, "Entity (78)\n");
			sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Type 2 \n radius: %f \n msgid: %d \n", seq_id, g_curr_file, value, msgids[2]);

			Create_entity(name, data, 78, seq_id, pos, Colors::Red, nullptr, bbox_p, founded, seq_ids);
			return;
		}
		if (type == 3)
		{
			SFPC(file, 4);
			XMFLOAT3 pos;
			READ(file, &pos, 12, &a);
			SFPC(file, 24);

			char* data = (char*)malloc(128);
			ZeroMemory(data, 128);
			char* name = (char*)malloc(24);
			ZeroMemory(name, 24);
			sprintf(name, "Entity (78)\n");
			sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Type 3 \n msgid: %d \n", seq_id, g_curr_file, msgids[2]);

			Create_entity(name, data, 78, seq_id, pos, Colors::Red, nullptr, nullptr, founded, seq_ids);
			return;
		}
	}
}

//Light obj
void load_enti_83(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x30);
	DWORD num1;
	READ(f, &num1, 4, &a);
	SFPC(f, num1 * 4);
	DWORD hash;
	READ(f, &hash, 4, &a);

	XMFLOAT3 pos;
	XMFLOAT4 rot;
	READ(f, &pos, 12, &a);
	READ(f, &rot, 16, &a);

	char* data = (char*)malloc(128);
	ZeroMemory(data, 128);
	char* name = (char*)malloc(24);
	ZeroMemory(name, 24);
	sprintf(name, "Entity Light obj (83)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);
	//XMFLOAT3 dir = { pos.x + rot.x, pos.y + rot.y, pos.z + rot.z };
	//g_lines.push_back({ pos, dir, XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f) }); // blue line
	bbox* bbox_p = new bbox;
	bbox_p->p1 = { pos.x + 0.1f, pos.y + 0.1f, pos.z + 0.1f };
	bbox_p->p2 = { pos.x - 0.1f, pos.y - 0.1f, pos.z - 0.1f };
	bbox_p->rot = rot;
	bbox_p->Color = { 1.0f, 1.0f, 0.0f, 1.0f };
	/*
	entity_prop* prop = new entity_prop;
	ZeroMemory(prop, sizeof(entity_prop));
	prop->visible = 1;
	prop->model_hash = hash;
	prop->pos = pos;
	prop->angle = rot;
	*/
	Create_entity(name, data, 83, seq_id, pos, Colors::Yellow, nullptr, bbox_p);
}

void load_enti_92(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x3C);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	DWORD num1;
	READ(f, &num1, 4, &a);
	SFPC(f, num1 * 4);
	READ(f, &pos, 12, &a);
	READ(f, &rot, 16, &a);
	SFPC(f, 25);
	XMFLOAT2 some_float_data;
	READ(f, &some_float_data, 8, &a);
	SFPC(f, 116);
	DWORD* seq_ids = (DWORD*)malloc(8);
	READ(f, seq_ids, 8, &a);

	char* data = (char*)malloc(128);
	ZeroMemory(data, 128);
	char* name = (char*)malloc(24);
	ZeroMemory(name, 24);
	sprintf(name, "Entity alien? (92)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Curr_health: %f \n Max_health: %f \n", seq_id, g_curr_file, some_float_data.x, some_float_data.y);
	//XMFLOAT3 dir = { pos.x + rot.x, pos.y + rot.y, pos.z + rot.z };
	//g_lines.push_back({ pos, dir, XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f) }); // blue line
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
	SFPS(f, 0x3C);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	DWORD num1;
	READ(f, &num1, 4, &a);
	SFPC(f, num1 * 4);
	READ(f, &pos, 12, &a);
	READ(f, &rot, 16, &a);
	SFPC(f, 25);
	XMFLOAT2 some_float_data;
	READ(f, &some_float_data, 8, &a);
	SFPC(f, 104);
	DWORD* seq_ids = (DWORD*)malloc(8);
	READ(f, seq_ids, 8, &a);

	char* data = (char*)malloc(128);
	ZeroMemory(data, 128);
	char* name = (char*)malloc(24);
	ZeroMemory(name, 24);
	sprintf(name, "Entity facehugger ? (94)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Curr_health: %f \n Max_health: %f \n", seq_id, g_curr_file, some_float_data.x, some_float_data.y);
	//XMFLOAT3 dir = { pos.x + rot.x, pos.y + rot.y, pos.z + rot.z };
	//g_lines.push_back({ pos, dir, XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f) }); // blue line
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

void load_enti_97(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x3C);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	DWORD num1;
	READ(f, &num1, 4, &a);
	SFPC(f, num1 * 4);
	READ(f, &pos, 12, &a);
	READ(f, &rot, 16, &a);
	SFPC(f, 25);
	XMFLOAT2 some_float_data;
	READ(f, &some_float_data, 8, &a);
	SFPC(f, 112);
	DWORD* seq_ids = (DWORD*)malloc(8);
	READ(f, seq_ids, 8, &a);

	char* data = (char*)malloc(128);
	ZeroMemory(data, 128);
	char* name = (char*)malloc(24);
	ZeroMemory(name, 24);
	sprintf(name, "Entity ? (97)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Curr_health: %f \n Max_health: %f \n", seq_id, g_curr_file, some_float_data.x, some_float_data.y);
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

void load_enti_114(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x38);
	DWORD num;
	READ(f, &num, 4, &a);
	SFPC(f, num * 4);
	DWORD flags;
	XMFLOAT3 pos;
	READ(f, &flags, 4, &a);
	READ(f, &pos, 12, &a);
	DWORD* seq_ids = (DWORD*)malloc(4);
	READ(f, seq_ids, 4, &a);
	if (*seq_ids == -1)
		seq_ids = nullptr;

	char* data = (char*)malloc(256);
	ZeroMemory(data, 128);
	char* name = (char*)malloc(24);
	ZeroMemory(name, 24);
	sprintf(name, "Entity waypoint (114)\n");

	bool reversible = flags & 0x80;
	bool start_node = flags & 0x10;
	const char* str = (reversible) ? "true" : "false";
	const char* str2 = (start_node) ? "true" : "false";

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n Reversible: %s \nStart_node: %s \n", seq_id, g_curr_file, flags, str, str2);
	Create_entity(name, data, 63, seq_id, pos, Colors::Cyan, nullptr, nullptr, 1, seq_ids);
}

void load_enti_100(HANDLE file, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(file, 0x3C);
	DWORD flags;
	READ(file, &flags, 4, &a);
	XMFLOAT3 pos;
	READ(file, &pos, 12, &a);
	SFPC(file, 0x4);
	DWORD* seq_ids = (DWORD*)malloc(8);
	READ(file, seq_ids, 8, &a);
	SFPC(file, 0x30);
	XMFLOAT3 pos2;
	READ(file, &pos2, 12, &a);

	g_lines.push_back({ pos, pos2, XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f) }); // blue line

	char* data = (char*)malloc(256);
	ZeroMemory(data, 256);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "Entity waypoint_controller (100)\n");

	bool reversible = flags & 0x80;
	const char* str = (reversible) ? "true" : "false";

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n Reversible: %s \n", seq_id, g_curr_file, flags, str);
	Create_entity(name, data, 64, seq_id, pos, Colors::Cyan, nullptr, nullptr, 2, seq_ids);
}

void load_enti_126(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x2C);
	DWORD num;
	READ(f, &num, 4, &a);
	SFPC(f, num * 4);
	XMFLOAT3 pos;
	READ(f, &pos, 12, &a);

	char* data = (char*)malloc(256);
	ZeroMemory(data, 256);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "Checkpoint (126)\n");
	sprintf(data, "SEQ_ID: %d \n FILE: %ws\n", seq_id, g_curr_file);

	Create_entity(name, data, 126, seq_id, pos, Colors::Purple);
}

void load_enti_130(HANDLE f, DWORD seq_id)
{
	char* data = (char*)malloc(256);
	ZeroMemory(data, 256);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "GameScores? (130)\n");
	sprintf(data, "SEQ_ID: %d \n FILE: %ws\n", seq_id, g_curr_file);
	Create_global_entity(name, data, 130, seq_id);
}

void load_enti_132(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x2C);
	XMFLOAT3 pos;
	DWORD num1;
	READ(f, &num1, 4, &a);
	SFPC(f, num1 * 4);
	READ(f, &pos, 12, &a);
	SFPC(f, 24);

	asura_bbox ar;
	XMFLOAT4 rot;
	READ(f, &ar, 24, &a);
	READ(f, &rot, 16, &a);

	DWORD num2;
	READ(f, &num2, 4, &a);
	DWORD* seq_ids = (DWORD*)malloc(num2 * 4);
	READ(f, seq_ids, num2 * 4, &a);

	char* data = (char*)malloc(256);
	ZeroMemory(data, 256);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "Entity ??? (132)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);

	bbox* bbox_p = new bbox;
	bbox_p->p1 = { ar.x1, ar.y1, ar.z1 };
	bbox_p->p2 = { ar.x2, ar.y2, ar.z2 };
	bbox_p->rot = rot;
	bbox_p->Color = {0.0f, 0.0f, 0.0f, 1.0f};

	Create_entity(name, data, 132, seq_id, pos, Colors::White, nullptr, bbox_p, num2, seq_ids);
}

void load_enti_135(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x40);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	DWORD num1;
	READ(f, &num1, 4, &a);
	SFPC(f, num1 * 4);
	READ(f, &pos, 12, &a);
	READ(f, &rot, 16, &a);
	SFPC(f, 25);
	XMFLOAT2 some_float_data;
	READ(f, &some_float_data, 8, &a);
	SFPC(f, 0x24);
	DWORD type;
	READ(f, &type, 4, &a);
	int skip_bytes = type == 1 ? 0x23 : 0xD8;
	SFPC(f, skip_bytes);
	DWORD* seq_ids = (DWORD*)malloc(8);
	READ(f, seq_ids, 8, &a);

	char* data = (char*)malloc(128);
	ZeroMemory(data, 128);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "Entity Combat_droid (135)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Curr_health: %f \n Max_health: %f \n", seq_id, g_curr_file, some_float_data.x, some_float_data.y);
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
	prop->model_hash = hash_from_str(0, (char*)"combat_android");
	prop->pos = pos;
	prop->angle = rot;

	Create_entity(name, data, 135, seq_id, pos, Colors::Yellow, prop, bbox_p, 2, seq_ids);
}

void load_enti_133(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x30);
	DWORD count = 0;
	READ(f, &count, 4, &a);

	DWORD found = 0;
	DWORD totaly_found = 0;
	DWORD current_pos = 0;
	DWORD* seq_ids = nullptr;

	for (int i = 0; i < count; i++)
	{
		WORD msgid;
		SFPC(f, 2);
		READ(f, &msgid, 2, &a);
		found = 0;
		search_in_SMSG(msgid, 0, &found);
		totaly_found += found;
	}
	seq_ids = (DWORD*)malloc(totaly_found * 4);
	ZeroMemory(seq_ids, totaly_found * 4);
	SFPS(f, 0x34);

	for (int i = 0; i < count; i++)
	{
		WORD msgid;
		SFPC(f, 2);
		READ(f, &msgid, 2, &a);
		found = 0;
		DWORD* seq_ids2 = search_in_SMSG(msgid, 0, &found);
		if (seq_ids2 == nullptr)
			continue;
		memcpy(seq_ids + current_pos, seq_ids2, found * 4);
		current_pos += found;
	}
	found = totaly_found;

	char* data = (char*)malloc(512);
	ZeroMemory(data, 512);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "some event (133)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);

	Create_global_entity(name, data, 133, seq_id, found, seq_ids);
}

void load_enti_136(HANDLE file, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(file, 0x34);
	DWORD flags;
	READ(file, &flags, 4, &a);
	XMFLOAT3 pos;
	READ(file, &pos, 12, &a);
	char* data = (char*)malloc(256);
	ZeroMemory(data, 128);
	char* name = (char*)malloc(24);
	ZeroMemory(name, 24);
	sprintf(name, "Entity obstacle (136)\n");

	DWORD state;
	bool reversible = flags & 0x80;
	bool start_node = flags & 0x10;
	const char* str = (reversible) ? "true" : "false";


	if (start_node)
	{
		DWORD* seq_ids = (DWORD*)malloc(4);
		READ(file, seq_ids, 4, &a);
		SFPC(file, 4);
		READ(file, &state, 4, &a);
		sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n State1: %x\n State2: %x\n Reversible: %s \nStart_node \n", seq_id, g_curr_file, flags, LOWORD(state), HIWORD(state), str);
		Create_entity(name, data, 136, seq_id, pos, Colors::Cyan, nullptr, nullptr, 0, nullptr);
	}
	else
	{
		SFPC(file, 4);
		READ(file, &state, 4, &a);
		sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n State1: %x\n State2: %x\n Reversible: %s \n End_node \n", seq_id, g_curr_file, flags, LOWORD(state), HIWORD(state), str);
		Create_entity(name, data, 136, seq_id, pos, Colors::Cyan, nullptr, nullptr, 0, nullptr);
	}
}

void load_enti_138(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x38);
	DWORD num;
	READ(f, &num, 4, &a);
	SFPC(f, num * 4);
	DWORD num1;
	XMFLOAT3 pos;
	READ(f, &num1, 4, &a);
	READ(f, &pos, 12, &a);

	char* data = (char*)malloc(128);
	ZeroMemory(data, 128);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "Entity Predator spots? (138)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n flags: %x \n", seq_id, g_curr_file, num1);

	Create_entity(name, data, 138, seq_id, pos, Colors::Cyan);
}

void load_enti_140(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x40);
	asura_bbox ar;
	XMFLOAT4 rot;
	READ(f, &ar, 24, &a);
	READ(f, &rot, 16, &a);
	SFPS(f, 0x7C);
	DWORD num;
	READ(f, &num, 4, &a);
	SFPC(f, num * 4);
	DWORD num1;
	XMFLOAT3 pos;
	READ(f, &num1, 4, &a);
	READ(f, &pos, 12, &a);
	DWORD num2;
	DWORD num3;
	SFPC(f, 20);
	READ(f, &num2, 4, &a);
	DWORD* seq_ids = (DWORD*)malloc(num2 * 4);
	READ(f, seq_ids, num2 * 4, &a);
	READ(f, &num3, 4, &a);
	DWORD* seq_ids2 = (DWORD*)malloc((num3 + num2) * 4);
	memcpy(seq_ids2, seq_ids, num2 * 4);
	READ(f, seq_ids2 + num2, num3 * 4, &a);

	bbox* bbox_p = new bbox;
	bbox_p->p1 = { ar.x1, ar.y1, ar.z1 };
	bbox_p->p2 = { ar.x2, ar.y2, ar.z2 };
	bbox_p->rot = rot;
	bbox_p->Color = { 0.0f, 0.0f, 0.0f, 1.0f };

	char* data = (char*)malloc(128);
	ZeroMemory(data, 128);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "patrol area ? (140)\n");

	g_bboxes.push_back({ { pos.x + 0.2f, pos.y + 0.2f, pos.z + 0.2f }, { pos.x - 0.2f, pos.y - 0.2f, pos.z - 0.2f }, { 1.0f, 0.0f, 1.0f, 1.0f } });

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n flags: %x \n", seq_id, g_curr_file, num1);
	//dbgprint("ent 140", "%f %f %f %d %d\n", pos.x, pos.y, pos.z, num2, num3);
	Create_entity(name, data, 140, seq_id, pos, Colors::Magenta, nullptr, bbox_p, num2 + num3, seq_ids2);
}

void load_enti_141(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x40);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	DWORD num1;
	READ(f, &num1, 4, &a);
	SFPC(f, num1 * 4);
	READ(f, &pos, 12, &a);
	READ(f, &rot, 16, &a);
	SFPC(f, 25);
	XMFLOAT2 some_float_data;
	READ(f, &some_float_data, 8, &a);
	SFPC(f, 0x24);
	DWORD type;
	READ(f, &type, 4, &a);
	int skip_bytes = type == 1 ? 0x23 : 0x115;
	SFPC(f, skip_bytes);
	DWORD* seq_ids = (DWORD*)malloc(8);
	READ(f, seq_ids, 8, &a);

	char* data = (char*)malloc(128);
	ZeroMemory(data, 128);
	char* name = (char*)malloc(24);
	ZeroMemory(name, 24);
	sprintf(name, "Colonist (141)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Curr_health: %f \n Max_health: %f \n", seq_id, g_curr_file, some_float_data.x, some_float_data.y);
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
	SFPS(f, 0x40);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	DWORD num1;
	READ(f, &num1, 4, &a);
	SFPC(f, num1 * 4);
	READ(f, &pos, 12, &a);
	READ(f, &rot, 16, &a);
	SFPC(f, 25);
	XMFLOAT2 some_float_data;
	READ(f, &some_float_data, 8, &a);
	SFPC(f, 0x24);
	DWORD type;
	READ(f, &type, 4, &a);
	int skip_bytes = type == 1 ? 0x23 : 0xD8;
	SFPC(f, skip_bytes);
	DWORD* seq_ids = (DWORD*)malloc(8);
	READ(f, seq_ids, 8, &a);

	char* data = (char*)malloc(128);
	ZeroMemory(data, 128);
	char* name = (char*)malloc(24);
	ZeroMemory(name, 24);
	sprintf(name, "Entity soldier (142)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Curr_health: %f \n Max_health: %f \n", seq_id, g_curr_file, some_float_data.x, some_float_data.y);
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
	SFPS(f, 0x3C);
	DWORD flags;
	READ(f, &flags, 4, &a);
	XMFLOAT3 pos;
	READ(f, &pos, 12, &a);
	char* data = (char*)malloc(256);
	ZeroMemory(data, 256);
	char* name = (char*)malloc(128);
	ZeroMemory(name, 128);
	
	DWORD state;
	SFPC(f, 4);
	READ(f, &state, 4, &a);
	XMFLOAT3 angles;
	READ(f, &angles, 12, &a);
	DWORD check;
	READ(f, &check, 4, &a);
	DWORD type = check & 0xff; // first byte

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n angles?: %f %f %f", seq_id, g_curr_file, flags,
			angles.x, angles.y, angles.z);
	sprintf(name, "Entity obstacle (144) type %d\n", type);

	DWORD count;
	DWORD* seq_ids;
	XMFLOAT3 pos2;

	switch (type)
	{
	case 1:
		READ(f, &count, 4, &a);
		seq_ids = (DWORD*)malloc(count * 4);
		READ(f, seq_ids, count * 4, &a);
		Create_entity(name, data, 144, seq_id, pos, Colors::Cyan, nullptr, nullptr, count, seq_ids);
		break;
	case 2:
	case 3:
		SFPC(f, 4);
		READ(f, &pos2, 12, &a);
		g_lines.push_back({ pos, pos2, XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f) });
		SFPC(f, 12);
		READ(f, &count, 4, &a);
		seq_ids = (DWORD*)malloc(count * 4);
		READ(f, seq_ids, count * 4, &a);
		Create_entity(name, data, 144, seq_id, pos, Colors::Cyan, nullptr, nullptr, count, seq_ids);
		break;
	case 4:
		SFPC(f, 4);
		READ(f, &pos2, 12, &a);
		g_lines.push_back({ pos, pos2, XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f) });
		SFPC(f, 12);
		XMFLOAT3 pos3;
		READ(f, &pos3, 12, &a);
		g_lines.push_back({ pos, pos3, XMFLOAT4(0.2f, 0.7f, 0.7f, 1.0f) });
		SFPC(f, 12);
		READ(f, &count, 4, &a);
		seq_ids = (DWORD*)malloc(count * 4);
		READ(f, seq_ids, count * 4, &a);
		Create_entity(name, data, 144, seq_id, pos, Colors::Cyan, nullptr, nullptr, count, seq_ids);
	}
}

void load_enti_145(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x30);
	DWORD num;
	READ(f, &num, 4, &a);
	SFPC(f, num * 4);

	DWORD flags;
	READ(f, &flags, 4, &a);
	XMFLOAT3 pos;
	READ(f, &pos, 12, &a);
	char* data = (char*)malloc(256);
	ZeroMemory(data, 256);
	char* name = (char*)malloc(128);
	ZeroMemory(name, 128);

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n f", seq_id, g_curr_file, flags);
	sprintf(name, "some waypoint? (145)\n");

	Create_entity(name, data, 145, seq_id, pos, Colors::Cyan);
}

void load_enti_146(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x2C);
	DWORD num;
	READ(f, &num, 4, &a);
	SFPC(f, num * 4);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	READ(f, &pos, 12, &a);
	READ(f, &rot, 16, &a);
	SFPC(f, 10);
	DWORD num1;
	READ(f, &num1, 4, &a);
	for (int i = 0; i < num1; i++)
	{
		XMFLOAT3 pos2;
		READ(f, &pos2, 12, &a);
		SFPC(f, 16); // rotator
		g_lines.push_back({ pos, pos2, {0.0f, 0.0f, 1.0f, 1.0f} });
	}
	char* data = (char*)malloc(128);
	ZeroMemory(data, 128);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "alien hidding spot (146)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);

	Create_entity(name, data, 146, seq_id, pos, Colors::Magenta);
}

void load_enti_149(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x30);
	int num1;
	READ(f, &num1, 4, &a);
	SFPC(f, num1 * 4 + 12);
	DWORD id[3];
	READ(f, id, 12, &a);
	float f1;
	READ(f, &f1, 4, &a);


	char* data = (char*)malloc(512);
	ZeroMemory(data, 512);
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n id1: %d \n id2: %d \n id3: %d \n float: %f \n", seq_id, g_curr_file, id[0], id[1], id[2], f1);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "Entity Monster properties ? (149)\n");
	Create_global_entity(name, data, 149, seq_id);
}

void load_enti_156(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x34);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	DWORD num1;
	READ(f, &num1, 4, &a);
	SFPC(f, num1 * 4);
	READ(f, &pos, 12, &a);
	READ(f, &rot, 16, &a);
	SFPC(f, 0x29);
	DWORD hash;
	READ(f, &hash, 4, &a);


	char* data = (char*)malloc(256);
	ZeroMemory(data, 256);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "Entity Collectable (156)\n");

	

	entity_prop* prop = new entity_prop;
	ZeroMemory(prop, sizeof(entity_prop));
	prop->visible = 1;
	prop->model_hash = hash;
	prop->pos = pos;
	prop->angle = rot;

	hash_tree_node2* node = search_by_hash2(model_hashs, prop->model_hash);
	if (!node)
	{
		node = g_default_model; // Default mdl
	}
	model_info* mi = (model_info*)node->ptr;

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Model_name: %s \n", seq_id, g_curr_file, mi->mdl_name);

	Create_entity(name, data, 156, seq_id, pos, Colors::Magenta, prop);
}

void load_enti_157(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x30);
	XMFLOAT3 pos;
	DWORD flags;
	DWORD num1;
	READ(f, &num1, 4, &a);
	SFPC(f, num1 * 4);
	READ(f, &flags, 4, &a);
	READ(f, &pos, 12, &a);
	SFPC(f, 12);
	DWORD* seq_ids = (DWORD*)malloc(4);
	READ(f, seq_ids, 4, &a);
	char* data = (char*)malloc(256);
	ZeroMemory(data, 256);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "Entity Target point (157)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n", seq_id, g_curr_file, flags);

	Create_entity(name, data, 157, seq_id, pos, Colors::Magenta, nullptr, nullptr, 1, seq_ids);
}

void load_enti_158(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x4C);
	XMFLOAT3 pos;
	DWORD flags;
	DWORD num1;
	READ(f, &num1, 4, &a);
	SFPC(f, num1 * 4);
	READ(f, &flags, 4, &a);
	READ(f, &pos, 12, &a);
	SFPC(f, 12);
	DWORD* seq_ids = (DWORD*)malloc(4);
	READ(f, seq_ids, 4, &a);
	char* data = (char*)malloc(256);
	ZeroMemory(data, 256);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "Entity Mission Target (158)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Flags: %x \n", seq_id, g_curr_file, flags);

	Create_entity(name, data, 158, seq_id, pos, Colors::Purple, nullptr, nullptr, 1, seq_ids);
}

void load_enti_162(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x3C);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	DWORD num1;
	READ(f, &num1, 4, &a);
	SFPC(f, num1 * 4);
	READ(f, &pos, 12, &a);
	READ(f, &rot, 16, &a);
	SFPC(f, 25);
	XMFLOAT2 some_float_data;
	READ(f, &some_float_data, 8, &a);
	DWORD hash;
	SFPC(f, 8);
	READ(f, &hash, 4, &a);
	SFPC(f, 0xBF);
	DWORD* seq_ids = (DWORD*)malloc(4);
	READ(f, seq_ids, 4, &a);

	char* data = (char*)malloc(128);
	ZeroMemory(data, 128);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "Entity battery_station (162)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Curr_health: %f \n Max_health: %f \n", seq_id, g_curr_file, some_float_data.x, some_float_data.y);
	//XMFLOAT3 dir = { pos.x + rot.x, pos.y + rot.y, pos.z + rot.z };
	//g_lines.push_back({ pos, dir, XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f) }); // blue line
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
	SFPS(f, 0x3C);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	DWORD num1;
	READ(f, &num1, 4, &a);
	SFPC(f, num1 * 4);
	READ(f, &pos, 12, &a);
	READ(f, &rot, 16, &a);
	SFPC(f, 25);
	XMFLOAT2 some_float_data;
	READ(f, &some_float_data, 8, &a);
	DWORD hash;
	SFPC(f, 8);
	READ(f, &hash, 4, &a);
	SFPC(f, 0xBF);
	DWORD* seq_ids = (DWORD*)malloc(4);
	READ(f, seq_ids, 4, &a);

	char* data = (char*)malloc(128);
	ZeroMemory(data, 128);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "Entity ??? (163)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n Curr_health: %f \n Max_health: %f \n", seq_id, g_curr_file, some_float_data.x, some_float_data.y);
	//XMFLOAT3 dir = { pos.x + rot.x, pos.y + rot.y, pos.z + rot.z };
	//g_lines.push_back({ pos, dir, XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f) }); // blue line
	entity_prop* prop = new entity_prop;
	ZeroMemory(prop, sizeof(entity_prop));
	prop->visible = 1;
	prop->model_hash = hash;
	prop->pos = pos;
	prop->angle = rot;

	Create_entity(name, data, 163, seq_id, pos, Colors::Red, prop, nullptr, 1, seq_ids);
}

void load_enti_166(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x2C);
	XMFLOAT3 pos;
	DWORD num1;
	READ(f, &num1, 4, &a);
	SFPC(f, num1 * 4);
	READ(f, &pos, 12, &a);
	SFPC(f, 8);
	DWORD num2;
	READ(f, &num2, 4, &a);
	for (int i = 0; i < num2; i++)
	{
		SFPC(f, 8);
		asura_bbox ar;
		XMFLOAT4 rot;
		READ(f, &ar, 24, &a);
		READ(f, &rot, 16, &a);
		//g_bboxes.push_back({ {ar.x1, ar.y1, ar.z1 }, {ar.x2, ar.y2, ar.z2 }, {1.0f, 0.0f, 0.0f, 1.0f}, rot }); make for futher less messy
	}

	char* data = (char*)malloc(1024);
	ZeroMemory(data, 1024);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "Entity Zone_area (166)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);

	char* data2 = (char*)malloc(512);
	ZeroMemory(data2, 512);


	DWORD num3;
	READ(f, &num3, 4, &a);
	DWORD* seq_ids = (DWORD*)malloc(num3 * 4);
	for (int i = 0; i < num3; i++)
	{
		DWORD id1;
		DWORD id2;
		XMFLOAT3 pos_;
		DWORD id3;
	
		READ(f, &id1, 4, &a);
		READ(f, &id2, 4, &a);
		READ(f, seq_ids + i, 4, &a);
		READ(f, &pos_, 12, &a);

		//g_bboxes.push_back({ {pos_.x + 0.2f, pos_.y + 0.2f, pos_.z + 0.2f },{ pos_.x - 0.2f, pos_.y - 0.2f, pos_.z - 0.2f },{ 1.0f, 0.0f, 0.0f, 1.0f }}); // translation point ?

		READ(f, &id3, 4, &a);
		//sprintf(data2, "%d -> { %d, %d, %d } \n", i, id1, id2, id3);
		strcat(data, data2);
	}

	Create_entity(name, data, 166, seq_id, pos, Colors::Cyan, nullptr, nullptr, num3, seq_ids);
}

void load_enti_169(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x2C);
	DWORD num;
	READ(f, &num, 4, &a);
	SFPC(f, num * 4);
	DWORD* seq_ids = (DWORD*)malloc(4);
	READ(f, seq_ids, 4, &a);
	XMFLOAT3 pos;
	READ(f, &pos, 12, &a);
	char* data = (char*)malloc(256);
	ZeroMemory(data, 256);
	char* name = (char*)malloc(128);
	ZeroMemory(name, 128);

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);
	sprintf(name, "specific point (169)\n");

	Create_entity(name, data, 169, seq_id, pos, Colors::Magenta, nullptr, nullptr, 1, seq_ids);
}

void load_enti_170(HANDLE f, DWORD seq_id)
{
	DWORD a = 0;
	SFPS(f, 0x38);
	XMFLOAT3 pos;
	XMFLOAT4 rot;
	DWORD num1;
	READ(f, &num1, 4, &a);
	SFPC(f, num1 * 4);
	READ(f, &pos, 12, &a);
	SFPC(f, 0x30);
	asura_bbox bb;
	READ(f, &bb, 24, &a);
	READ(f, &rot, 16, &a);

	g_bboxes.push_back({ { bb.x1, bb.y1, bb.z1 },{ bb.x2, bb.y2, bb.z2 },{ 1.0f, 0.0f, 0.0f, 1.0f }, rot }); 

	// There so many connections followed by this CODE: Short (id) byte (id2)

	char* data = (char*)malloc(256);
	ZeroMemory(data, 256);
	char* name = (char*)malloc(64);
	ZeroMemory(name, 64);
	sprintf(name, "Entity In door area (170)\n");

	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);

	Create_entity(name, data, 170, seq_id, pos, Colors::Red);
}

void load_effect(HANDLE file, DWORD seq_id)
{
	DWORD bytes_readen = 0;
	//SetFilePointer(file, 0x34, NULL, FILE_BEGIN); // effect name HASH
	SetFilePointer(file, 0x38, NULL, FILE_BEGIN);
	XMFLOAT3 pos;
	XMFLOAT3 dir;
	ReadFile(file, &pos, sizeof(XMFLOAT3), &bytes_readen, NULL);
	SetFilePointer(file, 0x48, NULL, FILE_BEGIN);
	ReadFile(file, &dir, sizeof(XMFLOAT3), &bytes_readen, NULL);

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
	dbgprint("SMSG", "!!!!!!!!!!!!!!!!!!!!!!!!! MESSAGE LIST !!!!!!!!!!!!!!!!!!!!!!!!! \n");
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
	READ(f, &header, 24, &a);


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
		READ(f, &b1, 1, &a);
		if (b1)
		{
			SMSG_entry entry;
			READ(f, &entry, sizeof(entry), &a);
			//dbgprint("SMSG", "entry: %d %d %d (%d) -> \n",entry.dw1, entry.type, entry.dw2, entry.Count);
			void* mem = malloc(sizeof(SMSG_subentry) * entry.Count + 4);
			*(DWORD*)mem = entry.Count;
			for (int j = 0; j < entry.Count; j++)
			{
				SMSG_subentry subentry;
				READ(f, &subentry, sizeof(SMSG_subentry), &a);
				dbgprint("SMSG", "%d : {%hu, %hu -> %d} \n", i, subentry.w1, subentry.w2, subentry.seq_id);
				if (entry.type >= 6)
					SFPC(f, 4); //unk dword
				switch (subentry.w1)
				{
				case 0x7B:
					SFPC(f, 84);
					break;
				case 0x7C:
				case 0x7D:
				case 0x7E:
				case 0x7F:
					SFPC(f, 40); // 32 + 8 unk
					break;
				case 0x80:
					SFPC(f, 80);
					break;
				default:
					WORD w2 = (subentry.w2 + 3) & 0xFFFFFFFC;
					SFPC(f, w2);
					break;
				}
				memcpy((DWORD*)mem + 1 + j * 4, &subentry, sizeof(subentry));
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



void load_point_entity(HANDLE file, DWORD entity_id, DWORD seq_id)
{
	DWORD bytes_readen = 0;
	//dbgprint("Entity", "Load point entity: %d\n", entity_id);
	DWORD offset = e_info[entity_id].pos_offset;

	//deal with special cases
	DWORD offset2 = 0;
	switch (entity_id)
	{
	case 7:
		load_prop(file, seq_id);
		return;
	case 14:
		//load_enti_14(file, seq_id); // unknown invisible in game
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
	case 97:
		load_enti_97(file, seq_id); // Predator
		return;
	case 100:
		load_enti_100(file, seq_id);// Waypoint activator
		return;
	case 114:
		load_enti_114(file, seq_id);// NPC waypoints
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
		load_enti_140(file, seq_id); // 
		return;
	case 141:
		load_enti_141(file, seq_id);
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
	case 166:
		load_enti_166(file, seq_id); // Areas
		return;
	case 169:
		load_enti_169(file, seq_id);
		return;
	case 170:
		//load_enti_170(file, seq_id);// indoor area
		return;

	default:
		break;
	}

	SetFilePointer(file, offset + offset2, NULL, FILE_BEGIN);
	XMFLOAT3 pos;
	ReadFile(file, &pos, sizeof(pos), &bytes_readen, NULL);
	g_bboxes.push_back({ XMFLOAT3(pos.x + 0.2f, pos.y + 0.2f, pos.z + 0.2f),XMFLOAT3(pos.x - 0.2f, pos.y - 0.2f, pos.z - 0.2f), XMFLOAT4(0.0f,1.0f,1.0f,1.0f) }); // small cyan bbox

	return;
}

void load_area_entity(HANDLE file, DWORD entity_id)
{
	DWORD bytes_readen = 0;
	//dbgprint("Entity", "Load point entity: %d\n", entity_id);
	DWORD offset = e_info[entity_id].pos_offset;
	DWORD offset2 = e_info[entity_id].pos2_offset;
	//deal with special cases
	DWORD offset3 = 0;
	switch (entity_id)
	{
	
	default:
		break;
	}

	SetFilePointer(file, offset + offset3, NULL, FILE_BEGIN);
	XMFLOAT3 pos;
	ReadFile(file, &pos, sizeof(pos), &bytes_readen, NULL);
	SetFilePointer(file, offset2 + offset3, NULL, FILE_BEGIN);
	XMFLOAT3 pos2;
	ReadFile(file, &pos2, sizeof(pos2), &bytes_readen, NULL);

	g_bboxes.push_back({ pos, pos2, XMFLOAT4(1.0f,1.0f,1.0f,1.0f) }); // small cyan bbox

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
	char* data  = (char*)malloc(128);
	ZeroMemory(data, 128);
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, file_name);
	char* name = (char*)malloc(24);
	ZeroMemory(name, 24);
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
		load_enti_1(file, seq_id);
		return;
	case 3:
		load_enti_3(file, seq_id);
		return;
	case 11:
		load_enti_11(file, seq_id);
		return;
	case 32:
		load_enti_32(file, seq_id);
		return;
	case 34:
		load_enti_34(file, seq_id);
		return;
	case 68:
		load_enti_68(file, seq_id);
		return;
	case 74:
		load_enti_74(file, seq_id);
		return;
	case 130:
		load_enti_130(file, seq_id);
		return;
	case 133:
		load_enti_133(file, seq_id);
		return;
	case 149:
		load_enti_149(file, seq_id);
		return;
	default:
		break;
	}

	char* data = (char*)malloc(128);
	ZeroMemory(data, 128);
	sprintf(data, "SEQ_ID: %d \n FILE: %ws \n", seq_id, g_curr_file);
	char* name = (char*)malloc(24);
	ZeroMemory(name, 24);
	sprintf(name, "Global Entity %d \n", entity_id);
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
	DWORD bytes_readen = 0;
	HANDLE file = CreateFileW(file_path, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	ENTI_header file_header;
	ReadFile(file, &file_header, 24, &bytes_readen, NULL);
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
	if (ent_id > 173 || ent_id <= 0)
	{
		dbgprint("Entity", "Ent_id > 173 or <= 0: %ws \n", file_name);
		return;
	}
	ENTITY_TYPE type = e_info[ent_id].type;

	switch (type)
	{
	case ENTITY_NONE:
	case ENTITY_UNKNOWN:
		load_default_entity(ent_id, file_header.seq_id, file_name);
		break;
	case ENTITY_GLOBAL:
		load_global_entity(file, ent_id, file_header.seq_id);
		break;
	case ENTITY_AREA: // actualy not used yet
		load_area_entity(file, ent_id);
		break;
	case ENTITY_POINT:
		load_point_entity(file, ent_id, file_header.seq_id);
		break;
	default:
		dbgprint("Entity", "Error in switch case condition: type = %d \n", type);
	}

	CloseHandle(file);
	return;
}

