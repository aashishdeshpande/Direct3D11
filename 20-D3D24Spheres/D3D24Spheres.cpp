#include<Windows.h>
#include<stdio.h>

#include<d3d11.h>
#include<d3dcompiler.h>

#pragma warning(disable : 4838)
#include "XNAMath\xnamath.h"
#include "Sphere.h"

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "D3dcompiler.lib")
#pragma comment (lib, "Sphere.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

enum {
	VDG_ATTRIBUTE_POSITION = 0,
	VDG_ATTRIBUTE_COLOR,
	VDG_ATTRIBUTE_NORMAL,
	VDG_ATTRIBUTE_TEXTURE0,
};

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

FILE *gpFile = NULL;
char gszLogFileName[] = "Log.txt";

HWND ghwnd = NULL;

DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };

bool gbActiveWindow = false;
bool gbEscapeKeyIsPressed = false;
bool gbFullscreen = false;
float gAnglePerspectiveDeg = 45.0f;

float glClearColor[4];
IDXGISwapChain *gpIDXGISwapChain = NULL;
ID3D11Device *gpID3D11Device = NULL;
ID3D11DeviceContext *gpID3D11DeviceContext = NULL;
ID3D11RenderTargetView *gpID3D11RenderTargetView = NULL;

ID3D11VertexShader *gpID3D11VertexShader = NULL;
ID3D11PixelShader *gpID3D11PixelShader = NULL;

//For Sphere
ID3D11Buffer *gpID3D11Buffer_VertexBuffer_Sphere_Position = NULL;
ID3D11Buffer *gpID3D11Buffer_VertexBuffer_Sphere_Normal = NULL;
ID3D11Buffer *gpID3D11Buffer_IndexBuffer = NULL;

//For Disabling BackFace Culling
ID3D11RasterizerState *gpID3D11RasterizerState = NULL;

ID3D11InputLayout *gpID3D11InputLayout = NULL;
ID3D11Buffer *gpID3D11Buffer_ConstantBuffer = NULL;

//For Depth
ID3D11DepthStencilView *gpID3D11DepthStencilView = NULL;

D3D11_VIEWPORT d3dViewPort;
int giWinWidth, giWinHeight;

float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];
unsigned int gNumElements;
unsigned int gNumVertices;

struct CBUFFER
{
	XMMATRIX WorldMatrix;
	XMMATRIX ViewMatrix;
	XMMATRIX ProjectionMatrix;

	XMVECTOR Ld;
	XMVECTOR Kd;

	XMVECTOR La;
	XMVECTOR Ka;

	XMVECTOR Ls;
	XMVECTOR Ks;

	float material_shininess;

	XMVECTOR LightPosition;
	unsigned int LKeyPressed;
};

bool gbLight;
bool gbXaxis = false;
bool gbYaxis = false;
bool gbZaxis = false;

float gfangleX_axis = 0.0f;
float gfangleY_axis = 0.0f;
float gfangleZ_axis = 0.0f;

float lightAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
float lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float lightPosition[] = { 100.0f, 100.0f, -100.0f, 1.0f };

//Emrald
float material1_ambient[] = { 0.0215f, 0.1745f, 0.0215f, 1.0f };
float material1_diffuse[] = { 0.07568f, 0.061424f, 0.07568f, 1.0f };
float material1_specular[] = { 0.633f, 0.727811f, 0.633f, 1.0f };
float material1_shininess = 76.8f;

//Jade
float material2_ambient[] = { 0.135f, 0.2225f ,0.1575f, 1.0f };
float material2_diffuse[] = { 0.54f, 0.89f, 0.63f, 1.0f };
float material2_specular[] = { 0.316288f, 0.316288f, 0.316288f, 1.0f };
float material2_shininess = 12.8f;

//Obsidian
float material3_ambient[] = { 0.05375f, 0.05f, 0.06625f, 1.0f };
float material3_diffuse[] = { 0.18275f, 0.17f, 0.22525f, 1.0f };
float material3_specular[] = { 0.332741f, 0.328634f, 0.346435f, 1.0f };
float material3_shininess = 38.4f;

//Pearl
float material4_ambient[] = { 0.25f, 0.20725f, 0.20725f, 1.0f };
float material4_diffuse[] = { 1.0f, 0.829f, 0.829f, 1.0f };
float material4_specular[] = { 0.296648f, 0.296648f, 0.296648f, 1.0f };
float material4_shininess = 11.264f;

//Ruby
float material5_ambient[] = { 0.1745f, 0.01175f, 0.01175f, 1.0f };
float material5_diffuse[] = { 0.61424f, 0.04136f, 0.04136f, 1.0f };
float material5_specular[] = { 0.727811f, 0.626959f, 0.626959f, 1.0f };
float material5_shininess = 76.8f;

//Turquoise
float material6_ambient[] = { 0.1f, 0.18725f, 0.1745f, 1.0f };
float material6_diffuse[] = { 0.396f, 0.74151f, 0.69102f, 1.0f };
float material6_specular[] = { 0.297254f, 0.30829f, 0.306678f, 1.0f };
float material6_shininess = 12.8f;

//Brass
float material7_ambient[] = { 0.329412f, 0.223529f, 0.027451f, 1.0f };
float material7_diffuse[] = { 0.780392f, 0.568627f, 0.113725f, 1.0f };
float material7_specular[] = { 0.992157f, 0.941176f, 0.807843f, 1.0f };
float material7_shininess = 27.897443616f;

//Bronze
float material8_ambient[] = { 0.2125f, 0.1275f, 0.054f, 1.0f };
float material8_diffuse[] = { 0.714f, 0.4284f, 0.18144f, 1.0f };
float material8_specular[] = { 0.393548f, 0.271906f, 0.166721f, 1.0f };
float material8_shininess = 25.6f;

//Chrome
float material9_ambient[] = { 0.25f, 0.25f, 0.25f, 1.0f };
float material9_diffuse[] = { 0.4f, 0.4f, 0.4f, 1.0f };
float material9_specular[] = { 0.774597f, 0.774597f, 0.774597f, 1.0f };
float material9_shininess = 76.8f;

//Copper
float material10_ambient[] = { 0.19125f, 0.0735f, 0.0225f, 1.0f };
float material10_diffuse[] = { 0.7038f, 0.27048f, 0.0828f, 1.0f };
float material10_specular[] = { 0.256777f, 0.137622f, 0.086014f, 1.0f };
float material10_shininess = 12.8f;

//Gold
float material11_ambient[] = { 0.24725f, 0.1995f, 0.0745f, 1.0f };
float material11_diffuse[] = { 0.75164f, 0.60648f, 0.22648f, 1.0f };
float material11_specular[] = { 0.628281f, 0.555802f, 0.366065f, 1.0f };
float material11_shininess = 51.2f;

//Silver
float material12_ambient[] = { 0.19225f, 0.19225f, 0.19225f, 1.0f };
float material12_diffuse[] = { 0.50754f, 0.50754f, 0.50754f, 1.0f };
float material12_specular[] = { 0.508273f, 0.508273f, 0.508273f, 1.0f };
float material12_shininess = 51.2f;

//Black
float material13_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
float material13_diffuse[] = { 0.01f, 0.01f, 0.01f, 1.0f };
float material13_specular[] = { 0.50f, 0.50f, 0.50f, 1.0f };
float material13_shininess = 32.0f;

