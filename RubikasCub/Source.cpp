#include <d3d11.h>
#include <dinput.h>
#include <vector>
#include <thread>
#include <fstream>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <SpriteFont.h>
#include <Keyboard.h>
#include <Mouse.h>


#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")



///////////////// TYPES /////////////////

// цвета
enum class Colors
{
	None, White, Yellow, Red, Green, Blue, Orange
};

using Colors = ::Colors;

// вершина
struct Vertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 color;
};

struct MatrixPerFrame {
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX proj;
};

struct MatrixPerModel {
	DirectX::XMMATRIX world;
};

struct RECTF
{
	float left, top, right, bottom;
};
// часть кубика
struct Part
{
	std::vector<int> indices;
};

// плоскость
struct Plane
{
	//std::vector<int>
};

//////////////// GLOBALS ///////////////
HINSTANCE hinstance;
HWND hwnd;
LPCSTR windowClassName = "TEST_WINDOW";
ID3D11Device* device3d;
ID3D11DeviceContext* context3d;
IDXGISwapChain* swapChain;
ID3D11RenderTargetView* renderTargetView;
//IDirectInput8* directInput;
//IDirectInputDevice8* keyboard, *mouse;
UINT screenWidth, screenHeight;
constexpr float cubeRadius = 1.0f;
Part parts[3][3][3];
constexpr float distance = cubeRadius * 2.2f;
//BYTE keyStates[256];
DirectX::XMVECTOR fullCubeRotation;
constexpr int rotateSpeed = 45; // градусов
float rotSpeed;
std::thread graphicsThread, inputThread, gameThread;
bool threadEnabled;
int animateTime;
bool animateEnable;
constexpr int animationFullTime = 1000;
DirectX::XMVECTOR animatev;
float animatea, animateStep;
DirectX::BoundingOrientedBox fullBound, partBound;
ID3D11InputLayout* inputLayout, *guiInputLayout;
ID3D11VertexShader* vertexShader, *guiVertexShader;
ID3D11PixelShader* pixelShader;
ID3D11Buffer* indexBuffer, *vertexBuffer, *matrixBufferPerFrame, *matrixBufferPerModel;
MatrixPerFrame perFrameMatrix;
MatrixPerModel perModelMatrix;
ID3D11DepthStencilView* depthView;
ID3D11Texture2D* depthBuffer;
ID3D11Buffer* guiVertexBuffer, *guiIndexBuffer;
RECTF shufleRect;
//DIMOUSESTATE mouseState;
//POINT mousePosition;
std::wstring shufleText = L"Shufle", timerText;
DirectX::SpriteFont* spriteFont;
DirectX::SpriteBatch *spriteBatch;
DirectX::Keyboard *keyboard;
ID3D11DepthStencilState* depthStencilState;
DirectX::Mouse *mouse;


///////////////// RELEASE //////////////

