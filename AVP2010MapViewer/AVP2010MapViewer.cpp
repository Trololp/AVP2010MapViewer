// AVP2010MapViewer made by Trololp in 2020.

#include "AVP2010MapViewer.h"
#include "Debug.h"
#include "Keyboard.h"
#include "Hash_tree.h"
#include "Textures.h"
#include "Load_map.h"
#include "Entity_loader.h"
#include "Models.h"
#include "DDSLoader.h"
#include "ViewFrustrum.h"
#include <SpriteFont.h>
#include <ShObjIdl.h>
#include "Console.h"
#include "EMOD_load.h"
#include "Collisions.h"
#include "PHEN_loader.h"

#pragma comment(lib, "d3d11.lib")

using namespace DirectX;


float ScreenNear = 0.1f;
float ScreenFar = 500.0f;
float vis_radius = 600.0f;
int screen_width = GetSystemMetrics(SM_CXSCREEN);
int screen_height = GetSystemMetrics(SM_CYSCREEN);
//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
wchar_t* g_path1 = nullptr; // path for [Map_name]/[Map_name] folder
wchar_t* g_path2 = nullptr; // path for [Map_name]/[Mission_name] folder
wchar_t* g_cwd; // current working directory
avp_texture AVP_TEXTURES[1500];
ID3D11Device*           g_pd3dDevice = nullptr;
ID3D11DeviceContext*    g_pImmediateContext = nullptr;
ID3D11Buffer*           g_pVertexBuffer_env = nullptr; // Enviroment
ID3D11Buffer*           g_pIndexBuffer_env = nullptr; // Enviroment
ID3D11Buffer*		   g_pVertexBuffer_stuff = nullptr; // generic bboxes
ID3D11Buffer*			g_pVertexBuffer_lines = nullptr;
ID3D11Buffer*			g_dynamic_buffer_lines = nullptr; // Entities bboxes and stuff
ID3D11Buffer*			g_pVertexBuffer_emod_bbox = nullptr; // EMOD

Map_mesh_info*			g_mesh_info_structs = nullptr;
std::vector<entity_prop> g_props;
std::vector<entity>		g_entities;

HINSTANCE               g_hInst = nullptr;
HWND                    g_hWnd = nullptr;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_REFERENCE;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device1*          g_pd3dDevice1 = nullptr;
ID3D11DeviceContext1*   g_pImmediateContext1 = nullptr;
IDXGISwapChain*         g_pSwapChain = nullptr;
IDXGISwapChain1*        g_pSwapChain1 = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
ID3D11VertexShader*     g_pVertexShader = nullptr;
ID3D11PixelShader*      g_pPixelShader = nullptr;
ID3D11InputLayout*      g_pVertexLayout = nullptr;
ID3D11VertexShader*     g_pVertexShader_s = nullptr;
ID3D11PixelShader*      g_pPixelShader_s = nullptr;
ID3D11InputLayout*      g_pVertexLayout_s = nullptr;
ID3D11VertexShader*     g_pVertexShader_m = nullptr;
ID3D11PixelShader*      g_pPixelShader_m = nullptr;
ID3D11InputLayout*      g_pVertexLayout_m = nullptr;
ID3D11Buffer*           g_pConstantBuffer = nullptr;
ID3D11RasterizerState*  g_Env_RS = nullptr;
ID3D11RasterizerState*  g_Wireframe_RS = nullptr;
ID3D11DepthStencilState*	g_pDSState = nullptr;
ID3D11DepthStencilView* g_pDSV = nullptr;
ID3D11Texture2D*		g_pDepthStencilBuffer = nullptr;
ID3D11ShaderResourceView* g_pDefaultTexture = nullptr;
ID3D11SamplerState*     g_pSS = nullptr;
ID3D11BlendState*		g_pBlendStateNoBlend = nullptr;
Console*				g_pConsole = nullptr;
Collisions*				g_pCollisions = nullptr;

XMMATRIX                g_World;
XMMATRIX                g_View;
XMMATRIX                g_Projection;
XMMATRIX				g_OrthoMatrix;
float					pos_x = -5.0f;
float					pos_y = 0.0f;
float					pos_z = 0.0f;
float					g_alpha = 0.0f;
float					g_beta = 0.0f;
int						check_x;
int						check_y; // Check variables for selecting entity bbox by mouse input
bool					LMB_check = 0;
DWORD					g_selected_entity_id = 0;
DWORD					g_points_stuff;
DWORD					g_points_lines;
DWORD					g_total_meshes = 0;
DWORD					g_points_emod_bbox = 0;
std::vector <bbox>		g_bboxes;
std::vector <line>		g_lines;

ViewFrustum				g_frustum;
UINT8*					g_mesh_vis;
int						g_last_x = 0;
int						g_last_y = 0;
int						g_framecount = 0;
std::unique_ptr<DirectX::SpriteFont> m_font;
XMVECTOR				m_fontPos = {300.0f, 50.0f};
XMVECTOR				g_DataFontPos = { screen_width - 200.0f, screen_height/2 };
std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
char					g_pos_string[100] = { 0 };
hash_tree_node2*		g_default_model;
char					g_prop_info[250] = { 0 };

// Test map models
unsigned int			g_test_map_model_id;
bool					g_test_map_model_enabled = false;


extern hash_tree_node* hash_tree_texture;
extern hash_tree_node2* model_hashs;
extern bool				b_show_emod;
extern bool				g_emod_loaded;
extern unsigned int     g_emod_count;
extern EMOD_entry* g_emod_entrys;

