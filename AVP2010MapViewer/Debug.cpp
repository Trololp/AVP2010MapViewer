#include "Debug.h"

extern ID3D11Device*           g_pd3dDevice;
extern ID3D11Buffer*		   g_pVertexBuffer_stuff;
extern ID3D11Buffer*			g_pVertexBuffer_lines;

void dbgprint(const char* debug_note ,const char* fmt, ...)
{
	wchar_t* cwd = _wgetcwd(0, 0);
	wchar_t file_path[260];
	wsprintf(file_path, L"%ws\\debug.txt", cwd);
	FILE* file = _wfopen(file_path, L"a+");
	if (!file)
	{
		MessageBox(nullptr, L"Cannot open debug log !", L"Error", MB_OK);
		return;
	}
	SYSTEMTIME st;
	GetLocalTime(&st);
	fprintf(file, "%02d.%02d.%d %02d:%02d (%s) | ", st.wDay, st.wMonth, 
		st.wYear, st.wHour, st.wMinute, debug_note);

	va_list args;
	va_start(args, fmt);

	vfprintf(file, fmt, args);

	va_end(args);
	fclose(file);
}

int bbox_to_vertex(std::vector <bbox> &bboxes, DWORD* g_points)
{
	HRESULT hr = S_OK;


	int bboxes_count = bboxes.size();
	int size_of_vertex_data = 12 * 2 * sizeof(simple_vertex) * bboxes_count;
	simple_vertex* vertices = (simple_vertex*)malloc(size_of_vertex_data);
	int r = 0;
	*g_points = size_of_vertex_data;

	dbgprint("Helpers", "Bbox: count: %d\n", bboxes_count);

	for (std::vector<bbox>::iterator it = bboxes.begin(); it != bboxes.end(); ++it)
	{
		XMFLOAT4 clr = (*it).Color;
		XMFLOAT3 p1 = (*it).p1;
		XMFLOAT3 p2 = (*it).p2;
		XMVECTOR p1_ = XMLoadFloat3(&p1);
		XMVECTOR p2_ = XMLoadFloat3(&p2);
		XMMATRIX Q = XMMatrixRotationQuaternion(XMLoadFloat4(&((*it).rot)));
		XMVECTOR p = (p1_ + p2_) / 2;
		XMFLOAT3 a = p1;
		XMFLOAT3 b = XMFLOAT3(p2.x, p1.y, p1.z);
		XMFLOAT3 c = XMFLOAT3(p2.x, p2.y, p1.z);
		XMFLOAT3 d = XMFLOAT3(p1.x, p2.y, p1.z);
		XMFLOAT3 e = XMFLOAT3(p1.x, p1.y, p2.z);
		XMFLOAT3 f = XMFLOAT3(p2.x, p1.y, p2.z);
		XMFLOAT3 g = p2;
		XMFLOAT3 h = XMFLOAT3(p1.x, p2.y, p2.z);

		vertices[0 + r] = { a, clr };  vertices[12 + r] = { g, clr };//
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
	}


	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = size_of_vertex_data;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertices;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer_stuff);
	if (FAILED(hr))
	{
		dbgprint("Helpers", "Bbox CreateBuffer failed ! \n");
		return 0;
	}
	free(vertices);
	return 1;
}

int line_to_vertex(std::vector<line> &lines, DWORD* g_points)
{
	HRESULT hr = S_OK;


	int lines_count = lines.size();
	int size_of_vertex_data = 2 * sizeof(simple_vertex) * lines_count;
	simple_vertex* vertices = (simple_vertex*)malloc(size_of_vertex_data);
	int r = 0;
	*g_points = size_of_vertex_data;

	dbgprint("Helpers", "Lines: count: %d\n", lines_count);

	for (std::vector<line>::iterator it = lines.begin(); it != lines.end(); ++it)
	{
		XMFLOAT4 clr = (*it).Color;
		XMFLOAT3 p1 = (*it).p1;
		XMFLOAT3 p2 = (*it).p2;

		vertices[0 + r] = { p1, clr }; // (p1) ---- (p2)
		vertices[1 + r] = { p2, clr }; //
		r += 2;
	}


	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = size_of_vertex_data;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertices;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer_lines);
	if (FAILED(hr))
	{
		dbgprint("Helpers", "Lines CreateBuffer failed ! \n");
		return 0;
	}
	free(vertices);
	return 1;
}

void Dump_matrix(const char* name, XMMATRIX matrix)
{
	if (!name)
	{
		dbgprint("Debug", "Matrix dump: \n");
	}
	else
	{
		dbgprint("Debug", "Matrix dump %s: \n", name);
	}
	for (int r = 0; r < 4; r++)
	{
		dbgprint("Debug", " %f %f %f %f \n", matrix.r[r].m128_f32[0], matrix.r[r].m128_f32[1], matrix.r[r].m128_f32[2], matrix.r[r].m128_f32[3]);
	}
}

void Dump_vector(const char* name, XMVECTOR vector)
{
	if (!name)
	{
		dbgprint("Debug", "Vector dump: %f %f %f %f \n", vector.m128_f32[0], vector.m128_f32[1], vector.m128_f32[2], vector.m128_f32[3]);
	}
	else
	{
		dbgprint("Debug", "Vector dump %s: %f %f %f %f \n", name, vector.m128_f32[0], vector.m128_f32[1], vector.m128_f32[2], vector.m128_f32[3]);
	}
}