// обработчик событий окна
LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
	switch (m)
	{
		//case WM_CLOSE:
		//	DestroyWindow(hwnd);
		//	break;
	case WM_DESTROY:
		PostQuitMessage(WM_QUIT);
		break;
	case WM_ACTIVATEAPP:
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		keyboard->ProcessMessage(m, w, l);
		mouse->ProcessMessage(m, w, l);
		break;
	default:
		return DefWindowProc(h, m, w, l);
	}
	return 0;
}
// инициализация окна
void InitialWindow() {
	WNDCLASS wc{ 0 };
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hInstance = hinstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = windowClassName;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&wc);

	hwnd = CreateWindow(windowClassName, "", WS_POPUPWINDOW, 0, 0, screenWidth, screenHeight, HWND_DESKTOP, nullptr, hinstance, nullptr);
	ShowWindow(hwnd, SW_NORMAL);
	UpdateWindow(hwnd);
}
// инициализация графики
bool InitialGraphics() {
	DXGI_SWAP_CHAIN_DESC scd{ 0 };
	scd.BufferCount = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.Height = screenHeight;
	scd.BufferDesc.RefreshRate.Numerator = 1;
	scd.BufferDesc.RefreshRate.Denominator = 60;
	scd.BufferDesc.Width = screenWidth;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = hwnd;
	scd.SampleDesc.Count = 1;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Windowed = true;

	D3D_FEATURE_LEVEL levels[]{ D3D_FEATURE_LEVEL_11_0 };
	D3D_FEATURE_LEVEL level;
	if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_DEBUG, levels, 1, D3D11_SDK_VERSION, &scd, &swapChain, &device3d, &level, &context3d)))
		return false;

	ID3D11Texture2D* tex;
	HRESULT hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&tex);

	hr = device3d->CreateRenderTargetView(tex, nullptr, &renderTargetView);
	tex->Release();

	D3D11_TEXTURE2D_DESC td{ 0 };
	td.Width = screenWidth;
	td.Height = screenHeight;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	td.SampleDesc.Count = 1;
	td.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	device3d->CreateTexture2D(&td, nullptr, &depthBuffer);
	device3d->CreateDepthStencilView(depthBuffer, nullptr, &depthView);

	context3d->OMSetRenderTargets(1, &renderTargetView, depthView);

	D3D11_VIEWPORT viewport{ 0 };
	viewport.Width = screenWidth;
	viewport.Height = screenHeight;
	viewport.MaxDepth = 1.0f;

	context3d->RSSetViewports(1, &viewport);

	// create of raster state
	CD3D11_DEPTH_STENCIL_DESC dpd;
	ZeroMemory(&dpd, sizeof(dpd));
	dpd.DepthEnable = true;
	dpd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dpd.DepthFunc = D3D11_COMPARISON_LESS;
	dpd.StencilEnable = true;
	dpd.StencilReadMask = dpd.StencilWriteMask = 0xff;

	dpd.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dpd.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dpd.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dpd.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	dpd.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dpd.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dpd.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dpd.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	device3d->CreateDepthStencilState(&dpd, &depthStencilState);

	return true;
}
// инициализация клавиатуры и мыши
void InitialInput() {
	/*DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&directInput, nullptr);
	directInput->CreateDevice(GUID_SysKeyboard, &keyboard, nullptr);
	keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	keyboard->SetDataFormat(&c_dfDIKeyboard);
	keyboard->Acquire();
	directInput->CreateDevice(GUID_SysMouse, &mouse, nullptr);
	mouse->SetCooperativeLevel(hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	mouse->SetDataFormat(&c_dfDIMouse);
	mouse->Acquire();
	::SetCursorPos(0, 0);*/

	keyboard = new DirectX::Keyboard();
	mouse = new DirectX::Mouse();
	mouse->SetMode(DirectX::Mouse::Mode::MODE_ABSOLUTE);
	mouse->SetWindow(hwnd);
}
// инициализация устройств
void InitialCore() {
	screenHeight = GetSystemMetrics(SM_CYSCREEN);
	screenWidth = GetSystemMetrics(SM_CXSCREEN);

	InitialWindow();
	InitialGraphics();
	InitialInput();
}

// создает собранный куб
void CreateCube() {
	parts[0][0][0] = { {0,0,0} };
	parts[0][0][1] = { {0,0,1} };
	parts[0][0][2] = { {0,0,2} };

	parts[0][1][0] = { {0,1,0} };
	parts[0][1][1] = { {0,1,1} };
	parts[0][1][2] = { {0,1,2} };

	parts[0][2][0] = { {0,2,0} };
	parts[0][2][1] = { {0,2,1} };
	parts[0][2][2] = { {0,2,2} };


	parts[1][0][0] = { {1,0,0} };
	parts[1][0][1] = { {1,0,1} };
	parts[1][0][2] = { {1,0,2} };

	parts[1][1][0] = { {1,1,0} };
	parts[1][1][1] = { {1,1,1} };
	parts[1][1][2] = { {1,1,2} };

	parts[1][2][0] = { {1,2,0} };
	parts[1][2][1] = { {1,2,1} };
	parts[1][2][2] = { {1,2,2} };

	parts[2][0][0] = { {2,0,0} };
	parts[2][0][1] = { {2,0,1} };
	parts[2][0][2] = { {2,0,2} };

	parts[2][1][0] = { {2,1,0} };
	parts[2][1][1] = { {2,1,1} };
	parts[2][1][2] = { {2,1,2} };

	parts[2][2][0] = { {2,2,0} };
	parts[2][2][1] = { {2,2,1} };
	parts[2][2][2] = { {2,2,2} };
}