extern bool b_show_collision;


//Console Trash here
VIEWER_CONSOLE_ARG cmd_tp_argschema[4] = {
	CONARG_FLOAT,
	CONARG_FLOAT,
	CONARG_FLOAT,
	CONARG_END };
Command cmd_tp = {
	"tp",
	CONCMD_RET_ZERO,
	CONARG_MULTIPLE,
	Console_command_tp,
	cmd_tp_argschema
};
extern Command cmd_find_entity;
extern Command cmd_dump_smsg;
extern Command cmd_dump_emod;
extern Command cmd_test_map_mesh;
extern Command cmd_print_list_map_meshs;
extern Command cmd_dump_map;
extern Command cmd_collision_emod_id;
//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(InitWindow(hInstance, nCmdShow)))
		return 0;

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
		COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileOpenDialog *pFileOpen;
		
		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

		if (SUCCEEDED(hr))
		{
			
			for (int i = 0; i < 2; i++)
			{
				FILEOPENDIALOGOPTIONS* pfos = new FILEOPENDIALOGOPTIONS;
				ZeroMemory(pfos, sizeof(pfos));
				pFileOpen->GetOptions(pfos);
				*pfos = *pfos |= FOS_PICKFOLDERS;
				pFileOpen->SetOptions(*pfos);

				
				if (i == 0)
					pFileOpen->SetTitle(L"Select folder with map (example \"Ruins\")");
				if (i == 1)
					pFileOpen->SetTitle(L"Select folder with mission or not select (example \"A03_Ruins\")");

				// Show the Open dialog box.
				hr = pFileOpen->Show(NULL);

				// Get the file name from the dialog box.
				if (SUCCEEDED(hr))
				{
					IShellItem *pItem;
					hr = pFileOpen->GetResult(&pItem);
					if (SUCCEEDED(hr))
					{
						if(i == 0)
							hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &g_path1);
						if(i == 1)
							hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &g_path2);
						if (FAILED(hr))
						{
							dbgprint("wWinMain", "failed to choose folder %d with error hr = %x \n", i, hr);
							MessageBoxW(NULL, L"Bad folder or else ...", L"File Path", MB_OK);
						}
						pItem->Release();
					}
				}
			}
			
			pFileOpen->Release();
		}
		CoUninitialize();
	}

	if (FAILED(InitDevice()))
	{
		CleanupDevice();
		return 0;
	}

	// Main message loop
	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			MovementFunc();
			Render();
		}
	}

	CleanupDevice();

	return (int)msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;//LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"TutorialWindowClass";
	wcex.hIconSm = NULL;//LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	// Create window
	g_hInst = hInstance;
	RECT rc = { 0, 0, screen_width, screen_height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(L"TutorialWindowClass", L"AVP 2010 VIEWER",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
		nullptr);
	if (!g_hWnd)
		return E_FAIL;

	ShowWindow(g_hWnd, nCmdShow);

	return S_OK;
}


HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;

	// Disable optimizations to further improve shader debugging
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			dbgprint("ShaderCompiler", "Error: %s\n", reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			pErrorBlob->Release();
		}
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
	dbgprint("Init", "Executable Started !\n");

	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd = {0};
	
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = g_hWnd;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Windowed = TRUE;
	sd.Flags = 0;


	if (D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, 0, featureLevels, 3, D3D11_SDK_VERSION,
		&sd, &g_pSwapChain, &g_pd3dDevice, 0, &g_pImmediateContext) < 0
		|| !g_pd3dDevice
		|| !g_pImmediateContext
		|| !g_pSwapChain)
	{
		CleanupDevice(); // can be bugged
		dbgprint("Init", "Failed to initialise D3D11.\n");
		MessageBoxA(g_hWnd, "Failed to initialise D3D11.", "Viewer", 0x10u);
		exit(1);
	}
	

	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	IDXGIFactory1* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		hr = g_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}
	if (FAILED(hr))
		return hr;

	
	// Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
	//dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);

	dxgiFactory->Release();

	if (FAILED(hr))
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hr))
		return hr;

	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

	

	D3D11_TEXTURE2D_DESC DSBuffdesc;
	ZeroMemory(&DSBuffdesc, sizeof(DSBuffdesc));
	DSBuffdesc.Width = width;
	DSBuffdesc.Height = height;
	DSBuffdesc.MipLevels = 1;
	DSBuffdesc.ArraySize = 1;
	DSBuffdesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DSBuffdesc.SampleDesc.Count = 1;
	DSBuffdesc.SampleDesc.Quality = 0;
	DSBuffdesc.Usage = D3D11_USAGE_DEFAULT;
	DSBuffdesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	DSBuffdesc.CPUAccessFlags = 0;
	DSBuffdesc.MiscFlags = 0;

	hr = g_pd3dDevice->CreateTexture2D(&DSBuffdesc, NULL, &g_pDepthStencilBuffer);
	if (FAILED(hr))
	{
		dbgprint("Init", "Error create depth buffer %x\n", hr);
		return hr;
	}

	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	hr = CompileShaderFromFile(L"Shaders.fx", "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		dbgprint("Init", "VS compile failed %x \n", hr);
		return hr;
	}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader);
	if (FAILED(hr))
	{
		dbgprint("Init", "VS create failed %x \n", hr);
		pVSBlob->Release();
		return hr;
	}
	
	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R16G16B16A16_SNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TANGENT", 0, DXGI_FORMAT_R16G16B16A16_SNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 1, DXGI_FORMAT_R16G16B16A16_SNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pVertexLayout);
	pVSBlob->Release();
	if (FAILED(hr))
	{
		dbgprint("Init", "CreateInputLayout failed : %d\n", hr);
		return hr;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
	hr = CompileShaderFromFile(L"Shaders.fx", "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		dbgprint("Init", "PS create failed %x \n", hr);
		return hr;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;

	// Compile the vertex shader
	ID3DBlob* pVSBlob2 = nullptr;
	hr = CompileShaderFromFile(L"Shaders_line.fx", "VS", "vs_4_0", &pVSBlob2);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		dbgprint("Init", "VS (line) compile failed %x \n", hr);
		return hr;
	}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob2->GetBufferPointer(), pVSBlob2->GetBufferSize(), nullptr, &g_pVertexShader_s);
	if (FAILED(hr))
	{
		dbgprint("Init", "VS (line) create failed %x \n", hr);
		pVSBlob2->Release();
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout2[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements2 = ARRAYSIZE(layout2);

	// Create the input layout
	hr = g_pd3dDevice->CreateInputLayout(layout2, numElements2, pVSBlob2->GetBufferPointer(),
		pVSBlob2->GetBufferSize(), &g_pVertexLayout_s);
	pVSBlob2->Release();
	if (FAILED(hr))
	{
		dbgprint("Init", "CreateInputLayout (line) failed : %d\n", hr);
		return hr;
	}

	// Compile the model shader
	ID3DBlob* pPSBlob2 = nullptr;
	hr = CompileShaderFromFile(L"Shaders_line.fx", "PS", "ps_4_0", &pPSBlob2);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		dbgprint("Init", "PS (line) create failed %x \n", hr);
		return hr;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob2->GetBufferPointer(), pPSBlob2->GetBufferSize(), nullptr, &g_pPixelShader_s);
	pPSBlob2->Release();
	if (FAILED(hr))
		return hr;

	// Compile the vertex shader
	ID3DBlob* pVSBlob3 = nullptr;
	hr = CompileShaderFromFile(L"Shaders_model.fx", "VS", "vs_4_0", &pVSBlob3);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		dbgprint("Init", "VS (model) compile failed %x \n", hr);
		return hr;
	}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob3->GetBufferPointer(), pVSBlob3->GetBufferSize(), nullptr, &g_pVertexShader_m);
	if (FAILED(hr))
	{
		dbgprint("Init", "VS (model) create failed %x \n", hr);
		pVSBlob3->Release();
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout3[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R16G16B16A16_SNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R16G16B16A16_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R16G16B16A16_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R16G16B16A16_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R16G16B16A16_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements3 = ARRAYSIZE(layout3);

	// Create the input layout
	hr = g_pd3dDevice->CreateInputLayout(layout3, numElements3, pVSBlob3->GetBufferPointer(),
		pVSBlob3->GetBufferSize(), &g_pVertexLayout_m);
	pVSBlob3->Release();
	if (FAILED(hr))
	{
		dbgprint("Init", "CreateInputLayout (model) failed : %d\n", hr);
		return hr;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob3 = nullptr;
	hr = CompileShaderFromFile(L"Shaders_model.fx", "PS", "ps_4_0", &pPSBlob3);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		dbgprint("Init", "PS (model) create failed %x \n", hr);
		return hr;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob3->GetBufferPointer(), pPSBlob3->GetBufferSize(), nullptr, &g_pPixelShader_m);
	pPSBlob3->Release();
	if (FAILED(hr))
		return hr;

	D3D11_DEPTH_STENCIL_DESC DSDesc;
	DSDesc.DepthEnable = TRUE;
	DSDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DSDesc.DepthFunc = D3D11_COMPARISON_LESS;
	DSDesc.StencilEnable = TRUE;
	DSDesc.StencilReadMask = 0xFF;
	DSDesc.StencilWriteMask = 0xFF;
	DSDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	DSDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	DSDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	DSDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	DSDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	DSDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	DSDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	DSDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	
	hr = g_pd3dDevice->CreateDepthStencilState(&DSDesc, &g_pDSState);
	if (FAILED(hr))
	{
		dbgprint("Init", "CreateDepthStencilState failed : %d\n", hr);
		return hr;
	}

	g_pImmediateContext->OMSetDepthStencilState(g_pDSState, 1);
	D3D11_DEPTH_STENCIL_VIEW_DESC DSVdesc;
	ZeroMemory(&DSVdesc, sizeof(DSVdesc));

	DSVdesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DSVdesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DSVdesc.Texture2D.MipSlice = 0;

	hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencilBuffer, &DSVdesc, &g_pDSV);
	if (FAILED(hr))
	{
		dbgprint("Init", "CreateDepthStencilView failed : %d\n", hr);
		return hr;
	}

	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDSV);

	// Set the input layout
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

	D3D11_RASTERIZER_DESC RDesc;
	ZeroMemory(&RDesc, sizeof(RDesc));

	RDesc.FillMode = D3D11_FILL_SOLID;
	RDesc.CullMode = D3D11_CULL_BACK;
	RDesc.DepthBias = 0;
	RDesc.DepthBiasClamp = 0;
	RDesc.SlopeScaledDepthBias = 0;
	RDesc.DepthClipEnable = 1;
	RDesc.ScissorEnable = 0;
	RDesc.MultisampleEnable = 0;
	RDesc.AntialiasedLineEnable = 0;
	RDesc.FrontCounterClockwise = 1;
	hr = g_pd3dDevice->CreateRasterizerState(&RDesc, &g_Env_RS);
	if (FAILED(hr))
	{
		dbgprint("Init", "CreateRasterizerState failed : %d\n", hr);
		return hr;
	}
	g_pImmediateContext->RSSetState(g_Env_RS);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp);

	D3D11_BLEND_DESC BlendState;
	ZeroMemory(&BlendState, sizeof(D3D11_BLEND_DESC));
	BlendState.RenderTarget[0].BlendEnable = FALSE;
	BlendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = g_pd3dDevice->CreateBlendState(&BlendState, &g_pBlendStateNoBlend);
	if (FAILED(hr))
	{
		dbgprint("Init", "CreateBlendState Failed %d\n", hr);
	}


	g_bboxes.push_back({ XMFLOAT3(1.0f,1.0f,1.0f),XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f,1.0f,1.0f,1.0f)});
	
	if (!g_path1)
	{
		dbgprint("Init", "No path was chosen\n");
		exit(0);
	}

	g_cwd = _wgetcwd(0, 0);
	wchar_t Resource_str[260] = { 0 };


	lstrcpyW(Resource_str, g_path1);
	lstrcatW(Resource_str, L"\\graphics\\");

	hash_tree_texture = new hash_tree_node;
	hash_tree_texture->Hash = 0;
	hash_tree_texture->bigger = nullptr;
	hash_tree_texture->smaller = nullptr;
	hash_tree_texture->texture = nullptr;

	if (!Read_textures(Resource_str, 0))
	{
		dbgprint("Init", "Read Textures 1 Failed\n");
	}

	lstrcpyW(Resource_str, g_path1);
	lstrcatW(Resource_str, L"\\MARE_chunk\\*.MARE");

	if (!Read_materials(Resource_str, 0))
	{
		dbgprint("Init", "Read Materials Failed\n");
	}

	lstrcpyW(Resource_str, g_path1);
	lstrcatW(Resource_str, L"\\EMOD_chunk\\*.EMOD");

	if (!read_EMOD_chunk(Resource_str))
	{
		dbgprint("Init", "Read EMOD failed \n");
	}
	else
	{
		g_pCollisions = new Collisions();
	}

	lstrcpyW(Resource_str, g_path1);
	lstrcatW(Resource_str, L"\\PHEN_chunk\\*.PHEN");

	if (!read_PHEN_chunk(Resource_str))
	{
		dbgprint("Init", "Read PHEN failed \n");
	}

	lstrcpyW(Resource_str, g_path1);
	lstrcatW(Resource_str, L"\\Env.RSCF");

	if (!Read_map(Resource_str, &g_total_meshes))
	{
		dbgprint("Init", "Read Map Failed\n");
		exit(0);
	}
	

	model_hashs = new hash_tree_node2;

	if (g_path2)
	{
		lstrcpyW(Resource_str, g_path2);
		lstrcatW(Resource_str, L"\\graphics\\");

		if (!Read_textures(Resource_str, 1))
		{
			dbgprint("Init", "Read Textures 2 Failed\n");
		}

		lstrcpyW(Resource_str, g_path2);
		lstrcatW(Resource_str, L"\\MARE_chunk\\*.MARE");

		if (!Read_materials(Resource_str, 1))
		{
			dbgprint("Init", "Read Materials 2 Failed\n");
		}

		lstrcpyW(Resource_str, g_path2);
		lstrcatW(Resource_str, L"\\*.RSCF");

		if (!Read_models(Resource_str))
		{
			dbgprint("Init", "Read Models failed \n");
		}

		lstrcpyW(Resource_str, g_path2);
		lstrcatW(Resource_str, L"\\SMSG_chunk\\*.SMSG");

		if (!load_smsg(Resource_str))
		{
			dbgprint("Init", "Read SMSG failed \n");
		}

		if (!load_actor_names())
		{
			dbgprint("Init", "Load actor names failed \n");
		}
	}
	
	wsprintf(Resource_str, L"%ws\\default.RSCF", g_cwd);

	Read_model(Resource_str, 1);
	g_default_model = search_by_hash2(model_hashs, 0x18405); // Default mdl (is a cup model)

	if (!g_default_model)
	{
		dbgprint("Init", "Default model not loaded \n");
		exit(0);
	}

	if (g_path2)
	{
		lstrcpyW(Resource_str, g_path2);
		lstrcatW(Resource_str, L"\\ENTI_chunk\\*.ENTI");

		if (!load_enti(Resource_str))
		{
			dbgprint("Init", "Read ENTI failed \n");
		}
	}

	wsprintf(Resource_str, L"%ws\\default.dds", g_cwd);

	CreateDDSTextureFromFile(g_pd3dDevice, Resource_str, nullptr, &g_pDefaultTexture);
	if (!g_pDefaultTexture)
	{
		dbgprint("Init", "Default texture not loaded \n");
		exit(0);
	}

	g_mesh_vis = (UINT8*)_aligned_malloc(g_total_meshes, 16);
	ZeroMemory(g_mesh_vis, g_total_meshes);
	
	if (!bbox_to_vertex(g_bboxes, &g_points_stuff))
	{
		dbgprint("Init", "BBoxes failed !\n");
	}// bboxes

	if (!line_to_vertex(g_lines, &g_points_lines))
	{
		dbgprint("Init", "lines failed !\n");
	}// lines

	if (!Init_EMOD_vertex())
	{
		dbgprint("Init", "Init EMOD vertex failed \n");
	}



	// Initialize the world matrix
	g_World = XMMatrixIdentity();

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
	g_View = XMMatrixLookAtLH(Eye, At, Up);

	// Initialize the projection matrix
	g_Projection = XMMatrixPerspectiveFovRH(XM_PIDIV2, width / (FLOAT)height, ScreenNear, ScreenFar);

	D3D11_RASTERIZER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_RASTERIZER_DESC));
	desc.FillMode = D3D11_FILL_WIREFRAME;
	desc.CullMode = D3D11_CULL_BACK;
	g_pd3dDevice->CreateRasterizerState(&desc, &g_Wireframe_RS);

	g_OrthoMatrix = XMMatrixOrthographicLH((float)width, (float)height, ScreenNear, ScreenFar);

	D3D11_SAMPLER_DESC SD;
	ZeroMemory(&SD, sizeof(SD));

	D3D11_BUFFER_DESC bd = {};
	// Create the constant buffer
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;

	// Constant buffer init data
	ConstantBuffer const_buf_init_data;
	
	const_buf_init_data.mWorld = XMMatrixTranspose(g_World);
	const_buf_init_data.mView = XMMatrixTranspose(g_View);
	const_buf_init_data.mProjection = XMMatrixTranspose(g_Projection);

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = &const_buf_init_data;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pConstantBuffer);
	if (FAILED(hr))
		return hr;

	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	// Create a texture sampler state description.
	SD.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SD.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SD.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SD.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	SD.MipLODBias = 0.0f;
	SD.MaxAnisotropy = 1;
	SD.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	SD.BorderColor[0] = 0;
	SD.BorderColor[1] = 0;
	SD.BorderColor[2] = 0;
	SD.BorderColor[3] = 0;
	SD.MinLOD = 0;
	SD.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	hr = g_pd3dDevice->CreateSamplerState(&SD, &g_pSS);
	if (FAILED(hr))
	{
		dbgprint("Init", "CreateSamplerState failed %x \n", hr);
		return hr;
	}

	m_font = std::make_unique<SpriteFont>(g_pd3dDevice, L"font.spitefont");
	m_spriteBatch = std::make_unique<SpriteBatch>(g_pImmediateContext);

	g_pConsole = new Console(5, 25.0f, Colors::Red, 0.5f);

	g_pConsole->Add_command(&cmd_tp);
	g_pConsole->Add_command(&cmd_find_entity);
	g_pConsole->Add_command(&cmd_dump_smsg);
	g_pConsole->Add_command(&cmd_dump_emod);
	g_pConsole->Add_command(&cmd_test_map_mesh);
	g_pConsole->Add_command(&cmd_print_list_map_meshs);
	g_pConsole->Add_command(&cmd_dump_map);
	g_pConsole->Add_command(&cmd_collision_emod_id);

	return S_OK;
}



