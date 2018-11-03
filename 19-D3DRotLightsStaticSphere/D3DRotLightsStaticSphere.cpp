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

//For Per Vertex
ID3D11VertexShader *gpID3D11VertexShader_PerVertex = NULL;
ID3D11PixelShader *gpID3D11PixelShader_PerVertex = NULL;

//For Per Pixel
ID3D11VertexShader *gpID3D11VertexShader_PerPixel = NULL;
ID3D11PixelShader *gpID3D11PixelShader_PerPixel = NULL;


//For Sphere
ID3D11Buffer *gpID3D11Buffer_VertexBuffer_Sphere_Position = NULL;
ID3D11Buffer *gpID3D11Buffer_VertexBuffer_Sphere_Normal = NULL;
ID3D11Buffer *gpID3D11Buffer_IndexBuffer = NULL;

//For Disabling BackFace Culling
ID3D11RasterizerState *gpID3D11RasterizerState = NULL;

ID3D11InputLayout *gpID3D11InputLayout_PerVertex = NULL;
ID3D11InputLayout *gpID3D11InputLayout_PerPixel = NULL;

ID3D11Buffer *gpID3D11Buffer_ConstantBuffer = NULL;

//For Depth
ID3D11DepthStencilView *gpID3D11DepthStencilView = NULL;

bool gbLight;
bool gbPerVertex = true;
bool gbPerFragment = false;


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

	XMVECTOR Ld0;
	XMVECTOR La0;
	XMVECTOR Ls0;

	XMVECTOR Kd0;
	XMVECTOR Ka0;
	XMVECTOR Ks0;

	XMVECTOR Ld1;
	XMVECTOR La1;
	XMVECTOR Ls1;

	XMVECTOR Kd1;
	XMVECTOR Ka1;
	XMVECTOR Ks1;

	XMVECTOR Ld2;
	XMVECTOR La2;
	XMVECTOR Ls2;

	XMVECTOR Kd2;
	XMVECTOR Ka2;
	XMVECTOR Ks2;

	XMVECTOR Light0Position;
	XMVECTOR Light1Position;
	XMVECTOR Light2Position;

	float material0Shininess;
	float material1Shininess;
	float material2Shininess;

	unsigned int LKeyPressed;
};

XMMATRIX gPerspectiveProjectionMatrix;

float lightAmbient0[] = { 0.0f, 0.0f, 0.0f, 1.0f };
float lightDiffuse0[] = { 1.0f, 0.0f, 0.0f, 1.0f };
float lightSpecular0[] = { 1.0f, 0.0f, 0.0f, 1.0f };

float lightAmbient1[] = { 0.0f, 0.0f, 0.0f, 1.0f };
float lightDiffuse1[] = { 0.0f, 1.0f, 0.0f, 1.0f };
float lightSpecular1[] = { 0.0f, 1.0f, 0.0f, 1.0f };

float lightAmbient2[] = { 0.0f, 0.0f, 0.0f, 1.0f };
float lightDiffuse2[] = { 0.0f, 0.0f, 1.0f, 1.0f };
float lightSpecular2[] = { 0.0f, 0.0f, 1.0f, 1.0f };

float material_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
float material_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float material_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

float material_shininess = 50.0f;

