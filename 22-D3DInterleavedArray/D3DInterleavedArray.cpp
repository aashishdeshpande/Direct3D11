#include<Windows.h>
#include<stdio.h>

#include<d3d11.h>
#include<d3dcompiler.h>

#pragma warning(disable : 4838)
#include "XNAMath\xnamath.h"
#include "WICTextureLoader.h"

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "D3dcompiler.lib")
#pragma comment (lib, "DirectXTK.lib")

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

//For Cube
ID3D11Buffer *gpID3D11Buffer_VertexBuffer_Cube_VCNT = NULL;

//For Disabling BackFace Culling
ID3D11RasterizerState *gpID3D11RasterizerState = NULL;

ID3D11InputLayout *gpID3D11InputLayout = NULL;
ID3D11Buffer *gpID3D11Buffer_ConstantBuffer = NULL;

//For Depth
ID3D11DepthStencilView *gpID3D11DepthStencilView = NULL;

//For subresources a shader can access during rendering. 
ID3D11ShaderResourceView *gpID3D11ShaderResourceView_Texture_Marble = NULL;

ID3D11SamplerState *gpID3D11SamplerState_Texture_Marble = NULL;

//rotation variables
float gAngleCube = 0.0f;
bool gbLight;

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

float la[] = { 0.0f, 0.0f, 0.0f, 1.0f };
float ld[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float ls[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float lightPosition[] = { 0.0f, 20.0f, -100.0f, 1.0f };

float ka[] = { 0.0f, 0.0f, 0.0f, 1.0f };
float kd[] = { 0.5f, 0.5f, 0.5f, 1.0f };
float ks[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float material_shininess = 50.0f;

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
		TEXT("D3D11 Window : Interleaved"),
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
	HRESULT Load3DTexture(const wchar_t *, ID3D11ShaderResourceView **);

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
		"float2 texcoord : TEXCOORD;									\n" \
		"float4 color : COLOR;											\n" \
		"};																\n" \
		"vertex_output main(float4 pos : POSITION, float4 normal : NORMAL, float4 col : COLOR, float2 tex : TEXCOORD)	\n" \
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
		"output.texcoord = tex;											\n" \
		"output.color = col;											\n" \
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
		"float2 texcoord : TEXCOORD;					\n" \
		"float4 color : COLOR;							\n" \
		"};												\n" \
		"Texture2D myTexture2D;							\n" \
		"SamplerState mySamplerState;					\n" \
		"float4 main(float4 position : SV_POSITION, vertex_output input) : SV_TARGET		\n" \
		"{																					\n" \
		"float4 phong_ads_color;															\n" \
		"float4 FragColor;																	\n" \
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
		"}				\n"
		"float4  outputTexColor =  myTexture2D.Sample(mySamplerState, input.texcoord);		\n" \
		//FragColor = phong_ads_color * input.color * myTexture2D.Sample(mySamplerState, input.texcoord);	\n 
		"FragColor = phong_ads_color * input.color * outputTexColor;	\n" \
		"return(FragColor);							\n" \
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
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[4] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, VDG_ATTRIBUTE_POSITION, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, VDG_ATTRIBUTE_COLOR, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, VDG_ATTRIBUTE_NORMAL, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, VDG_ATTRIBUTE_TEXTURE0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	

	hr = gpID3D11Device->CreateInputLayout(inputElementDesc, 4, pID3DBlob_VertexShaderCode->GetBufferPointer(), pID3DBlob_VertexShaderCode->GetBufferSize(), &gpID3D11InputLayout);
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

	/*************FOR Cube***************/
	float cubeVCNT[] = 
	{
		// SIDE 1 ( TOP )
		// triangle 1
		-1.0f, +1.0f, +1.0f,  1.0f, 0.0f, 0.0f,  0.0f, +1.0f, 0.0f, +0.0f, +0.0f,
		+1.0f, +1.0f, +1.0f,  1.0f, 0.0f, 0.0f,  0.0f, +1.0f, 0.0f, +0.0f, +1.0f,
		-1.0f, +1.0f, -1.0f,  1.0f, 0.0f, 0.0f,  0.0f, +1.0f, 0.0f, +1.0f, +0.0f,
		// triangle 2
		-1.0f, +1.0f, -1.0f,  1.0f, 0.0f, 0.0f,  0.0f, +1.0f, 0.0f, +1.0f, +0.0f,
		+1.0f, +1.0f, +1.0f,  1.0f, 0.0f, 0.0f,  0.0f, +1.0f, 0.0f, +0.0f, +1.0f,
		+1.0f, +1.0f, -1.0f,  1.0f, 0.0f, 0.0f,  0.0f, +1.0f, 0.0f, +1.0f, +1.0f,

		// SIDE 2 ( BOTTOM )
		// triangle 1
		+1.0f, -1.0f, -1.0f,  0.0f, 1.0f, 0.0f,  0.0f, -1.0f, 0.0f, +0.0f, +0.0f,
		+1.0f, -1.0f, +1.0f,  0.0f, 1.0f, 0.0f,  0.0f, -1.0f, 0.0f, +0.0f, +1.0f,
		-1.0f, -1.0f, -1.0f,  0.0f, 1.0f, 0.0f,  0.0f, -1.0f, 0.0f, +1.0f, +0.0f,
		// triangle 2
		-1.0f, -1.0f, -1.0f,  0.0f, 1.0f, 0.0f,  0.0f, -1.0f, 0.0f, +1.0f, +0.0f,
		+1.0f, -1.0f, +1.0f,  0.0f, 1.0f, 0.0f,  0.0f, -1.0f, 0.0f, +0.0f, +1.0f,
		-1.0f, -1.0f, +1.0f,  0.0f, 1.0f, 0.0f,  0.0f, -1.0f, 0.0f, +1.0f, +1.0f,

		// SIDE 3 ( FRONT )
		// triangle 1
		-1.0f, +1.0f, -1.0f, 0.0f, 0.0f, 1.0f,  0.0f, 0.0f, -1.0f, +0.0f, +0.0f,
		1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f,  0.0f, 0.0f, -1.0f, +0.0f, +1.0f,
		-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,  0.0f, 0.0f, -1.0f, +1.0f, +0.0f,
		// triangle 2
		-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,  0.0f, 0.0f, -1.0f, +1.0f, +0.0f,
		1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f,  0.0f, 0.0f, -1.0f, +0.0f, +1.0f,
		+1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,  0.0f, 0.0f, -1.0f, +1.0f, +1.0f,

		// SIDE 4 ( BACK )
		// triangle 1
		+1.0f, -1.0f, +1.0f, 0.0f, 1.0f, 1.0f,  0.0f, 0.0f, +1.0f, +0.0f, +0.0f,
		+1.0f, +1.0f, +1.0f, 0.0f, 1.0f, 1.0f,  0.0f, 0.0f, +1.0f, +0.0f, +1.0f,
		-1.0f, -1.0f, +1.0f, 0.0f, 1.0f, 1.0f,  0.0f, 0.0f, +1.0f, +1.0f, +0.0f,
		// triangle 2
		-1.0f, -1.0f, +1.0f, 0.0f, 1.0f, 1.0f,  0.0f, 0.0f, +1.0f, +1.0f, +0.0f,
		+1.0f, +1.0f, +1.0f, 0.0f, 1.0f, 1.0f,  0.0f, 0.0f, +1.0f, +0.0f, +1.0f,
		-1.0f, +1.0f, +1.0f, 0.0f, 1.0f, 1.0f,  0.0f, 0.0f, +1.0f, +1.0f, +1.0f,

		// SIDE 5 ( LEFT )
		// triangle 1
		-1.0f, +1.0f, +1.0f,  1.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f, +0.0f, +0.0f,
		-1.0f, +1.0f, -1.0f,  1.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f, +0.0f, +1.0f,
		-1.0f, -1.0f, +1.0f,  1.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f, +1.0f, +0.0f,
		// triangle 2
		-1.0f, -1.0f, +1.0f,  1.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f, +1.0f, +0.0f,
		-1.0f, +1.0f, -1.0f,  1.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f, +0.0f, +1.0f,
		-1.0f, -1.0f, -1.0f,  1.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f, +1.0f, +1.0f,

		// SIDE 6 ( RIGHT )
		// triangle 1
		+1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, +1.0f, 0.0f, 0.0f, +0.0f, +0.0f,
		+1.0f, +1.0f, -1.0f, 1.0f, 1.0f, 0.0f, +1.0f, 0.0f, 0.0f, +0.0f, +1.0f,
		+1.0f, -1.0f, +1.0f, 1.0f, 1.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, +0.0f,
		// triangle 2
		+1.0f, -1.0f, +1.0f, 1.0f, 1.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, +0.0f,
		+1.0f, +1.0f, -1.0f, 1.0f, 1.0f, 0.0f, +1.0f, 0.0f, 0.0f, +0.0f, +1.0f,
		+1.0f, +1.0f, +1.0f, 1.0f, 1.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, +1.0f
	};
	
	//create vertex buffer for Square position
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(float) * ARRAYSIZE(cubeVCNT);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc, NULL, &gpID3D11Buffer_VertexBuffer_Cube_VCNT);

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
	gpID3D11DeviceContext->Map(gpID3D11Buffer_VertexBuffer_Cube_VCNT, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubResource);
	memcpy(mappedSubResource.pData, cubeVCNT, sizeof(cubeVCNT));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_VertexBuffer_Cube_VCNT, 0);

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

	hr = Load3DTexture(L"marble.bmp", &gpID3D11ShaderResourceView_Texture_Marble);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "LoadD3DTexture() Failed For Marble.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "LoadD3DTexture() Succeeded For Marble.\n");
		fclose(gpFile);
	}

	//Create the Sample State
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	hr = gpID3D11Device->CreateSamplerState(&samplerDesc, &gpID3D11SamplerState_Texture_Marble);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateSamplerState() Failed For Marble Texture.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateSamplerState() Succeeded For Marble Texture.\n");
		fclose(gpFile);
	}

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