//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
	if (g_pImmediateContext) g_pImmediateContext->ClearState();

	if (g_pConstantBuffer) g_pConstantBuffer->Release();
	if (g_pVertexBuffer_env) g_pVertexBuffer_env->Release(); // enviroment
	if (g_pIndexBuffer_env) g_pIndexBuffer_env->Release();
	if (g_pVertexBuffer_stuff) g_pVertexBuffer_stuff->Release(); // bboxes
	if (g_pVertexLayout) g_pVertexLayout->Release();
	if (g_pVertexShader) g_pVertexShader->Release();
	if (g_pPixelShader) g_pPixelShader->Release();
	if (g_pRenderTargetView) g_pRenderTargetView->Release();
	if (g_pSwapChain1) g_pSwapChain1->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext1) g_pImmediateContext1->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pd3dDevice1) g_pd3dDevice1->Release();
	if (g_pd3dDevice) g_pd3dDevice->Release();
	if (g_mesh_info_structs) _aligned_free(g_mesh_info_structs);
	if (g_mesh_vis) _aligned_free(g_mesh_vis);
	
	std::vector<bbox>().swap(g_bboxes);
	std::vector<entity_prop>().swap(g_props);
	std::vector<line>().swap(g_lines);
	delete_mat_list();
	delete_nodes(hash_tree_texture);
	Delete_models();
	//Entity_delete_all(); crashes and i dont wana bother
	EMOD_destroy(); // EMOD chunks
	m_font.reset();
	m_spriteBatch.reset();
}