// загружает откомпилированный шейдер из файла
std::vector<BYTE> LoadFile(std::string filename) {
	std::ifstream file{ filename };
	if (file.bad() || !file.good())
		throw std::exception(("Cannot to open shader file: " + filename).c_str());
	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	file.seekg(0, std::ios::beg);
	std::vector<BYTE> output(size);
	file.read((char*)output.data(), size);
	return std::move(output);
}

// цвет в Direct3D формат
DirectX::XMFLOAT4 toD3DColor(Colors color) {
	switch (color)
	{
	case Colors::None:
		return { 0.0f, 0.0f, 0.0f, 0.0f };
	case Colors::White:
		return { 1.0f, 1.0f, 1.0f, 1.0f };
	case Colors::Yellow:
		return { 1.0f, 1.0f, 0.0f, 1.0f };
	case Colors::Red:
		return { 1.0f, 0.0f, 0.0f, 1.0f };
	case Colors::Green:
		return { 0.0f, 1.0f, 0.0f, 1.0f };
	case Colors::Blue:
		return { 0.0f, 0.0f, 1.0f, 1.0f };
	case Colors::Orange:
		return { 1.0f, 0.5f, 0.0f, 1.0f };
	default:
		return { 0.0f, 0.0f, 0.0f, 0.0f };
	}
}

void CreateUserInterface() {
	Vertex vertices[]{
		{DirectX::XMFLOAT3(shufleRect.left, shufleRect.top, 0.1f), toD3DColor(Colors::White)},
		{DirectX::XMFLOAT3(shufleRect.right, shufleRect.top, 0.1f), toD3DColor(Colors::White)},
		{DirectX::XMFLOAT3(shufleRect.right, shufleRect.bottom, 0.1f), toD3DColor(Colors::White)},
		{DirectX::XMFLOAT3(shufleRect.left, shufleRect.bottom, 0.1f), toD3DColor(Colors::White)},
	};
	D3D11_BUFFER_DESC desc{ 0 };
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.ByteWidth = sizeof(Vertex) * 4;
	D3D11_SUBRESOURCE_DATA sub;
	sub.pSysMem = vertices;
	device3d->CreateBuffer(&desc, &sub, &guiVertexBuffer);
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.ByteWidth = sizeof(int) * 6;
	int indices[]{ 0,1,2, 0,2,3 };
	sub.pSysMem = indices;
	device3d->CreateBuffer(&desc, &sub, &guiIndexBuffer);

	// создаем шрифт
	spriteFont = new DirectX::SpriteFont(device3d, L"..\\Fonts\\consolas.font");
	spriteBatch = new DirectX::SpriteBatch(context3d);
}