HRESULT Load3DTexture(const wchar_t *textureFileName, ID3D11ShaderResourceView **ppID3D11ShaderResourceView)
{
	HRESULT hr;

	//create Texture
	hr = DirectX::CreateWICTextureFromFile(gpID3D11Device, gpID3D11DeviceContext, textureFileName, NULL, ppID3D11ShaderResourceView);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "CreateWICTextureFromFile() Failed For Texture Resource.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "CreateWICTextureFromFile() Succeeded For Texture Resource.\n");
		fclose(gpFile);
	}
	return(hr);
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

	//clear render target view to a chosen color
	gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, glClearColor);

	//clear render target view for depth
	gpID3D11DeviceContext->ClearDepthStencilView(gpID3D11DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	/*For Cube*/
	UINT stride = sizeof(float) * 11;
	UINT offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(VDG_ATTRIBUTE_POSITION, 1, &gpID3D11Buffer_VertexBuffer_Cube_VCNT, &stride, &offset);

	offset = 3 * sizeof(float);
	gpID3D11DeviceContext->IASetVertexBuffers(VDG_ATTRIBUTE_COLOR, 1, &gpID3D11Buffer_VertexBuffer_Cube_VCNT, &stride, &offset);

	offset = 6 * sizeof(float);
	gpID3D11DeviceContext->IASetVertexBuffers(VDG_ATTRIBUTE_NORMAL, 1, &gpID3D11Buffer_VertexBuffer_Cube_VCNT, &stride, &offset);

	offset = 9 * sizeof(float);
	gpID3D11DeviceContext->IASetVertexBuffers(VDG_ATTRIBUTE_TEXTURE0, 1, &gpID3D11Buffer_VertexBuffer_Cube_VCNT, &stride, &offset);

	//bind texture and sampler as pixel shader resource
	gpID3D11DeviceContext->PSSetShaderResources(0, 1, &gpID3D11ShaderResourceView_Texture_Marble);
	gpID3D11DeviceContext->PSSetSamplers(0, 1, &gpID3D11SamplerState_Texture_Marble);

	//select geometry primitive
	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	CBUFFER constantBuffer;
	ZeroMemory(&constantBuffer, sizeof(CBUFFER));

	if (gbLight == true)
	{
		constantBuffer.LKeyPressed = 1;
		constantBuffer.La = XMVectorSet(la[0], la[1], la[2], la[3]);
		constantBuffer.Ld = XMVectorSet(ld[0], ld[1], ld[2], ld[3]);
		constantBuffer.Ls = XMVectorSet(ls[0], ls[1], ls[2], ls[3]);

		constantBuffer.Ka = XMVectorSet(ka[0], ka[1], ka[2], ka[3]);
		constantBuffer.Kd = XMVectorSet(kd[0], kd[1], kd[2], kd[3]);
		constantBuffer.Ks = XMVectorSet(ks[0], ks[1], ks[2], ks[3]);

		constantBuffer.material_shininess = material_shininess;
		constantBuffer.LightPosition = XMVectorSet(lightPosition[0], lightPosition[1], lightPosition[2], lightPosition[3]);
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
	translationMatrix = XMMatrixTranslation(0.0f, 0.0f, 4.0f);

	//rotate square along x-axis
	rotationMatrix = XMMatrixIdentity();

	XMMATRIX R1 = XMMatrixRotationX(gAngleCube);
	XMMATRIX R2 = XMMatrixRotationY(gAngleCube);
	XMMATRIX R3 = XMMatrixRotationZ(gAngleCube);

	rotationMatrix = R1 * R2 * R3;

	XMMATRIX scaleMatrix = XMMatrixScaling(0.75f, 0.75f, 0.75f);

	worldMatrix = worldMatrix * scaleMatrix * rotationMatrix * translationMatrix;

	//load the data into the constant buffer
	constantBuffer.WorldMatrix = worldMatrix;
	constantBuffer.ViewMatrix = viewMatrix;
	constantBuffer.ProjectionMatrix = gPerspectiveProjectionMatrix;

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);

	//draw vertex buffer to render target
	gpID3D11DeviceContext->Draw(6, 0);
	gpID3D11DeviceContext->Draw(6, 6);
	gpID3D11DeviceContext->Draw(6, 12);
	gpID3D11DeviceContext->Draw(6, 18);
	gpID3D11DeviceContext->Draw(6, 24);
	gpID3D11DeviceContext->Draw(6, 30);

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

	if (gpID3D11Buffer_VertexBuffer_Cube_VCNT)
	{
		gpID3D11Buffer_VertexBuffer_Cube_VCNT->Release();
		gpID3D11Buffer_VertexBuffer_Cube_VCNT = NULL;
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
	gAngleCube = gAngleCube + 0.0005f;
	if (gAngleCube > 360.0f)
	{
		gAngleCube = 0.0f;
	}
}