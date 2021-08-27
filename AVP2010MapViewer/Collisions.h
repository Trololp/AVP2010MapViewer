#pragma once
// This one is for handling render and storage for 
// Collision information. That can be found inside EMOD.

#include "AVP2010MapViewer.h"

enum Collision_box_types {
	COLLISION_BOX_REND,
	COLLISION_BOX_COLL,
	COLLISION_BOX_WATER,
	COLLISION_BOX_UNK,
	COLLISION_BOX_PROP,
	COLLISION_BOX_UNDEFINED = -1
};

struct Collision_Box
{
	XMFLOAT3 pos;
	ID3D11Buffer* vtx_buf;
	Collision_box_types cb_type;
	DWORD emod_id;
	float radius;
	//bbox bbox; //orig bbox
};



int Global_index_buffer_for_boxes(ID3D11Buffer** Index_buffer);
int Collisions_to_box(bbox* bbox, ID3D11Buffer** vtx_buffer);

class Collisions
{
public:
	bool m_displayable = false;
	bool m_display_specific_type = false;
	bool m_draw_by_emod_id = false;
	DWORD m_emod_id;
	Collision_box_types m_display_this_type;
	std::vector<Collision_Box> m_boxes;


	Collisions();
	~Collisions();
	void Select_specific_type(Collision_box_types type) { 
		m_display_specific_type = true;
		m_display_this_type = type;
	};
	void Select_specific_emod(DWORD emod_id)
	{
		m_draw_by_emod_id = true;
		m_emod_id = emod_id;
	}

	void Draw();

private:
	ID3D11Buffer* m_global_index_buffer;
};