//Cyan
float material14_ambient[] = { 0.0f, 0.1f, 0.06f, 1.0f };
float material14_diffuse[] = { 0.0f, 0.50980392f, 0.50980392f, 1.0f };
float material14_specular[] = { 0.50196078f, 0.50196078f, 0.50196078f, 1.0f };
float material14_shininess = 32.0f;

//Green
float material15_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
float material15_diffuse[] = { 0.1f, 0.35f, 0.1f, 1.0f };
float material15_specular[] = { 0.45f, 0.55f, 0.45f, 1.0f };
float material15_shininess = 32.0f;

//Red
float material16_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
float material16_diffuse[] = { 0.5f, 0.0f, 0.0f, 1.0f };
float material16_specular[] = { 0.7f, 0.6f, 0.6f, 1.0f };
float material16_shininess = 32.0f;

//White
float material17_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
float material17_diffuse[] = { 0.55f, 0.55f, 0.55f, 1.0f };
float material17_specular[] = { 0.70f, 0.70f, 0.70f, 1.0f };
float material17_shininess = 32.0f;

//Yellow Plastic
float material18_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
float material18_diffuse[] = { 0.5f, 0.5f, 0.0f, 1.0f };
float material18_specular[] = { 0.60f, 0.60f, 0.50f, 1.0f };
float material18_shininess = 32.0f;

//Black
float material19_ambient[] = { 0.02f, 0.02f, 0.02f, 1.0f };
float material19_diffuse[] = { 0.01f, 0.01f, 0.01f, 1.0f };
float material19_specular[] = { 0.4f, 0.4f, 0.4f, 1.0f };
float material19_shininess = 10.0f;

//Cyan
float material20_ambient[] = { 0.0f, 0.05f, 0.05f, 1.0f };
float material20_diffuse[] = { 0.4f, 0.5f, 0.5f, 1.0f };
float material20_specular[] = { 0.04f, 0.7f, 0.7f, 1.0f };
float material20_shininess = 10.0f;

//Green
float material21_ambient[] = { 0.0f, 0.05f, 0.0f, 1.0f };
float material21_diffuse[] = { 0.4f, 0.5f, 0.4f, 1.0f };
float material21_specular[] = { 0.04f, 0.7f, 0.04f, 1.0f };
float material21_shininess = 10.0f;

//Red
float material22_ambient[] = { 0.05f, 0.0f, 0.0f, 1.0f };
float material22_diffuse[] = { 0.5f, 0.4f, 0.4f, 1.0f };
float material22_specular[] = { 0.7f, 0.04f, 0.04f, 1.0f };
float material22_shininess = 10.0f;

//White
float material23_ambient[] = { 0.05f, 0.05f, 0.05f, 1.0f };
float material23_diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
float material23_specular[] = { 0.7f, 0.7f, 0.7f, 1.0f };
float material23_shininess = 10.0f;

//Yellow Rubber
float material24_ambient[] = { 0.05f, 0.05f, 0.0f, 1.0f };
float material24_diffuse[] = { 0.5f, 0.5f, 0.4f, 1.0f };
float material24_specular[] = { 0.7f, 0.7f, 0.04f, 1.0f };
float material24_shininess = 10.0f;

XMMATRIX gPerspectiveProjectionMatrix;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	HRESULT initialize(void);
	void uninitialize(void);
	void display(void);
	void update(void);

	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szClassName[] = TEXT("D3D11");
	bool bDone = false;

	if (fopen_s(&gpFile, gszLogFileName, "w") != 0)
	{
		MessageBox(NULL, TEXT("Log File Can Not Be Created\n Exiting.."), TEXT("Error"), MB_OK | MB_TOPMOST | MB_ICONSTOP);
		exit(0);
	}
	else
	{
		fprintf_s(gpFile, "Log File is Successfully Opened.\n");
		fclose(gpFile);
	}

	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hInstance = hInstance;
	wndclass.lpszClassName = szClassName;
	wndclass.lpszMenuName = NULL;

	RegisterClassEx(&wndclass);

	int x = GetSystemMetrics(SM_CXSCREEN);
	int y = GetSystemMetrics(SM_CYSCREEN);

	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szClassName,
		TEXT("D3D11 Window : 24 Spheres"),
		WS_OVERLAPPEDWINDOW,
		(x / 2) - (WIN_WIDTH / 2),
		(y / 2) - (WIN_HEIGHT / 2),
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	ghwnd = hwnd;

	ShowWindow(hwnd, iCmdShow);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	HRESULT hr;
	hr = initialize();

	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "initialize() Failed. Exiting Now..\n");
		fclose(gpFile);
		DestroyWindow(hwnd);
		hwnd = NULL;
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "intialize() Succeeded.\n");
		fclose(gpFile);
	}

	//message Loop
	while (bDone == false)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				bDone = true;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			display();
			update();

			if (gbActiveWindow == true)
			{
				if (gbEscapeKeyIsPressed == true)
					bDone = true;
			}
		}
	}
	uninitialize();
	return((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	HRESULT resize(int, int);
	void ToggleFullscreen(void);
	void uninitialize(void);

	HRESULT hr;

	static bool bIsAKeyPressed = false;
	static bool bIsLKeyPressed = false;

	switch (iMsg)
	{
	case WM_ACTIVATE:
		if (HIWORD(wParam) == 0)
			gbActiveWindow = true;
		else
			gbActiveWindow = false;
		break;

	case WM_ERASEBKGND:
		return(0);
		break;

	case WM_SIZE:
		if (gpID3D11DeviceContext)
		{
			hr = resize(LOWORD(lParam), HIWORD(lParam));
			if (FAILED(hr))
			{
				fopen_s(&gpFile, gszLogFileName, "a+");
				fprintf_s(gpFile, "resize() Failed.\n");
				fclose(gpFile);
				return(hr);
			}
			else
			{
				fopen_s(&gpFile, gszLogFileName, "a+");
				fprintf_s(gpFile, "resize() Succeeded.\n");
				fclose(gpFile);
			}
		}
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			if (gbEscapeKeyIsPressed == false)
			{
				gbEscapeKeyIsPressed = true;
			}
			break;

		case 0x46:
			if (gbFullscreen == false)
			{
				ToggleFullscreen();
				gbFullscreen = true;
			}
			else
			{
				ToggleFullscreen();
				gbFullscreen = false;
			}
			break;

		case 0x4C:
			if (bIsLKeyPressed == false)
			{
				gbLight = true;
				bIsLKeyPressed = true;
			}
			else
			{
				gbLight = false;
				bIsLKeyPressed = false;
			}
			break;

		case 0x58:
			gbXaxis = true;
			gbYaxis = false;
			gbZaxis = false;

			break;

		case 0x59:
			gbYaxis = true;
			gbXaxis = false;
			gbZaxis = false;
			break;

		case 0x5A:
			gbZaxis = true;
			gbYaxis = false;
			gbXaxis = false;
			break;

		default:
			break;
		}
		break;

	case WM_LBUTTONDOWN:
		break;

	case WM_CLOSE:
		uninitialize();
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		break;
	}
	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullscreen(void)
{
	MONITORINFO mi;

	if (gbFullscreen == false)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi = { sizeof(MONITORINFO) };
			if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
			}
			ShowCursor(FALSE);
		}
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
		ShowCursor(TRUE);
	}
}

