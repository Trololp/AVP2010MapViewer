#include "Hash_tree.h"
#include "Debug.h"

list_node mat_nodes[1023];
hash_tree_node* hash_tree_texture;
hash_tree_node2* model_hashs;

//I just paste it from decompiler
DWORD hash_from_str(DWORD init, char* str) 
{
	DWORD result; // eax@1
	char *v3; // esi@1
	char i; // cl@2
	int v5; // edx@7
	int v6; // eax@7

	result = init;
	v3 = str;
	if (str)
	{
		for (i = *str; i; result = v5 + v6)
		{
			if ((i - 'A') > 0x19u)
			{
				if (i == '\\')
					i = '/';
			}
			else
			{
				i += 32;
			}
			v5 = 31 * result;
			v6 = i;
			i = (v3++)[1];
		}
	}
	return result;
}

void add_hash(hash_tree_node* h_n, hash_tree_node* new_node) // its_texture 1 = texture 0 = mat
{
	if (!new_node->Hash)
		return;
	if (!h_n->Hash)
	{
		h_n->Hash = new_node->Hash;
		h_n->texture = new_node->texture;
		return;
	}
	if (h_n->Hash == new_node->Hash)
		return;
	if (h_n->Hash > new_node->Hash)
	{
		if (!h_n->smaller)
		{
			h_n->smaller = new_node;
			return;
		}
		else
		{
			add_hash(h_n->smaller, new_node);
		}
	}
	else
	{
		if (!h_n->bigger)
		{
			h_n->bigger = new_node;
			return;
		}
		else
		{
			add_hash(h_n->bigger, new_node);
		}
	}
}

void add_hash2(hash_tree_node2* h_n, hash_tree_node2* new_node) 
{
	if (!new_node->hash)
		return;
	if (!h_n->hash)
	{
		h_n->hash = new_node->hash;
		h_n->ptr = new_node->ptr;
		return;
	}
	if (h_n->hash == new_node->hash)
		return;
	if (h_n->hash > new_node->hash)
	{
		if (!h_n->smaller)
		{
			h_n->smaller = new_node;
			return;
		}
		else
		{
			add_hash2(h_n->smaller, new_node);
		}
	}
	else
	{
		if (!h_n->bigger)
		{
			h_n->bigger = new_node;
			return;
		}
		else
		{
			add_hash2(h_n->bigger, new_node);
		}
	}
}

hash_tree_node* search_by_hash(hash_tree_node* h_n, DWORD hash)
{
	if (!hash)
		return 0;
	hash_tree_node* hn = h_n;
	while (hn)
	{
		if (hn->Hash >= hash)
		{
			if (hn->Hash == hash)
				return hn;
			hn = hn->smaller;
		}
		else
		{
			hn = hn->bigger;
		}
	}
	return 0;
}

int branch = 0;
void dump_hash_tree(hash_tree_node* h_n)
{
	dbgprint("TREE", "(%d) Hash: %x bigger: %x smaller: %x data: %x \n", branch++, h_n->Hash, h_n->bigger, h_n->smaller, h_n->texture);
	if (h_n->bigger)
		dump_hash_tree(h_n->bigger);
	if (h_n->smaller)
		dump_hash_tree(h_n->smaller);
}

hash_tree_node2* search_by_hash2(hash_tree_node2* h_n, DWORD hash)
{
	if (!hash)
		return 0;
	hash_tree_node2* hn = h_n;
	while (hn)
	{
		if (hn->hash >= hash)
		{
			if (hn->hash == hash)
				return hn;
			hn = hn->smaller;
		}
		else
		{
			hn = hn->bigger;
		}
	}
	return 0;
}

void delete_nodes(hash_tree_node* hn)
{
	if (hn)
	{
		if (hn->bigger)
			delete_nodes(hn->bigger);
		if (hn->smaller)
			delete_nodes(hn->smaller);
		delete hn;
	}
}
void delete_nodes2(hash_tree_node2* hn)
{
	if (hn)
	{
		if (hn->bigger)
			delete_nodes2(hn->bigger);
		if (hn->smaller)
			delete_nodes2(hn->smaller);
		delete hn;
	}
}

list_node* search_mat_by_hash(DWORD hash)
{
	if (!hash)
		return 0;
	if (mat_nodes[1023 & hash].hash == hash)
		return &(mat_nodes[1023 & hash]);
	for (list_node* next_node = mat_nodes[1023 & hash].next; next_node; next_node = next_node->next)
	{
		if (next_node->hash == hash)
			return next_node;
	}
	return 0;
}

void add_hash_and_mat(Material* mat, DWORD hash)
{
	if (!hash)
		return;
	if (!(mat_nodes[1023 & hash].hash))
	{
		mat_nodes[1023 & hash].hash = hash;
		mat_nodes[1023 & hash].material = mat;
		list_node* node = new list_node;
		mat_nodes[1023 & hash].next = node;
	}
	else
	{
		if (mat_nodes[1023 & hash].hash == hash)
			return;
		for (list_node* next_node = mat_nodes[1023 & hash].next; next_node; next_node = next_node->next)
		{
			if (next_node->hash == hash)
				break;
			if (!(next_node->hash))
			{
				next_node->hash = hash;
				next_node->material = mat;
				list_node* node = new list_node;
				next_node->next = node;
				break;
			}
		}
	}

}

void delete_mat_list()
{
	for (int i = 0; i < 1023; i++)
	{
		if (Material* mat = mat_nodes[i].material)
		{
			delete(mat);
		}
		if (list_node* node = mat_nodes[i].next)
		{
			list_node* next_node = 0;
			do
			{
				next_node = node->next;
				delete(node->material);
				delete(node);
				if (next_node)
					node = next_node;
			} while (next_node);
		}
	}
}

