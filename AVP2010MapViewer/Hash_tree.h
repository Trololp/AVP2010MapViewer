#pragma once
#include "AVP2010MapViewer.h"

struct hash_tree_node
{
	DWORD Hash = 0;
	avp_texture* texture = nullptr;
	hash_tree_node* bigger = nullptr;
	hash_tree_node* smaller = nullptr;
};

struct list_node
{
	DWORD hash = 0;
	Material* material = nullptr;
	list_node* next = nullptr;
};

struct hash_tree_node2
{
	DWORD hash = 0;
	void* ptr = nullptr;
	hash_tree_node2* bigger = nullptr;
	hash_tree_node2* smaller = nullptr;
};

DWORD hash_from_str(DWORD init, char* str);
void add_hash(hash_tree_node* h_n, hash_tree_node* new_node);
void add_hash2(hash_tree_node2* h_n, hash_tree_node2* new_node);
hash_tree_node* search_by_hash(hash_tree_node* h_n, DWORD hash);
hash_tree_node2* search_by_hash2(hash_tree_node2* h_n, DWORD hash);
void delete_nodes(hash_tree_node* hn);
void delete_nodes2(hash_tree_node2* hn);
list_node* search_mat_by_hash(DWORD hash);
void add_hash_and_mat(Material* mat, DWORD hash);
void delete_mat_list();
void dump_hash_tree(hash_tree_node* n_h);