HRESULT initialize(void)
{
	void uninitialize(void);
	HRESULT resize(int, int);

	HRESULT hr;
	D3D_DRIVER_TYPE d3dDriverType;
	D3D_DRIVER_TYPE d3dDriverTypes[] = { D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_REFERENCE };
	D3D_FEATURE_LEVEL d3dFeatureLevel_required = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL d3dFeatureLevel_acquired = D3D_FEATURE_LEVEL_10_0;

	UINT createDeviceFlags = 0;
	UINT numDriverTypes = 0;
	UINT numFeatureLevels = 1;

	numDriverTypes = sizeof(d3dDriverType) / sizeof(d3dDriverTypes[0]);

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	ZeroMemory((void *)&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	dxgiSwapChainDesc.BufferCount = 1;
	dxgiSwapChainDesc.BufferDesc.Width = WIN_WIDTH;
	dxgiSwapChainDesc.BufferDesc.Height = WIN_HEIGHT;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.OutputWindow = ghwnd;
	dxgiSwapChainDesc.SampleDesc.Count = 4;
	dxgiSwapChainDesc.SampleDesc.Quality = 1;
	dxgiSwapChainDesc.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		d3dDriverType = d3dDriverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL,
			d3dDriverType,
			NULL,
			createDeviceFlags,
			&d3dFeatureLevel_required,
			numFeatureLevels,
			D3D11_SDK_VERSION,
			&dxgiSwapChainDesc,
			&gpIDXGISwapChain,
			&gpID3D11Device,
			&d3dFeatureLevel_acquired,
			&gpID3D11DeviceContext);
		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3D11CreateDeviceAndSwapChain() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3D11CreateDeviceAndSwapChain() Succeeded.\n");
		fprintf_s(gpFile, "The Chosen Driver is : ");
		if (d3dDriverType == D3D_DRIVER_TYPE_HARDWARE)
		{
			fprintf_s(gpFile, "Hardware Type.\n");
		}
		else if (d3dDriverType == D3D_DRIVER_TYPE_WARP)
		{
			fprintf_s(gpFile, "Warp Type.\n");
		}
		else if (d3dDriverType == D3D_DRIVER_TYPE_REFERENCE)
		{
			fprintf_s(gpFile, "Reference Type.\n");
		}
		else
		{
			fprintf_s(gpFile, "Unknown Type.\n");
		}

		fprintf_s(gpFile, "The Supported Highest Feature Level Is");
		if (d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_11_0)
		{
			fprintf_s(gpFile, "11.0\n");
		}
		else if (d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_10_1)
		{
			fprintf_s(gpFile, "10.1\n");
		}
		else if (d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_10_0)
		{
			fprintf_s(gpFile, "10.0\n");
		}
		else
		{
			fprintf_s(gpFile, "Unknown.\n");
		}
		fclose(gpFile);
	}

	//init Shader, input Layout, constant buffers etc.

	const char *vertexShaderSourceCode =
		"cbuffer ConstantBuffer											\n" \
		"{																\n"
		"float4x4 worldMatrix;											\n" \
		"float4x4 viewMatrix;											\n" \
		"float4x4 projectionMatrix;										\n" \
		"float4 ld;														\n" \
		"float4 kd;														\n" \
		"float4 la;														\n" \
		"float4 ka;														\n" \
		"float4 ls;														\n" \
		"float4 ks;														\n" \
		"float material_shininess;										\n" \
		"float4 lightPosition;											\n" \
		"unsigned int lKeyPressed;										\n" \
		"}																\n" \
		"struct vertex_output											\n" \
		"{																\n" \
		"float4 position : SV_POSITION;									\n" \
		"float3 transformed_normals : NORMAL0;							\n" \
		"float3 light_direction : NORMAL1;								\n" \
		"float3 viewer_vector : NORMAL2;								\n" \
		"};																\n" \
		"vertex_output main(float4 pos : POSITION, float4 normal : NORMAL)	\n" \
		"{																\n" \
		"vertex_output output;											\n" \
		"if(lKeyPressed == 1)											\n" \
		"{																\n" \
		"float4x4 worldMulview = mul(worldMatrix, viewMatrix);			\n" \
		"float4 eyeCoordinates = mul(worldMulview, pos);				\n" \
		"output.transformed_normals = mul((float3x3)(mul(viewMatrix, worldMatrix)), (float3)normal);		\n" \
		"output.light_direction = ((float3)(lightPosition) - eyeCoordinates.xyz);							\n" \
		"output.viewer_vector = (-eyeCoordinates.xyz);					\n" \
		"}																\n" \
		"float4x4 mulpos = mul(worldMatrix, viewMatrix);				\n" \
		"output.position = mul(mul(projectionMatrix, mulpos), pos);		\n" \
		"return(output);												\n" \
		"}";

	ID3DBlob *pID3DBlob_VertexShaderCode = NULL;
	ID3DBlob *pID3DBlob_Error = NULL;

	hr = D3DCompile(vertexShaderSourceCode,
		lstrlenA(vertexShaderSourceCode) + 1,
		"VS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		0,
		0,
		&pID3DBlob_VertexShaderCode,
		&pID3DBlob_Error);

	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "D3DCompile() Failed For Vertex Shader : %s.\n", (char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
			return(hr);
		}
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3DCompile() Succeeded For Vertex Shader.\n");
		fclose(gpFile);
	}

	hr = gpID3D11Device->CreateVertexShader(pID3DBlob_VertexShaderCode->GetBufferPointer(), pID3DBlob_VertexShaderCode->GetBufferSize(), NULL, &gpID3D11VertexShader);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateVertexShader() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateVertexShader() Succeeded.\n");
		fclose(gpFile);
	}
	gpID3D11DeviceContext->VSSetShader(gpID3D11VertexShader, 0, 0);

	const char *pixelShaderSourceCode =
		"cbuffer ConstantBuffer							\n" \
		"{												\n"
		"float4x4 worldMatrix;							\n" \
		"float4x4 viewMatrix;							\n" \
		"float4x4 projectionMatrix;						\n" \
		"float4 ld;										\n" \
		"float4 kd;										\n" \
		"float4 la;										\n" \
		"float4 ka;										\n" \
		"float4 ls;										\n" \
		"float4 ks;										\n" \
		"float material_shininess;						\n" \
		"float4 lightPosition;							\n" \
		"unsigned int lKeyPressed;						\n" \
		"}												\n" \
		"struct vertex_output							\n" \
		"{												\n" \
		"float4 position : SV_POSITION;					\n" \
		"float3 transformed_normals : NORMAL0;			\n" \
		"float3 light_direction : NORMAL1;				\n" \
		"float3 viewer_vector : NORMAL2;				\n" \
		"};												\n" \
		"float4 main(float4 position : SV_POSITION, vertex_output input) : SV_TARGET		\n" \
		"{																					\n" \
		"float4 phong_ads_color;															\n" \
		"if(lKeyPressed == 1)																\n" \
		"{																					\n"
		"float3 normalized_transformed_normals = normalize(input.transformed_normals);			\n" \
		"float3 normalized_light_direction = normalize(input.light_direction);					\n" \
		"float3 normalized_viewer_vector = normalize(input.viewer_vector);						\n" \
		"float4 ambient = la * ka;															\n" \
		"float tn_dot_ld = max(dot(normalized_transformed_normals, normalized_light_direction), 0.0);		\n" \
		"float4 diffuse = ld * kd * tn_dot_ld;												\n" \
		"float3 reflection_vector = reflect(-normalized_light_direction, normalized_transformed_normals);	\n" \
		"float4 specular = ls * ks * pow(max(dot(reflection_vector, normalized_viewer_vector), 0.0), material_shininess);		\n" \
		"phong_ads_color = ambient + diffuse + specular;									\n" \
		"}													\n"
		"else												\n" \
		"{													\n" \
		"phong_ads_color = float4(1.0f, 1.0f, 1.0f, 1.0f);	\n" \
		"}													\n"
		"return(phong_ads_color);							\n" \
		"}";

	ID3DBlob *pID3DBlob_PixelShaderCode = NULL;
	pID3DBlob_Error = NULL;

	hr = D3DCompile(pixelShaderSourceCode,
		lstrlenA(pixelShaderSourceCode) + 1,
		"PS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		0,
		0,
		&pID3DBlob_PixelShaderCode,
		&pID3DBlob_Error);

	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "D3DCompile() Failed For Pixel Shader : %s.\n", (char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
			return(hr);
		}
	}

	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3DCompile() Succeeded For Pixel Shader.\n");
		fclose(gpFile);
	}

	hr = gpID3D11Device->CreatePixelShader(pID3DBlob_PixelShaderCode->GetBufferPointer(), pID3DBlob_PixelShaderCode->GetBufferSize(), NULL, &gpID3D11PixelShader);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreatePixelShader() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreatePixelShader() Succeeded.\n");
		fclose(gpFile);
	}

	gpID3D11DeviceContext->PSSetShader(gpID3D11PixelShader, 0, 0);
	pID3DBlob_PixelShaderCode->Release();

	pID3DBlob_PixelShaderCode = NULL;

	//Create and set input layout
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[2];
	inputElementDesc[0].SemanticName = "POSITION";
	inputElementDesc[0].SemanticIndex = 0;
	inputElementDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[0].InputSlot = VDG_ATTRIBUTE_POSITION;
	inputElementDesc[0].AlignedByteOffset = 0;
	inputElementDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[0].InstanceDataStepRate = 0;

	//for color
	inputElementDesc[1].SemanticName = "NORMAL";
	inputElementDesc[1].SemanticIndex = 0;
	inputElementDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[1].InputSlot = VDG_ATTRIBUTE_NORMAL;
	inputElementDesc[1].AlignedByteOffset = 0;
	inputElementDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[1].InstanceDataStepRate = 0;

	hr = gpID3D11Device->CreateInputLayout(inputElementDesc, 2, pID3DBlob_VertexShaderCode->GetBufferPointer(), pID3DBlob_VertexShaderCode->GetBufferSize(), &gpID3D11InputLayout);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateInputLayout() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateInputLayout() Succeeded.\n");
		fclose(gpFile);
	}

	gpID3D11DeviceContext->IASetInputLayout(gpID3D11InputLayout);
	pID3DBlob_VertexShaderCode->Release();
	pID3DBlob_VertexShaderCode = NULL;

	/*************FOR Sphere***************/
	getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
	gNumElements = getNumberOfSphereElements();
	gNumVertices = getNumberOfSphereVertices();

	//create vertex buffer for Square position
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(float) * ARRAYSIZE(sphere_vertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc, NULL, &gpID3D11Buffer_VertexBuffer_Sphere_Position);

	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() Failed for Vertex Buffer For Sphere position.\n");
		fclose(gpFile);
		return(hr);
	}

	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() Succeeded For Vertex Buffer For Sphere position.\n");
		fclose(gpFile);
	}

	//copy Cube vertices into above buffer
	D3D11_MAPPED_SUBRESOURCE mappedSubResource;
	ZeroMemory(&mappedSubResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_VertexBuffer_Sphere_Position, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubResource);
	memcpy(mappedSubResource.pData, sphere_vertices, sizeof(sphere_vertices));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_VertexBuffer_Sphere_Position, 0);

	//create buffer for Cube color 
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(float) * ARRAYSIZE(sphere_normals);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc, NULL, &gpID3D11Buffer_VertexBuffer_Sphere_Normal);

	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() Failed for Vertex Buffer Sphere color.\n");
		fclose(gpFile);
		return(hr);
	}

	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() Succeeded For Vertex Buffer Sphere color.\n");
		fclose(gpFile);
	}

	//copy color into the above buffer created for color
	ZeroMemory(&mappedSubResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_VertexBuffer_Sphere_Normal, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubResource);
	memcpy(mappedSubResource.pData, sphere_normals, sizeof(sphere_normals));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_VertexBuffer_Sphere_Normal, 0);

	//create index buffer
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = gNumElements * sizeof(unsigned short);
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc, NULL, &gpID3D11Buffer_IndexBuffer);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() Failed for Index Buffer Sphere.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() Succeeded For Index Buffer Sphere.\n");
		fclose(gpFile);
	}

	//copy indices into above index buffer
	ZeroMemory(&mappedSubResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_IndexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubResource);
	memcpy(mappedSubResource.pData, sphere_elements, gNumElements * sizeof(unsigned short));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_IndexBuffer, NULL);

	//define and set the constant buffer
	D3D11_BUFFER_DESC bufferDesc_ConstantBuffer;
	ZeroMemory(&bufferDesc_ConstantBuffer, sizeof(D3D11_BUFFER_DESC));

	bufferDesc_ConstantBuffer.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc_ConstantBuffer.ByteWidth = sizeof(CBUFFER);
	bufferDesc_ConstantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc_ConstantBuffer, nullptr, &gpID3D11Buffer_ConstantBuffer);

	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer Failed for ConstantBuffer.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer Succeeded for ConstantBuffer.\n");
		fclose(gpFile);
	}

	gpID3D11DeviceContext->VSSetConstantBuffers(0, 1, &gpID3D11Buffer_ConstantBuffer);

	//For accessing the uniform in Pixel Shader
	gpID3D11DeviceContext->PSSetConstantBuffers(0, 1, &gpID3D11Buffer_ConstantBuffer);

	//To Disable the BackFace Culling which is by default ON in D3D
	D3D11_RASTERIZER_DESC rasterizer_Desc;
	ZeroMemory(&rasterizer_Desc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizer_Desc.AntialiasedLineEnable = FALSE;
	rasterizer_Desc.MultisampleEnable = FALSE;
	rasterizer_Desc.DepthBias = 0;
	rasterizer_Desc.DepthBiasClamp = 0.0f;
	rasterizer_Desc.SlopeScaledDepthBias = 0.0f;
	rasterizer_Desc.CullMode = D3D11_CULL_NONE;
	rasterizer_Desc.DepthClipEnable = TRUE;
	rasterizer_Desc.FillMode = D3D11_FILL_SOLID;
	rasterizer_Desc.FrontCounterClockwise = FALSE;
	rasterizer_Desc.ScissorEnable = FALSE;

	//Create the state of rasterizer
	hr = gpID3D11Device->CreateRasterizerState(&rasterizer_Desc, &gpID3D11RasterizerState);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateRasterizerState Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateRasterizerState Succeeded.\n");
		fclose(gpFile);
	}

	//Set the newly created rasterizer state
	gpID3D11DeviceContext->RSSetState(gpID3D11RasterizerState);

	//clear color to blue
	glClearColor[0] = 0.5f;
	glClearColor[1] = 0.5f;
	glClearColor[2] = 0.5f;
	glClearColor[3] = 0.5f;

	//set projection matrix to identity
	gPerspectiveProjectionMatrix = XMMatrixIdentity();

	gbLight = false;
	//call resize for first time
	hr = resize(WIN_WIDTH, WIN_HEIGHT);

	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "resize() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "resize() Succeeded.\n");
		fclose(gpFile);
	}
	return(S_OK);
}