//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_CHAR:
		KeyChar(wParam, lParam);
		break;
	case WM_KEYDOWN:
		KeyDown(wParam, lParam);
		break;
	case WM_KEYUP:
		KeyUp(wParam, lParam);
		break;

	case WM_MOUSEMOVE:
		MouseLookUp(wParam, lParam);
		break;
	
	case WM_RBUTTONDOWN:
		MouseRMB(1, wParam, lParam);
		break;

	case WM_RBUTTONUP:
		MouseRMB(0, wParam, lParam);
		break;

	case WM_LBUTTONDOWN:
		LMB_check = true;
		check_x = LOWORD(lParam);
		check_y = HIWORD(lParam);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

//---------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void Render()
{

	g_pImmediateContext->OMSetDepthStencilState(g_pDSState, 1);
	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDSV);
	g_pImmediateContext->RSSetState(g_Env_RS);
	g_pImmediateContext->OMSetBlendState(g_pBlendStateNoBlend, 0, 0xFFFFFFFF);

	g_frustum.ConsturctFrustum(ScreenFar / 16, g_Projection, g_View);

	
	//
	// Clear the back buffer
	//
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::MidnightBlue);
	g_pImmediateContext->ClearDepthStencilView(g_pDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
	//
	// Update variables
	//

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT hr = g_pImmediateContext->Map(g_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr))
	{
		dbgprint("Draw", "Failed to update constant buffer %x \n", hr);
		dbgprint("Draw", "g_pConstantBuffer = %08x \n", g_pConstantBuffer);
		CleanupDevice();
		exit(0);
	}

	ConstantBuffer* pCb = (ConstantBuffer*)mappedResource.pData;
	pCb->mWorld = XMMatrixTranspose(g_World);
	pCb->mView = XMMatrixTranspose(g_View);
	pCb->mProjection = XMMatrixTranspose(g_Projection);

	g_pImmediateContext->Unmap(g_pConstantBuffer, 0);

	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	g_pImmediateContext->VSSetShader(g_pVertexShader_m, nullptr, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader_m, nullptr, 0);
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout_m);
	UINT stride = sizeof(model_vtx);
	UINT offset = 0;
	
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSS);
	for (std::vector<entity>::iterator it = g_entities.begin(); it != g_entities.end(); ++it)
	{
		XMFLOAT3 pos = (*it).pos;

		if (!g_frustum.CheckPoint(pos))
		{
			continue;
		}
		if (!((*it).model))
			continue;
		entity_prop* ent_prop = (*it).model;
		hash_tree_node2* node = search_by_hash2(model_hashs, ent_prop->model_hash);
		if (!node)
		{
			node = g_default_model;
		}
		model_info* mi = (model_info*)node->ptr;
		
		if (mi->empty_model)
		{
			node = g_default_model;
			mi = (model_info*)node->ptr;
		}

		g_pImmediateContext->IASetVertexBuffers(0, 1, &(mi->pVertexBuff), &stride, &offset);
		g_pImmediateContext->IASetIndexBuffer(mi->pIndexBuff, DXGI_FORMAT_R16_UINT, 0);

		XMMATRIX mTrans = XMMatrixTranslation(ent_prop->pos.x, ent_prop->pos.y, ent_prop->pos.z);
		XMMATRIX mRot = XMMatrixRotationQuaternion({ ent_prop->angle.x , ent_prop->angle.y , ent_prop->angle.z , ent_prop->angle.w });

		hr = g_pImmediateContext->Map(g_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(hr))
		{
			dbgprint("Draw_props", "Failed to update constant buffer %x \n", hr);
			CleanupDevice();
			exit(0);
		}

		ConstantBuffer* pCb = (ConstantBuffer*)mappedResource.pData;
		pCb->mWorld = XMMatrixTranspose(g_World * mRot * mTrans);
		pCb->mView = XMMatrixTranspose(g_View);
		pCb->mProjection = XMMatrixTranspose(g_Projection);

		g_pImmediateContext->Unmap(g_pConstantBuffer, 0);
		g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
		//dbgprint("draw", "mmi: %X , mmi_count %d, mat_hash: %X \n", mmi, mi->mmi_count, mmi->mat_hash);
		
		if (mi->vertex_layout)
		{
			g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		}
		else
		{
			g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		}

		int start_index = 0;
		list_node* ln;
		for (unsigned int i = 0; i < mi->mmi_count; i++)
		{
			model_mat_info* mmi = (mi->mmi + i);
			if ((ln = search_mat_by_hash(mmi->mat_hash)))
			{
				if (AVP_TEXTURES[ln->material->texture_diff_id].pSRView)
				{
					g_pImmediateContext->PSSetShaderResources(0, 1, &(AVP_TEXTURES[ln->material->texture_diff_id].pSRView));
				}
				else
				{
					g_pImmediateContext->PSSetShaderResources(0, 1, &g_pDefaultTexture);
				}
			}
			else
			{
				//continue;
				g_pImmediateContext->PSSetShaderResources(0, 1, &g_pDefaultTexture); 
			}
			g_pImmediateContext->DrawIndexed(mmi->points_count, start_index, 0);
			
			//dbgprint("draw", "DrawIndexed ( points: %d, start_index : %d, 0 ) \n", mmi[i].points_count, start_index);
			start_index += mmi->points_count;
			
		}
		
		
	}
	


	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_env, &stride, &offset);
	g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer_env, DXGI_FORMAT_R16_UINT, 0);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	hr = g_pImmediateContext->Map(g_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr))
	{
		dbgprint("Draw_map", "Failed to update constant buffer %x \n", hr);
		CleanupDevice();
		exit(0);
	}

	pCb = (ConstantBuffer*)mappedResource.pData;
	pCb->mWorld = XMMatrixTranspose(g_World);
	pCb->mView = XMMatrixTranspose(g_View);
	pCb->mProjection = XMMatrixTranspose(g_Projection);

	// ???
	g_pImmediateContext->Unmap(g_pConstantBuffer, 0);

	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
	stride = sizeof(avp_vtx);
	offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_env, &stride, &offset);
	g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer_env, DXGI_FORMAT_R16_UINT, 0);

	list_node* ln;
	for (unsigned int i = 0; i < g_total_meshes; i++)
	{
		if (g_test_map_model_enabled && g_mesh_info_structs[i].emod_id != g_test_map_model_id)
			continue;


		float x = (g_mesh_info_structs[i].bound_x + g_mesh_info_structs[i].bound_x_) / 2;
		float y = (g_mesh_info_structs[i].bound_y + g_mesh_info_structs[i].bound_y_) / 2;
		float z = (g_mesh_info_structs[i].bound_z + g_mesh_info_structs[i].bound_z_) / 2;
		float rad_x = abs(g_mesh_info_structs[i].bound_x - g_mesh_info_structs[i].bound_x_);
		float rad_y = abs(g_mesh_info_structs[i].bound_y - g_mesh_info_structs[i].bound_y_);
		float rad_z = abs(g_mesh_info_structs[i].bound_z - g_mesh_info_structs[i].bound_z_);

		float radius = (rad_x >= rad_y)*(rad_x >= rad_z)*rad_x + (rad_y > rad_x)*(rad_y >= rad_z)*rad_y + (rad_z > rad_x)*(rad_z > rad_y)*rad_z;
		g_mesh_vis[i] = g_frustum.CheckCube({ x, y, z }, radius);

		if (g_mesh_vis[i])
		{
			{
				if ((ln = search_mat_by_hash(g_mesh_info_structs[i].mat1_hash)))
				{
					if (AVP_TEXTURES[ln->material->texture_diff_id].pSRView)
					{
						//dbgprint("Draw", "Used texture id : %d and pSRView was %x \n", ln->material->texture_diff_id, AVP_TEXTURES[ln->material->texture_diff_id].pSRView);
						g_pImmediateContext->PSSetShaderResources(0, 1, &(AVP_TEXTURES[ln->material->texture_diff_id].pSRView));
					}
					else
					{
						//continue;
						g_pImmediateContext->PSSetShaderResources(0, 1, &g_pDefaultTexture);
					}
				}
				else
				{
					//continue;
					g_pImmediateContext->PSSetShaderResources(0, 1, &g_pDefaultTexture); // Maybe some diff mat like 0x5C13D641 one ?
				}
				//dbgprint("Draw", "Attempt to draw: Mesh num: %d Tri: %d SI: %d BI: %d \n", curr_mesh, g_mesh_info_structs[curr_mesh].triangles_count * 3, g_mesh_info_structs[curr_mesh].start_index, g_mesh_info_structs[curr_mesh].Min_vertex_index);
				g_pImmediateContext->DrawIndexed(g_mesh_info_structs[i].triangles_count * 3, g_mesh_info_structs[i].start_index, g_mesh_info_structs[i].Min_vertex_index);
			}
		}

	}

	g_pImmediateContext->VSSetShader(g_pVertexShader_s, nullptr, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader_s, nullptr, 0);
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout_s);
	g_pImmediateContext->IASetIndexBuffer(NULL, DXGI_FORMAT_R16_UINT, 0);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	stride = sizeof(simple_vertex);
	offset = 0;

	if (g_path2 && Show_info())
	{

		int vtx_count = (g_entities[g_selected_entity_id]).connect_list.size() * 2 + (g_entities[g_selected_entity_id]).backward_connect_list.size() + 26; //idk why but 2 more points and this draw what i need
		g_pImmediateContext->IASetVertexBuffers(0, 1, &g_dynamic_buffer_lines, &stride, &offset);
		g_pImmediateContext->Draw(vtx_count, 0);// Draw dynamic lines
	}

	if (Show_info())
	{
		g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_lines, &stride, &offset);
		g_pImmediateContext->Draw(g_points_lines, 0);

		g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_stuff, &stride, &offset);
		g_pImmediateContext->Draw(g_points_stuff, 0);// Draw stuff (bboxes)
	}

	

	if (b_show_emod && g_emod_loaded)
	{
		g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_emod_bbox, &stride, &offset);
		g_pImmediateContext->Draw(12 * 2 * g_emod_count, 0);// Draw emod (bboxes)
	}


	//Collisons view
	if (b_show_collision)
		PHEN_draw();

	g_pCollisions->Draw();
	
	m_spriteBatch->Begin();

	sprintf(g_pos_string, "pos: %f %f %f ", pos_y, pos_z, pos_x);

	XMVECTOR origin = m_font->MeasureString((const char*)g_pos_string) / 2.f;


	m_font->DrawString(m_spriteBatch.get(), (const char*)g_pos_string,
		m_fontPos, Colors::White, 0.f, origin);

	g_pConsole->Draw();

	if (g_path2 && Show_info())
	{
		float entity_farnes = 65536.0f;
		DWORD old_selected_entity = g_selected_entity_id;
		for (std::vector<entity>::iterator it = g_entities.begin(); it != g_entities.end(); ++it)
		{
			if (g_frustum.CheckPoint((*it).pos))
			{
				// code that draw other stuff here
				if (g_selected_entity_id == it - g_entities.begin())
				{
					origin = m_font->MeasureString((const char*)(*it).data) / 2.f;
					m_font->DrawString(m_spriteBatch.get(), (const char*)(*it).data,
						g_DataFontPos, Colors::White, 0.f, origin, { 0.5f, 0.5f });
					origin = m_font->MeasureString((const char*)(*it).name) / 2.f;
					m_font->DrawString(m_spriteBatch.get(), (const char*)(*it).name,
						{ screen_width - 200.0f, 50.0f }, Colors::Magenta, 0.f, origin, { 0.5f, 0.5f });
				}
				XMVECTOR cam_pos = { pos_y, pos_z, pos_x };
				XMVECTOR pos = XMLoadFloat3(&(*it).pos);
				float d = XMVector3Length(pos - cam_pos).m128_f32[0];
				if (d <= 7.5f)
				{
					XMVECTOR rel_pos = XMVector3Transform(pos, g_View); // x, y, z in camera coords
					if (abs(rel_pos.m128_f32[0] / rel_pos.m128_f32[2]) <= 1.0f && abs(rel_pos.m128_f32[1] / rel_pos.m128_f32[2]) <= 1.0f)
					{
						XMVECTOR text_pos = { screen_height*(-rel_pos.m128_f32[0] / rel_pos.m128_f32[2] + 1.77f) / 2, screen_height*(+rel_pos.m128_f32[1] / rel_pos.m128_f32[2] + 1.0f) / 2 };
						XMVECTOR tilt = { 2.0f, 2.0f };
						origin = m_font->MeasureString((const char*)(*it).name);
						m_font->DrawString(m_spriteBatch.get(), (const char*)(*it).name, text_pos, { 0.0f, 0.0f, 0.0f, 1.0f }, 0.0f, origin, { (0.4f + d / 15, 0.4f + d / 15) });
						m_font->DrawString(m_spriteBatch.get(), (const char*)(*it).name, text_pos - tilt, { 1.0f, 1.0f, 1.0f, 1.0f }, 0.0f, origin, { (0.4f + d / 15, 0.4f + d / 15) });
					}
				}
				if (LMB_check)
				{
					XMFLOAT3 pos = (*it).pos;
					XMVECTOR VP = { pos.x - pos_y, pos.y - pos_z, pos.z - pos_x, 1.0f }; // if someone ever read this, Im so lazy, that my camera value names is incorrect, but i dont wanna change it
																						 //		so pos_y -> x		pos_z -> y			pos_x -> z				okay i think you now understand what its mean to be lazy ;)
					XMVECTOR Rz = { g_View.r[0].m128_f32[2], g_View.r[1].m128_f32[2], g_View.r[2].m128_f32[2], 0.0f }; // Taking Z axis from View matrix	 
					XMVECTOR Rx = { g_View.r[0].m128_f32[0], g_View.r[1].m128_f32[0], g_View.r[2].m128_f32[0], 0.0f };
					XMVECTOR Ry = { g_View.r[0].m128_f32[1], g_View.r[1].m128_f32[1], g_View.r[2].m128_f32[1], 0.0f };
					//Dump_vector("Rx", Rx);
					//Dump_vector("Ry", Ry);
					//Dump_vector("Rz", Rz);

					Rx = Rx * ((float)screen_width / (float)screen_height - 2 * (float)check_x / (float)screen_height);
					Ry = Ry * (2 * (float)check_y / (float)screen_height - 1.0f);
					//dbgprint("h w", "%f   %f \n", 2 * ((float)check_x / (float)screen_width - 0.5f), 2 * ((float)check_y / (float)screen_height - 0.5f));
					XMVECTOR R = Rz + Rx + Ry;
					//Dump_vector("R", R);

					XMVECTOR VPnR = XMVector3Cross(VP, R); // There is mathematic formula d = || VP x R || / || R || that gives us d - distance betwen line and point
					float d = XMVector3Length(VPnR).m128_f32[0]; // len(R) = 1.0f
					if (d <= 0.2f)
					{
						if (entity_farnes >= XMVector3Length(VP).m128_f32[0])
						{
							entity_farnes = XMVector3Length(VP).m128_f32[0]; // chose the closest to us entity;
																			 //second_selected_entity = g_selected_entity_id;
							g_selected_entity_id = it - g_entities.begin();
						}
					}
				}
			}
		}
		LMB_check = 0;
		if (old_selected_entity != g_selected_entity_id)
		{
			Entity_update_vertex_buffer(g_selected_entity_id);
			old_selected_entity = g_selected_entity_id;
		}
	}
	


	//EMOD info
	if (b_show_emod && g_emod_loaded)
	{
		for (int i = 0; i < g_emod_count; i++)
		{
			EMOD_entry* emod_e = g_emod_entrys + i;
			emod_e->visible = g_frustum.CheckPoint(emod_e->pos);
			if (emod_e->visible)
			{
				XMVECTOR cam_pos = { pos_y, pos_z, pos_x };
				XMVECTOR pos = XMLoadFloat3(&(emod_e->pos));
				float d = XMVector3Length(pos - cam_pos).m128_f32[0];
				if (d <= 30.0f)
				{
					XMVECTOR rel_pos = XMVector3Transform(pos, g_View); // x, y, z in camera coords
					if ((abs(rel_pos.m128_f32[0] / rel_pos.m128_f32[2]) <= 1.0f && abs(rel_pos.m128_f32[1] / rel_pos.m128_f32[2]) <= 1.0f) && emod_e->name)
					{
						XMVECTOR text_pos = { screen_height*(-rel_pos.m128_f32[0] / rel_pos.m128_f32[2] + 1.77f) / 2, screen_height*(+rel_pos.m128_f32[1] / rel_pos.m128_f32[2] + 1.0f) / 2 };
						XMVECTOR tilt = { 2.0f, 2.0f };
						XMVECTOR origin = m_font->MeasureString((const char*)(emod_e->name));
						m_font->DrawString(m_spriteBatch.get(), (const char*)(emod_e->name), text_pos, { 0.0f, 0.0f, 0.0f, 1.0f }, 0.0f, origin, { (0.4f + d / 60, 0.4f + d / 60) });
						m_font->DrawString(m_spriteBatch.get(), (const char*)(emod_e->name), text_pos - tilt, { 1.0f, 0.3f, 0.3f, 1.0f }, 0.0f, origin, { (0.4f + d / 60, 0.4f + d / 60) });
					}
				}
			}
		}
	}

	m_spriteBatch->End();

	

	g_pSwapChain->Present(1, 0);
	
	

	g_framecount++;

}