float gfangle_red = 0.0f;
float gfangle_green = 0.0f;
float gfangle_blue = 0.0f;


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
		TEXT("D3D11 Window : Rotating Lights Static Sphere"),
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
		case 0x51:
			if (gbEscapeKeyIsPressed == false)
			{
				gbEscapeKeyIsPressed = true;
			}
			break;

		case VK_ESCAPE:
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

		case 0x56:
			if (gbPerVertex == false)
			{
				gbPerVertex = true;
				gbPerFragment = false;
			}
			break;

		case 0x46:
			if (gbPerFragment == false)
			{
				gbPerFragment = true;
				gbPerVertex = false;
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

	/*VERTEX SHADER PER VERTEX*/
	const char *vertexShaderSourceCode_perVertex =
		"cbuffer ConstantBuffer" \
		"{"
		"float4x4 worldMatrix;" \
		"float4x4 viewMatrix;" \
		"float4x4 projectionMatrix;" \
		"float4 ld0;" \
		"float4 la0;" \
		"float4 ls0;" \
		"float4 kd0;" \
		"float4 ka0;" \
		"float4 ks0;" \
		"float4 ld1;" \
		"float4 la1;" \
		"float4 ls1;" \
		"float4 kd1;" \
		"float4 ka1;" \
		"float4 ks1;" \
		"float4 ld2;" \
		"float4 la2;" \
		"float4 ls2;" \
		"float4 kd2;" \
		"float4 ka2;" \
		"float4 ks2;" \
		"float4 light0Position;" \
		"float4 light1Position;" \
		"float4 light2Position;" \
		"float material0Shininess;" \
		"float material1Shininess;" \
		"float material2Shininess;" \
		"unsigned int lKeyPressed;" \
		"}" \
		"struct vertex_output" \
		"{" \
		"float4 position : SV_POSITION;" \
		"float4 phong_ads_color : COLOR;" \
		"};" \
		"vertex_output main(float4 pos : POSITION, float4 normal : NORMAL)" \
		"{" \
		"vertex_output output;" \
		"if(lKeyPressed == 1)" \
		"{" \
		"float4x4 worldMulview = mul(worldMatrix, viewMatrix);" \
		"float4 eyeCoordinates = mul(worldMulview, pos);" \
		"float3 transformed_normals = normalize(mul((float3x3)(mul(viewMatrix, worldMatrix)), (float3)normal));" \
		"float3 light_direction0 = normalize((float3)light0Position - eyeCoordinates.xyz);" \
		"float3 light_direction1 = normalize((float3)light1Position - eyeCoordinates.xyz);" \
		"float3 light_direction2 = normalize((float3)light2Position - eyeCoordinates.xyz);" \
		"float tn_dot_ld0 = max(dot(transformed_normals, light_direction0), 0.0);" \
		"float tn_dot_ld1 = max(dot(transformed_normals, light_direction1), 0.0);" \
		"float tn_dot_ld2 = max(dot(transformed_normals, light_direction2), 0.0);" \
		"float4 ambient0 = la0 * ka0;" \
		"float4 diffuse0 = ld0 * kd0 * tn_dot_ld0;" \
		"float3 reflection_vector0 = reflect(-light_direction0, transformed_normals);" \
		"float4 ambient1 = la1 * ka1;" \
		"float4 diffuse1 = ld1 * kd1 * tn_dot_ld1;" \
		"float3 reflection_vector1 = reflect(-light_direction1, transformed_normals);" \
		"float4 ambient2 = la2 * ka2;" \
		"float4 diffuse2 = ld2 * kd2 * tn_dot_ld2;" \
		"float3 reflection_vector2 = reflect(-light_direction2, transformed_normals);" \
		"float3 viewer_vector = normalize(-eyeCoordinates.xyz);" \
		"float4 specular0 = ls0 * ks0 * pow(max(dot(reflection_vector0, viewer_vector), 0.0), material0Shininess);" \
		"float4 specular1 = ls1 * ks1 * pow(max(dot(reflection_vector1, viewer_vector), 0.0), material1Shininess);" \
		"float4 specular2 = ls2 * ks2 * pow(max(dot(reflection_vector2, viewer_vector), 0.0), material2Shininess);" \
		"float4 phong_ads_color0 = ambient0 + diffuse0 + specular0;" \
		"float4 phong_ads_color1 = ambient1 + diffuse1 + specular1;" \
		"float4 phong_ads_color2 = ambient2 + diffuse2 + specular2;" \
		"output.phong_ads_color = (phong_ads_color0 + phong_ads_color1 + phong_ads_color2);" \
		"}" \
		"else" \
		"{" \
		"output.phong_ads_color = float4(1.0, 1.0, 1.0, 1.0);" \
		"}" \
		"float4x4 mulpos = mul(worldMatrix, viewMatrix);" \
		"output.position = mul(mul(projectionMatrix, mulpos), pos);" \
		"return(output);" \
		"}";

	ID3DBlob *pID3DBlob_VertexShaderCode_PerVertex = NULL;
	ID3DBlob *pID3DBlob_Error = NULL;

	hr = D3DCompile(vertexShaderSourceCode_perVertex,
		lstrlenA(vertexShaderSourceCode_perVertex) + 1,
		"VS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		0,
		0,
		&pID3DBlob_VertexShaderCode_PerVertex,
		&pID3DBlob_Error);

	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "D3DCompile() Failed For Vertex Shader Per Vertex : %s.\n", (char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
			return(hr);
		}
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3DCompile() Succeeded For Vertex Shader. For Per Vertex\n");
		fclose(gpFile);
	}

	hr = gpID3D11Device->CreateVertexShader(pID3DBlob_VertexShaderCode_PerVertex->GetBufferPointer(), pID3DBlob_VertexShaderCode_PerVertex->GetBufferSize(), NULL, &gpID3D11VertexShader_PerVertex);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateVertexShader() Failed For Per Vertex.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateVertexShader() Succeeded For Per Vertex.\n");
		fclose(gpFile);
	}

	/*VERTEX SHADER PER PIXEL*/

	const char *vertexShaderSourceCode_perPixel =
		"cbuffer ConstantBuffer" \
		"{"
		"float4x4 worldMatrix;" \
		"float4x4 viewMatrix;" \
		"float4x4 projectionMatrix;" \
		"float4 ld0;" \
		"float4 la0;" \
		"float4 ls0;" \
		"float4 kd0;" \
		"float4 ka0;" \
		"float4 ks0;" \
		"float4 ld1;" \
		"float4 la1;" \
		"float4 ls1;" \
		"float4 kd1;" \
		"float4 ka1;" \
		"float4 ks1;" \
		"float4 ld2;" \
		"float4 la2;" \
		"float4 ls2;" \
		"float4 kd2;" \
		"float4 ka2;" \
		"float4 ks2;" \
		"float4 light0Position;" \
		"float4 light1Position;" \
		"float4 light2Position;" \
		"float material0Shininess;" \
		"float material1Shininess;" \
		"float material2Shininess;" \
		"unsigned int lKeyPressed;" \
		"}" \
		"struct vertex_output											\n" \
		"{																\n" \
		"float4 position : SV_POSITION;									\n" \
		"float3 transformed_normals : NORMAL0;							\n" \
		"float3 light_direction0 : NORMAL1;								\n" \
		"float3 light_direction1 : NORMAL2;								\n" \
		"float3 light_direction2 : NORMAL3;								\n" \
		"float3 viewer_vector : NORMAL4;								\n" \
		"};																\n" \
		"vertex_output main(float4 pos : POSITION, float4 normal : NORMAL)	\n" \
		"{																\n" \
		"vertex_output output;											\n" \
		"if(lKeyPressed == 1)											\n" \
		"{																\n" \
		"float4x4 worldMulview = mul(worldMatrix, viewMatrix);			\n" \
		"float4 eyeCoordinates = mul(worldMulview, pos);				\n" \
		"output.transformed_normals = mul((float3x3)(mul(viewMatrix, worldMatrix)), (float3)normal);		\n" \
		"output.light_direction0 = ((float3)(light0Position) - eyeCoordinates.xyz);							\n" \
		"output.light_direction1 = ((float3)(light1Position) - eyeCoordinates.xyz);							\n" \
		"output.light_direction2 = ((float3)(light2Position) - eyeCoordinates.xyz);							\n" \
		"output.viewer_vector = (-eyeCoordinates.xyz);					\n" \
		"}																\n" \
		"float4x4 mulpos = mul(worldMatrix, viewMatrix);				\n" \
		"output.position = mul(mul(projectionMatrix, mulpos), pos);		\n" \
		"return(output);												\n" \
		"}";

	ID3DBlob *pID3DBlob_VertexShaderCode_PerPixel = NULL;
	pID3DBlob_Error = NULL;

	hr = D3DCompile(vertexShaderSourceCode_perPixel,
		lstrlenA(vertexShaderSourceCode_perPixel) + 1,
		"VS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		0,
		0,
		&pID3DBlob_VertexShaderCode_PerPixel,
		&pID3DBlob_Error);

	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "D3DCompile() Failed For Vertex Shader For Per Pixel : %s.\n", (char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
			return(hr);
		}
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3DCompile() Succeeded For Vertex Shader For Per Pixel.\n");
		fclose(gpFile);
	}

	hr = gpID3D11Device->CreateVertexShader(pID3DBlob_VertexShaderCode_PerPixel->GetBufferPointer(), pID3DBlob_VertexShaderCode_PerPixel->GetBufferSize(), NULL, &gpID3D11VertexShader_PerPixel);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateVertexShader() Failed For Per Pixel.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateVertexShader() Succeeded For Per Pixel.\n");
		fclose(gpFile);
	}


	//Set the Vertex Shader as per the Key 
	/*if (gbPerVertex == true)
	{
	gpID3D11DeviceContext->VSSetShader(gpID3D11VertexShader_PerVertex, 0, 0);
	}
	else if(gbPerFragment == true)
	{
	gpID3D11DeviceContext->VSSetShader(gpID3D11VertexShader_PerPixel, 0, 0);
	}*/


	/**********FOR PIXEL SHADER**********/


	/*PIXEL SHADER PER VERTEX*/

	const char *pixelShaderSourceCode_perVertex =
		"float4 main(float4 position : SV_POSITION,  float4 phong_ads_color : COLOR) : SV_TARGET" \
		"{" \
		"float4 outputColor;				\n" \
		"outputColor = phong_ads_color;	\n" \
		"return(outputColor);			\n" \
		"}";

	ID3DBlob *pID3DBlob_PixelShaderCode_PerVertex = NULL;
	pID3DBlob_Error = NULL;

	hr = D3DCompile(pixelShaderSourceCode_perVertex,
		lstrlenA(pixelShaderSourceCode_perVertex) + 1,
		"PS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		0,
		0,
		&pID3DBlob_PixelShaderCode_PerVertex,
		&pID3DBlob_Error);

	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "D3DCompile() Failed For Pixel Shader Per Vertex: %s.\n", (char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
			return(hr);
		}
	}

	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3DCompile() Succeeded For Pixel Shader Per Vertex.\n");
		fclose(gpFile);
	}

	hr = gpID3D11Device->CreatePixelShader(pID3DBlob_PixelShaderCode_PerVertex->GetBufferPointer(), pID3DBlob_PixelShaderCode_PerVertex->GetBufferSize(), NULL, &gpID3D11PixelShader_PerVertex);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreatePixelShader() Failed Per Vertex.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreatePixelShader() Succeeded Per Vertex.\n");
		fclose(gpFile);
	}


	/*PIXEL SHADER PER PIXEL*/
	const char *pixelShaderSourceCode_perPixel =
		"cbuffer ConstantBuffer" \
		"{"
		"float4x4 worldMatrix;" \
		"float4x4 viewMatrix;" \
		"float4x4 projectionMatrix;" \
		"float4 ld0;" \
		"float4 la0;" \
		"float4 ls0;" \
		"float4 kd0;" \
		"float4 ka0;" \
		"float4 ks0;" \
		"float4 ld1;" \
		"float4 la1;" \
		"float4 ls1;" \
		"float4 kd1;" \
		"float4 ka1;" \
		"float4 ks1;" \
		"float4 ld2;" \
		"float4 la2;" \
		"float4 ls2;" \
		"float4 kd2;" \
		"float4 ka2;" \
		"float4 ks2;" \
		"float4 light0Position;" \
		"float4 light1Position;" \
		"float4 light2Position;" \
		"float material0Shininess;" \
		"float material1Shininess;" \
		"float material2Shininess;" \
		"unsigned int lKeyPressed;" \
		"}" \
		"struct vertex_output											\n" \
		"{																\n" \
		"float4 position : SV_POSITION;									\n" \
		"float3 transformed_normals : NORMAL0;							\n" \
		"float3 light_direction0 : NORMAL1;								\n" \
		"float3 light_direction1 : NORMAL2;								\n" \
		"float3 light_direction2 : NORMAL3;								\n" \
		"float3 viewer_vector : NORMAL4;								\n" \
		"};																\n" \
		"float4 main(float4 position : SV_POSITION, vertex_output input) : SV_TARGET		\n" \
		"{																					\n" \
		"float4 phong_ads_color;															\n" \
		"if(lKeyPressed == 1)																\n" \
		"{																					\n" \
		"float3 normalized_transformed_normals = normalize(input.transformed_normals);			\n" \
		"float3 normalized_light_direction0 = normalize(input.light_direction0);				\n" \
		"float3 normalized_light_direction1 = normalize(input.light_direction1);				\n" \
		"float3 normalized_light_direction2 = normalize(input.light_direction2);				\n" \
		"float3 normalized_viewer_vector = normalize(input.viewer_vector);						\n" \
		"float4 ambient0 = la0 * ka0;															\n" \
		"float tn_dot_ld0 = max(dot(normalized_transformed_normals, normalized_light_direction0), 0.0);		\n" \
		"float4 diffuse0 = ld0 * kd0 * tn_dot_ld0;												\n" \
		"float4 ambient1 = la1 * ka1;															\n" \
		"float tn_dot_ld1 = max(dot(normalized_transformed_normals, normalized_light_direction1), 0.0);		\n" \
		"float4 diffuse1 = ld1 * kd1 * tn_dot_ld1;												\n" \
		"float4 ambient2 = la2 * ka2;															\n" \
		"float tn_dot_ld2 = max(dot(normalized_transformed_normals, normalized_light_direction2), 0.0);		\n" \
		"float4 diffuse2 = ld2 * kd2 * tn_dot_ld2;												\n" \
		"float3 reflection_vector0 = reflect(-normalized_light_direction0, normalized_transformed_normals);	\n" \
		"float3 reflection_vector1 = reflect(-normalized_light_direction1, normalized_transformed_normals);	\n" \
		"float3 reflection_vector2 = reflect(-normalized_light_direction2, normalized_transformed_normals);	\n" \
		"float4 specular0 = ls0 * ks0 * pow(max(dot(reflection_vector0, normalized_viewer_vector), 0.0), material0Shininess);	\n" \
		"float4 specular1 = ls1 * ks1 * pow(max(dot(reflection_vector1, normalized_viewer_vector), 0.0), material1Shininess);	\n" \
		"float4 specular2 = ls2 * ks2 * pow(max(dot(reflection_vector2, normalized_viewer_vector), 0.0), material2Shininess);	\n" \
		"float4 phong_ads_color0 = ambient0 + diffuse0 + specular0;									\n" 	
		"float4 phong_ads_color1 = ambient1 + diffuse1 + specular1;									\n" \
		"float4 phong_ads_color2 = ambient2 + diffuse2 + specular2;									\n" \
		"phong_ads_color = phong_ads_color0 + phong_ads_color1 + phong_ads_color2;									\n" \
		"}													\n"
		"else												\n" \
		"{													\n" \
		"phong_ads_color = float4(1.0f, 1.0f, 1.0f, 1.0f);	\n" \
		"}													\n"
		"return(phong_ads_color);							\n" \
		"}";

	ID3DBlob *pID3DBlob_PixelShaderCode_PerPixel = NULL;
	pID3DBlob_Error = NULL;

	hr = D3DCompile(pixelShaderSourceCode_perPixel,
		lstrlenA(pixelShaderSourceCode_perPixel) + 1,
		"PS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		0,
		0,
		&pID3DBlob_PixelShaderCode_PerPixel,
		&pID3DBlob_Error);

	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "D3DCompile() Failed For Pixel Shader Per Pixel : %s.\n", (char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
			return(hr);
		}
	}

	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3DCompile() Succeeded For Pixel Shader Per Pixel.\n");
		fclose(gpFile);
	}

	hr = gpID3D11Device->CreatePixelShader(pID3DBlob_PixelShaderCode_PerPixel->GetBufferPointer(), pID3DBlob_PixelShaderCode_PerPixel->GetBufferSize(), NULL, &gpID3D11PixelShader_PerPixel);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreatePixelShader() Failed Per Pixel.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreatePixelShader() Succeeded Per Pixel.\n");
		fclose(gpFile);
	}

	//Set Pixel Shader According to the key Pressed
	/*if (gbPerVertex == true)
	{
	gpID3D11DeviceContext->PSSetShader(gpID3D11PixelShader_PerVertex, 0, 0);
	}
	else if(gbPerFragment == true)
	{
	gpID3D11DeviceContext->PSSetShader(gpID3D11PixelShader_PerPixel, 0, 0);
	}*/

	pID3DBlob_PixelShaderCode_PerPixel->Release();
	pID3DBlob_PixelShaderCode_PerVertex->Release();

	pID3DBlob_PixelShaderCode_PerPixel = NULL;
	pID3DBlob_PixelShaderCode_PerVertex = NULL;

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

	if (gbPerVertex == true)
	{

		hr = gpID3D11Device->CreateInputLayout(inputElementDesc, 2, pID3DBlob_VertexShaderCode_PerVertex->GetBufferPointer(), pID3DBlob_VertexShaderCode_PerVertex->GetBufferSize(), &gpID3D11InputLayout_PerVertex);
		if (FAILED(hr))
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "ID3D11Device::CreateInputLayout() Failed For Per Vertex.\n");
			fclose(gpFile);
			return(hr);
		}
		else
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "ID3D11Device::CreateInputLayout() Succeeded For Per Vertex.\n");
			fclose(gpFile);
		}

		gpID3D11DeviceContext->IASetInputLayout(gpID3D11InputLayout_PerVertex);
		pID3DBlob_VertexShaderCode_PerVertex->Release();
		pID3DBlob_VertexShaderCode_PerVertex = NULL;
	}


	else if (gbPerFragment == true)
	{
		hr = gpID3D11Device->CreateInputLayout(inputElementDesc, 2, pID3DBlob_VertexShaderCode_PerPixel->GetBufferPointer(), pID3DBlob_VertexShaderCode_PerPixel->GetBufferSize(), &gpID3D11InputLayout_PerPixel);
		if (FAILED(hr))
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "ID3D11Device::CreateInputLayout() Failed For Per Pixel.\n");
			fclose(gpFile);
			return(hr);
		}
		else
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "ID3D11Device::CreateInputLayout() Succeeded For Per Pixel.\n");
			fclose(gpFile);
		}

		gpID3D11DeviceContext->IASetInputLayout(gpID3D11InputLayout_PerPixel);
		pID3DBlob_VertexShaderCode_PerPixel->Release();
		pID3DBlob_VertexShaderCode_PerPixel = NULL;
	}


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
	glClearColor[0] = 0.0f;
	glClearColor[1] = 0.0f;
	glClearColor[2] = 0.0f;
	glClearColor[3] = 1.0f;

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
	D3D11_VIEWPORT d3dViewPort;
	d3dViewPort.TopLeftX = 0;
	d3dViewPort.TopLeftY = 0;
	d3dViewPort.Width = (float)width;
	d3dViewPort.Height = (float)height;
	d3dViewPort.MinDepth = 0.0f;
	d3dViewPort.MaxDepth = 1.0f;

	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	//set perspective matrix
	gPerspectiveProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(gAnglePerspectiveDeg), (float)width / (float)height, 0.1f, 100.0f);

	return(hr);
}