HRESULT resize(int width, int height)
{
	HRESULT hr = S_OK;

	if (gpID3D11DepthStencilView)
	{
		gpID3D11DepthStencilView->Release();
		gpID3D11DepthStencilView = NULL;
	}

	if (gpID3D11RenderTargetView)
	{
		gpID3D11RenderTargetView->Release();
		gpID3D11RenderTargetView = NULL;
	}

	//resize buffers accordingly
	gpIDXGISwapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

	//get back buffer from swap chain
	ID3D11Texture2D *pID3D11Texture2D_BackBuffer;
	gpIDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&pID3D11Texture2D_BackBuffer);

	//get render target view from d3d11 device using above back buffer
	hr = gpID3D11Device->CreateRenderTargetView(pID3D11Texture2D_BackBuffer, NULL, &gpID3D11RenderTargetView);

	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateRenderTargetView() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateRenderTargetView() Succeeded.\n");
		fclose(gpFile);
	}
	pID3D11Texture2D_BackBuffer->Release();
	pID3D11Texture2D_BackBuffer = NULL;

	//set render target view as render target
	gpID3D11DeviceContext->OMSetRenderTargets(1, &gpID3D11RenderTargetView, NULL);

	//FOR DEPTH ENABLING WE USE TEXTURE BUFFER
	D3D11_TEXTURE2D_DESC texture_Desc;

	ZeroMemory(&texture_Desc, sizeof(D3D11_TEXTURE2D_DESC));

	texture_Desc.Width = width;
	texture_Desc.Height = height;
	texture_Desc.ArraySize = 1;
	texture_Desc.MipLevels = 1;
	texture_Desc.SampleDesc.Quality = 1;
	texture_Desc.SampleDesc.Count = 4;
	texture_Desc.Format = DXGI_FORMAT_D32_FLOAT;
	texture_Desc.Usage = D3D11_USAGE_DEFAULT;
	texture_Desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texture_Desc.CPUAccessFlags = 0;
	texture_Desc.MiscFlags = 0;

	ID3D11Texture2D *pID3D11Texture2D_DepthBuffer = NULL;

	hr = gpID3D11Device->CreateTexture2D(&texture_Desc, NULL, &pID3D11Texture2D_DepthBuffer);

	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateTexture2D() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateTexture2D() Succeeded.\n");
		fclose(gpFile);
	}

	//DSV(Depth Stencil View) Structure
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

	hr = gpID3D11Device->CreateDepthStencilView(pID3D11Texture2D_DepthBuffer, &depthStencilViewDesc, &gpID3D11DepthStencilView);

	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateDepthStencilView() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateDepthStencilView() Succeeded.\n");
		fclose(gpFile);
	}

	pID3D11Texture2D_DepthBuffer->Release();
	pID3D11Texture2D_DepthBuffer = NULL;

	//set render target View
	gpID3D11DeviceContext->OMSetRenderTargets(1, &gpID3D11RenderTargetView, gpID3D11DepthStencilView);

	//set viewport
	d3dViewPort.TopLeftX = 0;
	d3dViewPort.TopLeftY = 0;
	d3dViewPort.Width = (float)width;
	d3dViewPort.Height = (float)height;
	d3dViewPort.MinDepth = 0.0f;
	d3dViewPort.MaxDepth = 1.0f;

	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	giWinWidth = width;
	giWinHeight = height;

	//set perspective matrix
	gPerspectiveProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(gAnglePerspectiveDeg), (float)width / (float)height, 0.1f, 100.0f);

	return(hr);
}