// Инициализация игры
void InitialGame() {
	shufleRect = { -0.9f, 0.9f, -0.7f, 0.8f };

	CreateCube();
	fullCubeRotation = DirectX::XMQuaternionIdentity();

	//fullBound = BoundingOrientedBox(XMFLOAT3{ cubeRadius, cubeRadius, cubeRadius}, )

	D3D11_BUFFER_DESC desc{ 0 };
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.ByteWidth = sizeof(int) * 36;
	desc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	D3D11_SUBRESOURCE_DATA sub{ 0 };
	int indices[]{
		0,1,2, 0,2,3,
		4,5,6, 4,6,7,
		8,9,10, 8,10,11,
		12,13,14, 12,14,15,
		16,17,18, 16,18,19,
		20,21,22, 20,22,23
	};
	sub.pSysMem = indices;
	device3d->CreateBuffer(&desc, &sub, &indexBuffer);

	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.ByteWidth = sizeof(Vertex) * 24;

	DirectX::XMVECTOR mini = DirectX::XMVECTOR{ -cubeRadius, -cubeRadius, -cubeRadius },
		maxi = DirectX::XMVECTOR{ cubeRadius, cubeRadius, cubeRadius };

	Vertex vertices[]{
		{DirectX::XMFLOAT3(DirectX::XMVectorGetX(mini), DirectX::XMVectorGetY(maxi), DirectX::XMVectorGetZ(maxi)), toD3DColor(Colors::Orange)},
		{DirectX::XMFLOAT3(DirectX::XMVectorGetX(mini), DirectX::XMVectorGetY(maxi), DirectX::XMVectorGetZ(mini)), toD3DColor(Colors::Orange)},
		{DirectX::XMFLOAT3(DirectX::XMVectorGetX(mini), DirectX::XMVectorGetY(mini), DirectX::XMVectorGetZ(mini)), toD3DColor(Colors::Orange)},
		{DirectX::XMFLOAT3(DirectX::XMVectorGetX(mini), DirectX::XMVectorGetY(mini), DirectX::XMVectorGetZ(maxi)), toD3DColor(Colors::Orange)},

		{DirectX::XMFLOAT3(DirectX::XMVectorGetX(mini), DirectX::XMVectorGetY(mini), DirectX::XMVectorGetZ(mini)), toD3DColor(Colors::White)},
		{DirectX::XMFLOAT3(DirectX::XMVectorGetX(maxi), DirectX::XMVectorGetY(mini), DirectX::XMVectorGetZ(mini)), toD3DColor(Colors::White)},
		{DirectX::XMFLOAT3(DirectX::XMVectorGetX(maxi), DirectX::XMVectorGetY(mini), DirectX::XMVectorGetZ(maxi)), toD3DColor(Colors::White)},
		{DirectX::XMFLOAT3(DirectX::XMVectorGetX(mini), DirectX::XMVectorGetY(mini), DirectX::XMVectorGetZ(maxi)), toD3DColor(Colors::White)},

		{DirectX::XMFLOAT3(DirectX::XMVectorGetX(mini), DirectX::XMVectorGetY(maxi), DirectX::XMVectorGetZ(mini)), toD3DColor(Colors::Blue)},
		{DirectX::XMFLOAT3(DirectX::XMVectorGetX(maxi), DirectX::XMVectorGetY(maxi), DirectX::XMVectorGetZ(mini)), toD3DColor(Colors::Blue)},
		{DirectX::XMFLOAT3(DirectX::XMVectorGetX(maxi), DirectX::XMVectorGetY(mini), DirectX::XMVectorGetZ(mini)), toD3DColor(Colors::Blue)},
		{DirectX::XMFLOAT3(DirectX::XMVectorGetX(mini), DirectX::XMVectorGetY(mini), DirectX::XMVectorGetZ(mini)), toD3DColor(Colors::Blue)},

		{DirectX::XMFLOAT3(DirectX::XMVectorGetX(maxi), DirectX::XMVectorGetY(maxi), DirectX::XMVectorGetZ(mini)), toD3DColor(Colors::Red)},
		{DirectX::XMFLOAT3(DirectX::XMVectorGetX(maxi), DirectX::XMVectorGetY(maxi), DirectX::XMVectorGetZ(maxi)), toD3DColor(Colors::Red)},
		{DirectX::XMFLOAT3(DirectX::XMVectorGetX(maxi), DirectX::XMVectorGetY(mini), DirectX::XMVectorGetZ(maxi)), toD3DColor(Colors::Red)},
		{DirectX::XMFLOAT3(DirectX::XMVectorGetX(maxi), DirectX::XMVectorGetY(mini), DirectX::XMVectorGetZ(mini)), toD3DColor(Colors::Red)},

		{DirectX::XMFLOAT3(DirectX::XMVectorGetX(mini), DirectX::XMVectorGetY(maxi), DirectX::XMVectorGetZ(maxi)), toD3DColor(Colors::Yellow)},
		{DirectX::XMFLOAT3(DirectX::XMVectorGetX(maxi), DirectX::XMVectorGetY(maxi), DirectX::XMVectorGetZ(maxi)), toD3DColor(Colors::Yellow)},
		{DirectX::XMFLOAT3(DirectX::XMVectorGetX(maxi), DirectX::XMVectorGetY(maxi), DirectX::XMVectorGetZ(mini)), toD3DColor(Colors::Yellow)},
		{DirectX::XMFLOAT3(DirectX::XMVectorGetX(mini), DirectX::XMVectorGetY(maxi), DirectX::XMVectorGetZ(mini)), toD3DColor(Colors::Yellow)},

		{DirectX::XMFLOAT3(DirectX::XMVectorGetX(maxi), DirectX::XMVectorGetY(maxi), DirectX::XMVectorGetZ(maxi)), toD3DColor(Colors::Green)},
		{DirectX::XMFLOAT3(DirectX::XMVectorGetX(mini), DirectX::XMVectorGetY(maxi), DirectX::XMVectorGetZ(maxi)), toD3DColor(Colors::Green)},
		{DirectX::XMFLOAT3(DirectX::XMVectorGetX(mini), DirectX::XMVectorGetY(mini), DirectX::XMVectorGetZ(maxi)), toD3DColor(Colors::Green)},
		{DirectX::XMFLOAT3(DirectX::XMVectorGetX(maxi), DirectX::XMVectorGetY(mini), DirectX::XMVectorGetZ(maxi)), toD3DColor(Colors::Green)},

	};

	/*Vertex testTertices[]{
		{XMFLOAT3(100, 100, 0), toD3DColor(Colors::Orange)},
		{XMFLOAT3(200, 100, 0), toD3DColor(Colors::Orange)},
		{XMFLOAT3(200, 200, 0), toD3DColor(Colors::Orange)},
		{XMFLOAT3(100, 200, 0), toD3DColor(Colors::Orange)},
	};*/

	sub.pSysMem = vertices;
	//sub.pSysMem = testTertices;
	device3d->CreateBuffer(&desc, &sub, &vertexBuffer);

	// грузим шейдеры
	auto vs = LoadFile("..\\Shaders\\Simple3DVS.cso");
	D3D11_INPUT_ELEMENT_DESC inputElements[]{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	device3d->CreateInputLayout(inputElements, 2, vs.data(), vs.size(), &inputLayout);

	device3d->CreateVertexShader(vs.data(), vs.size(), nullptr, &vertexShader);

	auto ps = LoadFile("..\\Shaders\\Simple3DPS.cso");

	device3d->CreatePixelShader(ps.data(), ps.size(), nullptr, &pixelShader);

	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = sizeof(MatrixPerFrame);
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.Usage = D3D11_USAGE_DYNAMIC;

	device3d->CreateBuffer(&desc, nullptr, &matrixBufferPerFrame);

	desc.ByteWidth = sizeof(MatrixPerModel);

	device3d->CreateBuffer(&desc, nullptr, &matrixBufferPerModel);

	vs = LoadFile("..\\Shaders\\GUI_VS.cso");
	device3d->CreateInputLayout(inputElements, 2, vs.data(), vs.size(), &guiInputLayout);
	device3d->CreateVertexShader(vs.data(), vs.size(), nullptr, &guiVertexShader);

	CreateUserInterface();
}

// инициализация общая
void Initialize() {
	InitialCore();
	InitialGame();
}

void StartAnimation() {
	animateStep = animatea;
	animateStep /= animationFullTime;
	animateTime = 0;
	animateEnable = true;
}

void RotateCube(DirectX::XMVECTOR vec, float angle) {
	DirectX::XMVECTOR q = DirectX::XMQuaternionRotationAxis(vec, DirectX::XMConvertToRadians(angle));
	fullCubeRotation = DirectX::XMVector4Dot(fullCubeRotation, q);
}

void AnimateRotateCube(DirectX::XMVECTOR vec, float angle) {
	animatev = vec;
	animatea = angle;
	StartAnimation();
}

bool HitTest(RECT rect, POINT point) {
	return point.x >= rect.left && point.x < rect.right
		&& point.y >= rect.top && point.y < rect.bottom;
}

float horizontal(float v) {
	return (v + 1) * screenWidth / 2.0f;
}

float vertical(float v) {
	return (-v + 1) * screenHeight / 2.0f;
}

RECT toClient(RECTF r) {
	return { (LONG)horizontal(r.left),
		(LONG)vertical(r.top),
		(LONG)horizontal(r.right),
		(LONG)vertical(r.bottom) };
}

void MouseDown(BYTE key) {
	// проверяем попали ли в gui
	/*if (HitTest(toClient(shufleRect), mousePosition)) {
		MessageBox(0, "Do shufle!", 0, 0);
	}*/
}

void MouseUp(BYTE key) {

}

// обновление устройств ввода
void UpdateInput() {
	auto keyState = keyboard->GetState();

	if (keyState.Escape)
		SendMessage(hwnd, WM_CLOSE, 0, 0);

	if (keyState.A)
		AnimateRotateCube({ 0.0f, 1.0f, 0.0f }, -rotateSpeed);
	if (keyState.D)
		AnimateRotateCube({ 0.0f, 1.0f ,0.0f }, rotateSpeed);

	if (keyState.W)
		AnimateRotateCube({ 1.0f, 0.0f, 0.0f }, -rotateSpeed);
	if (keyState.S)
		AnimateRotateCube({ 1.0f, 0.0f ,0.0f }, rotateSpeed);

	auto mouseState = mouse->GetState();
	if (mouseState.leftButton) {
		if (HitTest(toClient(shufleRect), { mouseState.x, mouseState.y })) {
			MessageBox(0, "Do shufle!", 0, 0);
		}
	}
}

template <typename T>
void UpdateBuffer(ID3D11Buffer* buffer, T* data) {
	D3D11_MAPPED_SUBRESOURCE map;
	context3d->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
	memcpy(map.pData, data, sizeof(T));
	context3d->Unmap(buffer, 0);
}

// рисует куб
void DrawCube(DirectX::XMVECTOR pos, DirectX::XMVECTOR rot, DirectX::XMMATRIX parentTransform) {
	DirectX::XMMATRIX m = DirectX::XMMatrixTranslation(DirectX::XMVectorGetX(pos), DirectX::XMVectorGetY(pos), DirectX::XMVectorGetZ(pos));
	perModelMatrix.world = DirectX::XMMatrixTranspose(parentTransform * m);
	UpdateBuffer(matrixBufferPerModel, &perModelMatrix);

	context3d->VSSetConstantBuffers(1, 1, &matrixBufferPerModel);

	//context3d->Draw(4, 0);
	context3d->DrawIndexed(36, 0, 0);
}

// рисовает интерфейс
void DrawInterface() {
	context3d->VSSetShader(guiVertexShader, nullptr, 0);
	UINT strides = sizeof(Vertex), offsets = 0;
	context3d->IASetVertexBuffers(0, 1, &guiVertexBuffer, &strides, &offsets);
	context3d->IASetIndexBuffer(guiIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	context3d->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context3d->DrawIndexed(6, 0, 0);

	spriteBatch->Begin();
	auto r = toClient(shufleRect);
	auto r1 = spriteFont->MeasureDrawBounds(shufleText.c_str(), DirectX::XMFLOAT2(r.left, r.top));
	spriteFont->DrawString(spriteBatch, shufleText.c_str(), DirectX::XMFLOAT2(r1.left, r1.top),
		DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f));
	spriteBatch->End();
}

// обновление графики
void UpdateGraphics() {
	float color[]{ 0.5f, 0.5f, 0.5f, 1.0f };
	context3d->ClearRenderTargetView(renderTargetView, color);
	context3d->ClearDepthStencilView(depthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	perFrameMatrix.view = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH({ 0.0f, 0.0f, -20.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }));
	perFrameMatrix.proj = DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f));

	UpdateBuffer(matrixBufferPerFrame, &perFrameMatrix);
	context3d->VSSetConstantBuffers(0, 1, &matrixBufferPerFrame);


	//device3d->SetTransform(D3DTS_WORLD, D3DXMatrixRotationQuaternion(&m, &fullCubeRotation));

	context3d->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context3d->IASetInputLayout(inputLayout);
	context3d->IASetIndexBuffer(indexBuffer, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	UINT strides = sizeof(Vertex), offsets = 0;
	context3d->IASetVertexBuffers(0, 1, &vertexBuffer, &strides, &offsets);


	context3d->VSSetShader(vertexShader, nullptr, 0);
	context3d->PSSetShader(pixelShader, nullptr, 0);

	auto commonMatrix = DirectX::XMMatrixRotationQuaternion(fullCubeRotation);

	context3d->OMSetDepthStencilState(depthStencilState, 1);

	for (auto&& cx : parts) {
		for (auto&& cy : cx) {
			for (auto&& cz : cy) {
				DirectX::XMVECTOR pos{ cz.indices[0] * ::distance, cz.indices[1] * ::distance, cz.indices[2] * ::distance };
				DrawCube(pos/* - offset*/, { 0.0f, 0.0f, 0.0f }, commonMatrix);
			}
		}
	}

	DrawInterface();

	swapChain->Present(0, 0);
}
// обновление игры
void UpdateGame(int elapsedTime) {
	// обновляем анимацию
	if (animateEnable) {
		auto real = animateTime + elapsedTime;
		if (real >= animationFullTime) {
			real %= animationFullTime;
			animateEnable = false;
		}
		else {
			real = elapsedTime;
		}
		animateTime += real;
		RotateCube(animatev, animateStep * real);
	}
}