void display(void)
{

	if (gbPerVertex == true)
	{
		gpID3D11DeviceContext->VSSetShader(gpID3D11VertexShader_PerVertex, 0, 0);
		gpID3D11DeviceContext->PSSetShader(gpID3D11PixelShader_PerVertex, 0, 0);
	}
	else if (gbPerFragment == true)
	{
		gpID3D11DeviceContext->VSSetShader(gpID3D11VertexShader_PerPixel, 0, 0);
		gpID3D11DeviceContext->PSSetShader(gpID3D11PixelShader_PerPixel, 0, 0);
	}

	float lightPosition0[4] = { 0.0f, (float)200 * sin(gfangle_red), (float)200 * cos(gfangle_red), 1.0f };
	float lightPosition1[4] = { (float)200 * cos(gfangle_green), 0.0f, (float)200 * sin(gfangle_green), 1.0 };
	float lightPosition2[4] = { (float)200 * cos(gfangle_blue) , (float)200 * sin(gfangle_blue), 0.0f, 1.0 };

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
		constantBuffer.La0 = XMVectorSet(lightAmbient0[0], lightAmbient0[1], lightAmbient0[2], lightAmbient0[3]);
		constantBuffer.Ld0 = XMVectorSet(lightDiffuse0[0], lightDiffuse0[1], lightDiffuse0[2], lightDiffuse0[3]);
		constantBuffer.Ls0 = XMVectorSet(lightSpecular0[0], lightSpecular0[1], lightSpecular0[2], lightSpecular0[3]);

		constantBuffer.La1 = XMVectorSet(lightAmbient1[0], lightAmbient1[1], lightAmbient1[2], lightAmbient1[3]);
		constantBuffer.Ld1 = XMVectorSet(lightDiffuse1[0], lightDiffuse1[1], lightDiffuse1[2], lightDiffuse1[3]);
		constantBuffer.Ls1 = XMVectorSet(lightSpecular1[0], lightSpecular1[1], lightSpecular1[2], lightSpecular1[3]);

		constantBuffer.La2 = XMVectorSet(lightAmbient2[0], lightAmbient2[1], lightAmbient2[2], lightAmbient2[3]);
		constantBuffer.Ld2 = XMVectorSet(lightDiffuse2[0], lightDiffuse2[1], lightDiffuse2[2], lightDiffuse2[3]);
		constantBuffer.Ls2 = XMVectorSet(lightSpecular2[0], lightSpecular2[1], lightSpecular2[2], lightSpecular2[3]);

		constantBuffer.Ka0 = XMVectorSet(material_ambient[0], material_ambient[1], material_ambient[2], material_ambient[3]);
		constantBuffer.Kd0 = XMVectorSet(material_diffuse[0], material_diffuse[1], material_diffuse[2], material_diffuse[3]);
		constantBuffer.Ks0 = XMVectorSet(material_specular[0], material_specular[1], material_specular[2], material_specular[3]);

		constantBuffer.Ka1 = XMVectorSet(material_ambient[0], material_ambient[1], material_ambient[2], material_ambient[3]);
		constantBuffer.Kd1 = XMVectorSet(material_diffuse[0], material_diffuse[1], material_diffuse[2], material_diffuse[3]);
		constantBuffer.Ks1 = XMVectorSet(material_specular[0], material_specular[1], material_specular[2], material_specular[3]);

		constantBuffer.Ka2 = XMVectorSet(material_ambient[0], material_ambient[1], material_ambient[2], material_ambient[3]);
		constantBuffer.Kd2 = XMVectorSet(material_diffuse[0], material_diffuse[1], material_diffuse[2], material_diffuse[3]);
		constantBuffer.Ks2 = XMVectorSet(material_specular[0], material_specular[1], material_specular[2], material_specular[3]);

		constantBuffer.material0Shininess = material_shininess;
		constantBuffer.material1Shininess = material_shininess;
		constantBuffer.material2Shininess = material_shininess;

		constantBuffer.Light0Position = XMVectorSet(lightPosition0[0], lightPosition0[1], lightPosition0[2], lightPosition0[3]);
		constantBuffer.Light1Position = XMVectorSet(lightPosition1[0], lightPosition1[1], lightPosition1[2], lightPosition1[3]);
		constantBuffer.Light2Position = XMVectorSet(lightPosition2[0], lightPosition2[1], lightPosition2[2], lightPosition2[3]);
	}

	else
	{
		constantBuffer.LKeyPressed = 0;
	}

	XMMATRIX worldMatrix;
	XMMATRIX viewMatrix;
	XMMATRIX translationMatrix;
	XMMATRIX rotationMatrix;

	//Make Identity
	worldMatrix = XMMatrixIdentity();
	viewMatrix = XMMatrixIdentity();

	//translate the object in positive z axis (Left Hand Rule)
	translationMatrix = XMMatrixIdentity();
	translationMatrix = XMMatrixTranslation(0.0f, 0.0f, 2.0f);

	worldMatrix = worldMatrix * translationMatrix;

	//load the data into the constant buffer
	constantBuffer.WorldMatrix = worldMatrix;
	constantBuffer.ViewMatrix = viewMatrix;
	constantBuffer.ProjectionMatrix = gPerspectiveProjectionMatrix;

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

	if (gpID3D11InputLayout_PerPixel)
	{
		gpID3D11InputLayout_PerPixel->Release();
		gpID3D11InputLayout_PerPixel = NULL;
	}

	if (gpID3D11InputLayout_PerVertex)
	{
		gpID3D11InputLayout_PerVertex->Release();
		gpID3D11InputLayout_PerVertex = NULL;
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

	if (gpID3D11PixelShader_PerPixel)
	{
		gpID3D11PixelShader_PerPixel->Release();
		gpID3D11PixelShader_PerPixel = NULL;
	}

	if (gpID3D11PixelShader_PerVertex)
	{
		gpID3D11PixelShader_PerVertex->Release();
		gpID3D11PixelShader_PerVertex = NULL;
	}

	if (gpID3D11VertexShader_PerPixel)
	{
		gpID3D11VertexShader_PerPixel->Release();
		gpID3D11VertexShader_PerPixel = NULL;
	}

	if (gpID3D11VertexShader_PerVertex)
	{
		gpID3D11VertexShader_PerVertex->Release();
		gpID3D11VertexShader_PerVertex = NULL;
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
	gfangle_red = gfangle_red + 0.005f;
	if (gfangle_red > 360.0f)
		gfangle_red = gfangle_red - 360.0f;

	gfangle_green = gfangle_green + 0.005f;
	if (gfangle_green > 360.0f)
		gfangle_green = gfangle_green - 360.0f;

	gfangle_blue = gfangle_blue + 0.005f;
	if (gfangle_blue > 360.0f)
		gfangle_blue = gfangle_blue - 360.0f;
}