void display(void)
{

	//clear render target view to a chosen color
	gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, glClearColor);

	//clear render target view for depth
	gpID3D11DeviceContext->ClearDepthStencilView(gpID3D11DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	UINT stride = sizeof(float) * 3;
	UINT offset = 0;

	/*For Sphere*/
	gpID3D11DeviceContext->IASetVertexBuffers(VDG_ATTRIBUTE_POSITION, 1, &gpID3D11Buffer_VertexBuffer_Sphere_Position, &stride, &offset);

	stride = sizeof(float) * 3;
	offset = 0;

	//for color dynamic draw of Sphere
	gpID3D11DeviceContext->IASetVertexBuffers(VDG_ATTRIBUTE_NORMAL, 1, &gpID3D11Buffer_VertexBuffer_Sphere_Normal, &stride, &offset);

	//set index buffer
	gpID3D11DeviceContext->IASetIndexBuffer(gpID3D11Buffer_IndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	//select geometry primitive
	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	CBUFFER constantBuffer;
	ZeroMemory(&constantBuffer, sizeof(CBUFFER));

	if (gbLight == true)
	{
		constantBuffer.LKeyPressed = 1;
		constantBuffer.La = XMVectorSet(lightAmbient[0], lightAmbient[1], lightAmbient[2], lightAmbient[3]);
		constantBuffer.Ld = XMVectorSet(lightDiffuse[0], lightDiffuse[1], lightDiffuse[2], lightDiffuse[3]);
		constantBuffer.Ls = XMVectorSet(lightSpecular[0], lightSpecular[1], lightSpecular[2], lightSpecular[3]);

		if (gbXaxis == true)
		{
			lightPosition[0] = 0.0f;
			lightPosition[1] = (float)100 * sin(gfangleX_axis);
			lightPosition[2] = (float)100 * cos(gfangleX_axis);
			lightPosition[3] = 1.0f;
		}

		if (gbYaxis == true)
		{
			lightPosition[0] = (float)100 * cos(gfangleY_axis);
			lightPosition[1] = 0.0f;
			lightPosition[2] = (float)100 * sin(gfangleY_axis);
			lightPosition[3] = 1.0f;
		}

		if (gbZaxis == true)
		{
			lightPosition[0] = (float)100 * cos(gfangleZ_axis);
			lightPosition[1] = (float)100 * sin(gfangleZ_axis);
			lightPosition[2] = 0.0f;
			lightPosition[3] = 1.0f;
		}

		constantBuffer.LightPosition = XMVectorSet(lightPosition[0], lightPosition[1], lightPosition[2], lightPosition[3]);
	}

	else
	{
		constantBuffer.LKeyPressed = 0;
	}

	XMMATRIX worldMatrix;
	XMMATRIX viewMatrix;
	XMMATRIX translationMatrix;
	XMMATRIX scaleMatrix;

	//Make Identity
	worldMatrix = XMMatrixIdentity();
	viewMatrix = XMMatrixIdentity();

	//translate the object in positive z axis (Left Hand Rule)
	translationMatrix = XMMatrixIdentity();
	constantBuffer.ViewMatrix = viewMatrix;
	constantBuffer.ProjectionMatrix = gPerspectiveProjectionMatrix;

	translationMatrix = XMMatrixTranslation(1.0f, 0.0f, 3.0f);
	//scaleMatrix = XMMatrixScaling(0.50f, 0.50f, 0.50f);
	worldMatrix = worldMatrix * translationMatrix;
	//load the data into the constant buffer
	constantBuffer.WorldMatrix = worldMatrix;
	
	/*SPHERE-1*/
	d3dViewPort.TopLeftX = 0;
	d3dViewPort.TopLeftY = 0;
	d3dViewPort.Width = (float)giWinWidth / 6;
	d3dViewPort.Height = (float)giWinHeight / 4;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	constantBuffer.Ka = XMVectorSet(material1_ambient[0], material1_ambient[1], material1_ambient[2], material1_ambient[3]);
	constantBuffer.Kd = XMVectorSet(material1_diffuse[0], material1_diffuse[1], material1_diffuse[2], material1_diffuse[3]);
	constantBuffer.Ks = XMVectorSet(material1_specular[0], material1_specular[1], material1_specular[2], material1_specular[3]);
	constantBuffer.material_shininess = material1_shininess;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	//draw vertex buffer to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

	/*SPHERE-2*/
	d3dViewPort.TopLeftX = 200;
	d3dViewPort.TopLeftY = 0;
	d3dViewPort.Width = (float)giWinWidth / 6;
	d3dViewPort.Height = (float)giWinHeight / 4;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	constantBuffer.Ka = XMVectorSet(material2_ambient[0], material2_ambient[1], material2_ambient[2], material2_ambient[3]);
	constantBuffer.Kd = XMVectorSet(material2_diffuse[0], material2_diffuse[1], material2_diffuse[2], material2_diffuse[3]);
	constantBuffer.Ks = XMVectorSet(material2_specular[0], material2_specular[1], material2_specular[2], material2_specular[3]);
	constantBuffer.material_shininess = material2_shininess;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	//draw vertex buffer to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

	/*SPHERE-3*/
	d3dViewPort.TopLeftX = 400;
	d3dViewPort.TopLeftY = 0;
	d3dViewPort.Width = (float)giWinWidth / 6;
	d3dViewPort.Height = (float)giWinHeight / 4;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	constantBuffer.Ka = XMVectorSet(material3_ambient[0], material3_ambient[1], material3_ambient[2], material3_ambient[3]);
	constantBuffer.Kd = XMVectorSet(material3_diffuse[0], material3_diffuse[1], material3_diffuse[2], material3_diffuse[3]);
	constantBuffer.Ks = XMVectorSet(material3_specular[0], material3_specular[1], material3_specular[2], material3_specular[3]);
	constantBuffer.material_shininess = material3_shininess;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	//draw vertex buffer to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

	/*SPHERE-4*/
	d3dViewPort.TopLeftX = 600;
	d3dViewPort.TopLeftY = 0;
	d3dViewPort.Width = (float)giWinWidth / 6;
	d3dViewPort.Height = (float)giWinHeight / 4;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	constantBuffer.Ka = XMVectorSet(material4_ambient[0], material4_ambient[1], material4_ambient[2], material4_ambient[3]);
	constantBuffer.Kd = XMVectorSet(material4_diffuse[0], material4_diffuse[1], material4_diffuse[2], material4_diffuse[3]);
	constantBuffer.Ks = XMVectorSet(material4_specular[0], material4_specular[1], material4_specular[2], material4_specular[3]);
	constantBuffer.material_shininess = material4_shininess;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	//draw vertex buffer to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

	/*SPHERE-5*/
	d3dViewPort.TopLeftX = 800;
	d3dViewPort.TopLeftY = 0;
	d3dViewPort.Width = (float)giWinWidth / 6;
	d3dViewPort.Height = (float)giWinHeight / 4;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	constantBuffer.Ka = XMVectorSet(material5_ambient[0], material5_ambient[1], material5_ambient[2], material5_ambient[3]);
	constantBuffer.Kd = XMVectorSet(material5_diffuse[0], material5_diffuse[1], material5_diffuse[2], material5_diffuse[3]);
	constantBuffer.Ks = XMVectorSet(material5_specular[0], material5_specular[1], material5_specular[2], material5_specular[3]);
	constantBuffer.material_shininess = material5_shininess;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	//draw vertex buffer to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

	/*SPHERE-6*/
	d3dViewPort.TopLeftX = 1000;
	d3dViewPort.TopLeftY = 0;
	d3dViewPort.Width = (float)giWinWidth / 6;
	d3dViewPort.Height = (float)giWinHeight / 4;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	constantBuffer.Ka = XMVectorSet(material6_ambient[0], material6_ambient[1], material6_ambient[2], material6_ambient[3]);
	constantBuffer.Kd = XMVectorSet(material6_diffuse[0], material6_diffuse[1], material6_diffuse[2], material6_diffuse[3]);
	constantBuffer.Ks = XMVectorSet(material6_specular[0], material6_specular[1], material6_specular[2], material6_specular[3]);
	constantBuffer.material_shininess = material6_shininess;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	//draw vertex buffer to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

	/*SPHERE-7*/
	d3dViewPort.TopLeftX = 0;
	d3dViewPort.TopLeftY = 200;
	d3dViewPort.Width = (float)giWinWidth / 6;
	d3dViewPort.Height = (float)giWinHeight / 4;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	constantBuffer.Ka = XMVectorSet(material7_ambient[0], material7_ambient[1], material7_ambient[2], material7_ambient[3]);
	constantBuffer.Kd = XMVectorSet(material7_diffuse[0], material7_diffuse[1], material7_diffuse[2], material7_diffuse[3]);
	constantBuffer.Ks = XMVectorSet(material7_specular[0], material7_specular[1], material7_specular[2], material7_specular[3]);
	constantBuffer.material_shininess = material7_shininess;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	//draw vertex buffer to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

	/*SPHERE-8*/
	d3dViewPort.TopLeftX = 200;
	d3dViewPort.TopLeftY = 200;
	d3dViewPort.Width = (float)giWinWidth / 6;
	d3dViewPort.Height = (float)giWinHeight / 4;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	constantBuffer.Ka = XMVectorSet(material8_ambient[0], material8_ambient[1], material8_ambient[2], material8_ambient[3]);
	constantBuffer.Kd = XMVectorSet(material8_diffuse[0], material8_diffuse[1], material8_diffuse[2], material8_diffuse[3]);
	constantBuffer.Ks = XMVectorSet(material8_specular[0], material8_specular[1], material8_specular[2], material8_specular[3]);
	constantBuffer.material_shininess = material8_shininess;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	//draw vertex buffer to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

	/*SPHERE-9*/
	d3dViewPort.TopLeftX = 400;
	d3dViewPort.TopLeftY = 200;
	d3dViewPort.Width = (float)giWinWidth / 6;
	d3dViewPort.Height = (float)giWinHeight / 4;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	constantBuffer.Ka = XMVectorSet(material9_ambient[0], material9_ambient[1], material9_ambient[2], material9_ambient[3]);
	constantBuffer.Kd = XMVectorSet(material9_diffuse[0], material9_diffuse[1], material9_diffuse[2], material9_diffuse[3]);
	constantBuffer.Ks = XMVectorSet(material9_specular[0], material9_specular[1], material9_specular[2], material9_specular[3]);
	constantBuffer.material_shininess = material9_shininess;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	//draw vertex buffer to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

	/*SPHERE-10*/
	d3dViewPort.TopLeftX = 600;
	d3dViewPort.TopLeftY = 200;
	d3dViewPort.Width = (float)giWinWidth / 6;
	d3dViewPort.Height = (float)giWinHeight / 4;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	constantBuffer.Ka = XMVectorSet(material10_ambient[0], material10_ambient[1], material10_ambient[2], material10_ambient[3]);
	constantBuffer.Kd = XMVectorSet(material10_diffuse[0], material10_diffuse[1], material10_diffuse[2], material10_diffuse[3]);
	constantBuffer.Ks = XMVectorSet(material10_specular[0], material10_specular[1], material10_specular[2], material10_specular[3]);
	constantBuffer.material_shininess = material10_shininess;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	//draw vertex buffer to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

	/*SPHERE-11*/
	d3dViewPort.TopLeftX = 800;
	d3dViewPort.TopLeftY = 200;
	d3dViewPort.Width = (float)giWinWidth / 6;
	d3dViewPort.Height = (float)giWinHeight / 4;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	constantBuffer.Ka = XMVectorSet(material11_ambient[0], material11_ambient[1], material11_ambient[2], material11_ambient[3]);
	constantBuffer.Kd = XMVectorSet(material11_diffuse[0], material11_diffuse[1], material11_diffuse[2], material11_diffuse[3]);
	constantBuffer.Ks = XMVectorSet(material11_specular[0], material11_specular[1], material11_specular[2], material11_specular[3]);
	constantBuffer.material_shininess = material11_shininess;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	//draw vertex buffer to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

	/*SPHERE-12*/
	d3dViewPort.TopLeftX = 1000;
	d3dViewPort.TopLeftY = 200;
	d3dViewPort.Width = (float)giWinWidth / 6;
	d3dViewPort.Height = (float)giWinHeight / 4;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	constantBuffer.Ka = XMVectorSet(material12_ambient[0], material12_ambient[1], material12_ambient[2], material12_ambient[3]);
	constantBuffer.Kd = XMVectorSet(material12_diffuse[0], material12_diffuse[1], material12_diffuse[2], material12_diffuse[3]);
	constantBuffer.Ks = XMVectorSet(material12_specular[0], material12_specular[1], material12_specular[2], material12_specular[3]);
	constantBuffer.material_shininess = material12_shininess;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	//draw vertex buffer to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

	/*SPHERE-13*/
	d3dViewPort.TopLeftX = 0;
	d3dViewPort.TopLeftY = 400;
	d3dViewPort.Width = (float)giWinWidth / 6;
	d3dViewPort.Height = (float)giWinHeight / 4;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	constantBuffer.Ka = XMVectorSet(material13_ambient[0], material13_ambient[1], material13_ambient[2], material13_ambient[3]);
	constantBuffer.Kd = XMVectorSet(material13_diffuse[0], material13_diffuse[1], material13_diffuse[2], material13_diffuse[3]);
	constantBuffer.Ks = XMVectorSet(material13_specular[0], material13_specular[1], material13_specular[2], material13_specular[3]);
	constantBuffer.material_shininess = material13_shininess;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	//draw vertex buffer to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

	/*SPHERE-14*/
	d3dViewPort.TopLeftX = 200;
	d3dViewPort.TopLeftY = 400;
	d3dViewPort.Width = (float)giWinWidth / 6;
	d3dViewPort.Height = (float)giWinHeight / 4;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	constantBuffer.Ka = XMVectorSet(material14_ambient[0], material14_ambient[1], material14_ambient[2], material14_ambient[3]);
	constantBuffer.Kd = XMVectorSet(material14_diffuse[0], material14_diffuse[1], material14_diffuse[2], material14_diffuse[3]);
	constantBuffer.Ks = XMVectorSet(material14_specular[0], material14_specular[1], material14_specular[2], material14_specular[3]);
	constantBuffer.material_shininess = material14_shininess;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	//draw vertex buffer to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

	/*SPHERE-15*/
	d3dViewPort.TopLeftX = 400;
	d3dViewPort.TopLeftY = 400;
	d3dViewPort.Width = (float)giWinWidth / 6;
	d3dViewPort.Height = (float)giWinHeight / 4;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	constantBuffer.Ka = XMVectorSet(material15_ambient[0], material15_ambient[1], material15_ambient[2], material15_ambient[3]);
	constantBuffer.Kd = XMVectorSet(material15_diffuse[0], material15_diffuse[1], material15_diffuse[2], material15_diffuse[3]);
	constantBuffer.Ks = XMVectorSet(material15_specular[0], material15_specular[1], material15_specular[2], material15_specular[3]);
	constantBuffer.material_shininess = material15_shininess;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	//draw vertex buffer to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

	/*SPHERE-16*/
	d3dViewPort.TopLeftX = 600;
	d3dViewPort.TopLeftY = 400;
	d3dViewPort.Width = (float)giWinWidth / 6;
	d3dViewPort.Height = (float)giWinHeight / 4;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	constantBuffer.Ka = XMVectorSet(material16_ambient[0], material16_ambient[1], material16_ambient[2], material16_ambient[3]);
	constantBuffer.Kd = XMVectorSet(material16_diffuse[0], material16_diffuse[1], material16_diffuse[2], material16_diffuse[3]);
	constantBuffer.Ks = XMVectorSet(material16_specular[0], material16_specular[1], material16_specular[2], material16_specular[3]);
	constantBuffer.material_shininess = material16_shininess;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	//draw vertex buffer to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

	/*SPHERE-17*/
	d3dViewPort.TopLeftX = 800;
	d3dViewPort.TopLeftY = 400;
	d3dViewPort.Width = (float)giWinWidth / 6;
	d3dViewPort.Height = (float)giWinHeight / 4;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	constantBuffer.Ka = XMVectorSet(material17_ambient[0], material17_ambient[1], material17_ambient[2], material17_ambient[3]);
	constantBuffer.Kd = XMVectorSet(material17_diffuse[0], material17_diffuse[1], material17_diffuse[2], material17_diffuse[3]);
	constantBuffer.Ks = XMVectorSet(material17_specular[0], material17_specular[1], material17_specular[2], material17_specular[3]);
	constantBuffer.material_shininess = material17_shininess;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	//draw vertex buffer to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

	/*SPHERE-18*/
	d3dViewPort.TopLeftX = 1000;
	d3dViewPort.TopLeftY = 400;
	d3dViewPort.Width = (float)giWinWidth / 6;
	d3dViewPort.Height = (float)giWinHeight / 4;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	constantBuffer.Ka = XMVectorSet(material18_ambient[0], material18_ambient[1], material18_ambient[2], material18_ambient[3]);
	constantBuffer.Kd = XMVectorSet(material18_diffuse[0], material18_diffuse[1], material18_diffuse[2], material18_diffuse[3]);
	constantBuffer.Ks = XMVectorSet(material18_specular[0], material18_specular[1], material18_specular[2], material18_specular[3]);
	constantBuffer.material_shininess = material18_shininess;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	//draw vertex buffer to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

	/*SPHERE-19*/
	d3dViewPort.TopLeftX = 0;
	d3dViewPort.TopLeftY = 600;
	d3dViewPort.Width = (float)giWinWidth / 6;
	d3dViewPort.Height = (float)giWinHeight / 4;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	constantBuffer.Ka = XMVectorSet(material19_ambient[0], material19_ambient[1], material19_ambient[2], material19_ambient[3]);
	constantBuffer.Kd = XMVectorSet(material19_diffuse[0], material19_diffuse[1], material19_diffuse[2], material19_diffuse[3]);
	constantBuffer.Ks = XMVectorSet(material19_specular[0], material19_specular[1], material19_specular[2], material19_specular[3]);
	constantBuffer.material_shininess = material19_shininess;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	//draw vertex buffer to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

	/*SPHERE-20*/
	d3dViewPort.TopLeftX = 200;
	d3dViewPort.TopLeftY = 600;
	d3dViewPort.Width = (float)giWinWidth / 6;
	d3dViewPort.Height = (float)giWinHeight / 4;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	constantBuffer.Ka = XMVectorSet(material20_ambient[0], material20_ambient[1], material20_ambient[2], material20_ambient[3]);
	constantBuffer.Kd = XMVectorSet(material20_diffuse[0], material20_diffuse[1], material20_diffuse[2], material20_diffuse[3]);
	constantBuffer.Ks = XMVectorSet(material20_specular[0], material20_specular[1], material20_specular[2], material20_specular[3]);
	constantBuffer.material_shininess = material20_shininess;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	//draw vertex buffer to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

	/*SPHERE-21*/
	d3dViewPort.TopLeftX = 400;
	d3dViewPort.TopLeftY = 600;
	d3dViewPort.Width = (float)giWinWidth / 6;
	d3dViewPort.Height = (float)giWinHeight / 4;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	constantBuffer.Ka = XMVectorSet(material21_ambient[0], material21_ambient[1], material21_ambient[2], material21_ambient[3]);
	constantBuffer.Kd = XMVectorSet(material21_diffuse[0], material21_diffuse[1], material21_diffuse[2], material21_diffuse[3]);
	constantBuffer.Ks = XMVectorSet(material21_specular[0], material21_specular[1], material21_specular[2], material21_specular[3]);
	constantBuffer.material_shininess = material21_shininess;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	//draw vertex buffer to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

	/*SPHERE-22*/
	d3dViewPort.TopLeftX = 600;
	d3dViewPort.TopLeftY = 600;
	d3dViewPort.Width = (float)giWinWidth / 6;
	d3dViewPort.Height = (float)giWinHeight / 4;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);
	
	constantBuffer.Ka = XMVectorSet(material22_ambient[0], material22_ambient[1], material22_ambient[2], material22_ambient[3]);
	constantBuffer.Kd = XMVectorSet(material22_diffuse[0], material22_diffuse[1], material22_diffuse[2], material22_diffuse[3]);
	constantBuffer.Ks = XMVectorSet(material22_specular[0], material22_specular[1], material22_specular[2], material22_specular[3]);
	constantBuffer.material_shininess = material22_shininess;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	//draw vertex buffer to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

	/*SPHERE-23*/
	d3dViewPort.TopLeftX = 800;
	d3dViewPort.TopLeftY = 600;
	d3dViewPort.Width = (float)giWinWidth / 6;
	d3dViewPort.Height = (float)giWinHeight / 4;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	constantBuffer.Ka = XMVectorSet(material23_ambient[0], material23_ambient[1], material23_ambient[2], material23_ambient[3]);
	constantBuffer.Kd = XMVectorSet(material23_diffuse[0], material23_diffuse[1], material23_diffuse[2], material23_diffuse[3]);
	constantBuffer.Ks = XMVectorSet(material23_specular[0], material23_specular[1], material23_specular[2], material23_specular[3]);
	constantBuffer.material_shininess = material23_shininess;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	//draw vertex buffer to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

	/*SPHERE-24*/
	d3dViewPort.TopLeftX = 1000;
	d3dViewPort.TopLeftY = 600;
	d3dViewPort.Width = (float)giWinWidth / 6;
	d3dViewPort.Height = (float)giWinHeight / 4;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	constantBuffer.Ka = XMVectorSet(material24_ambient[0], material24_ambient[1], material24_ambient[2], material24_ambient[3]);
	constantBuffer.Kd = XMVectorSet(material24_diffuse[0], material24_diffuse[1], material24_diffuse[2], material24_diffuse[3]);
	constantBuffer.Ks = XMVectorSet(material24_specular[0], material24_specular[1], material24_specular[2], material24_specular[3]);
	constantBuffer.material_shininess = material24_shininess;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	//draw vertex buffer to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

	//switch between front and back vertices
	gpIDXGISwapChain->Present(0, 0);

}