void GraphicsThread() {
	while (threadEnabled)
	{
		UpdateGraphics();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
void InputThread() {
	while (threadEnabled)
	{
		UpdateInput();
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}
void GameThread() {
	auto oldTime = timeGetTime();
	while (threadEnabled)
	{
		auto newTime = timeGetTime();
		auto elapsed = newTime - oldTime;
		oldTime = newTime;
		UpdateGame(elapsed);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

// запуск
void Run() {
	threadEnabled = true;
	graphicsThread = std::thread{ GraphicsThread };
	inputThread = std::thread{ InputThread };
	gameThread = std::thread{ GameThread };

	MSG msg{ 0 };
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	threadEnabled = false;
	if (gameThread.joinable())
		gameThread.join();
	if (inputThread.joinable())
		inputThread.join();
	if (graphicsThread.joinable())
		graphicsThread.join();
}

// удаление данных
void Release() {
	depthStencilState->Release();
	delete spriteFont;
	delete spriteBatch;
	guiInputLayout->Release();
	guiVertexBuffer->Release();
	guiIndexBuffer->Release();
	guiVertexShader->Release();
	depthView->Release();
	depthBuffer->Release();
	inputLayout->Release();
	vertexShader->Release();
	pixelShader->Release();
	indexBuffer->Release();
	vertexBuffer->Release();
	matrixBufferPerFrame->Release();
	matrixBufferPerModel->Release();
	/*keyboard->Release();
	mouse->Release();
	directInput->Release();*/
	delete keyboard;
	delete mouse;
}

// точка входа
int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE, LPSTR, int) {
	::hinstance = hinstance;
	Initialize();
	Run();
	Release();
	return 0;
}