void uninitialize(void)
{
	if (gpID3D11Buffer_ConstantBuffer)
	{
		gpID3D11Buffer_ConstantBuffer->Release();
		gpID3D11Buffer_ConstantBuffer = NULL;
	}

	if (gpID3D11InputLayout)
	{
		gpID3D11InputLayout->Release();
		gpID3D11InputLayout = NULL;
	}

	if (gpID3D11RasterizerState)
	{
		gpID3D11RasterizerState->Release();
		gpID3D11RasterizerState = NULL;
	}

	if (gpID3D11Buffer_IndexBuffer)
	{
		gpID3D11Buffer_IndexBuffer->Release();
		gpID3D11Buffer_IndexBuffer = NULL;
	}

	if (gpID3D11Buffer_VertexBuffer_Sphere_Normal)
	{
		gpID3D11Buffer_VertexBuffer_Sphere_Normal->Release();
		gpID3D11Buffer_VertexBuffer_Sphere_Normal = NULL;
	}

	if (gpID3D11Buffer_VertexBuffer_Sphere_Position)
	{
		gpID3D11Buffer_VertexBuffer_Sphere_Position->Release();
		gpID3D11Buffer_VertexBuffer_Sphere_Position = NULL;
	}

	if (gpID3D11PixelShader)
	{
		gpID3D11PixelShader->Release();
		gpID3D11PixelShader = NULL;
	}

	if (gpID3D11VertexShader)
	{
		gpID3D11VertexShader->Release();
		gpID3D11VertexShader = NULL;
	}

	if (gpID3D11DepthStencilView)
	{
		gpID3D11DepthStencilView->Release();
		gpID3D11DepthStencilView = NULL;
	}

	if (gpID3D11RenderTargetView)
	{
		gpID3D11RenderTargetView->Release();
		gpID3D11RenderTargetView = NULL;
	}

	if (gpIDXGISwapChain)
	{
		gpIDXGISwapChain->Release();
		gpIDXGISwapChain = NULL;
	}

	if (gpID3D11DeviceContext)
	{
		gpID3D11DeviceContext->Release();
		gpID3D11DeviceContext = NULL;
	}

	if (gpID3D11Device)
	{
		gpID3D11Device->Release();
		gpID3D11Device = NULL;
	}

	if (gpFile)
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "uninitialize() Succeeded.\n");
		fprintf_s(gpFile, "Log File is Successfully closed.\n");
		fclose(gpFile);
	}
}

void update(void)
{
	gfangleX_axis = gfangleX_axis + 0.01f;

	if (gfangleX_axis >= 360.0f)
	{
		gfangleX_axis = gfangleX_axis - 360.0f;
	}


	gfangleY_axis = gfangleY_axis + 0.01f;

	if (gfangleY_axis >= 360.0f)
	{
		gfangleY_axis = gfangleY_axis - 360.0f;
	}

	gfangleZ_axis = gfangleZ_axis + 0.01f;

	if (gfangleZ_axis >= 360.0f)
	{
		gfangleZ_axis = gfangleZ_axis - 360.0f;
	}
}