#include <d3dx9.h>
#include <dinput.h>
#include <vector>
#include <thread>
#include <DirectXMath.h>
#include <DirectXCollision.h>


#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")

using namespace DirectX;

///////////////// TYPES /////////////////

// цвета
enum class Colors
{
	None, White, Yellow, Red, Green, Blue, Orange
};

// вершина
struct Vertex
{
	D3DXVECTOR3 pos;
	D3DCOLOR color;
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
IDirect3D9* direct3d;
IDirect3DDevice9* device3d;
IDirectInput8* directInput;
IDirectInputDevice8* keyboard, *mouse;
UINT screenWidth, screenHeight;
constexpr float cubeRadius = 1.0f;
Part parts[3][3][3];
constexpr float distance = cubeRadius * 2.2f;
BYTE keyStates[256];
D3DXQUATERNION fullCubeRotation;
constexpr int rotateSpeed = 45; // градусов
float rotSpeed;
std::thread graphicsThread, inputThread, gameThread;
bool threadEnabled;
int animateTime;
bool animateEnable;
constexpr int animationFullTime = 1000;
D3DXVECTOR3 animatev;
float animatea, animateStep;
BoundingOrientedBox fullBound, partBound;


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
	default:
		return DefWindowProc(h, m, w, l);
	}
	return 0;
}
// инициализаци€ окна
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
// инициализаци€ графики
bool InitialGraphics() {
	if (!(direct3d = Direct3DCreate9(D3D_SDK_VERSION)))
		return false;
	D3DPRESENT_PARAMETERS pp{ 0 };
	pp.BackBufferFormat = D3DFMT_X8R8G8B8;
	pp.BackBufferWidth = screenWidth;
	pp.BackBufferHeight = screenHeight;
	pp.AutoDepthStencilFormat = D3DFMT_D24S8;
	pp.EnableAutoDepthStencil = TRUE;
	pp.hDeviceWindow = hwnd;
	pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	pp.Windowed = TRUE;
	if (FAILED(direct3d->CreateDevice(0, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, &pp, &device3d)))
		return false;
	device3d->SetRenderState(D3DRS_LIGHTING, FALSE);
	return true;
}
// инициализаци€ клавиатуры и мыши
void InitialInput() {
	DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&directInput, nullptr);
	directInput->CreateDevice(GUID_SysKeyboard, &keyboard, nullptr);
	keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	keyboard->SetDataFormat(&c_dfDIKeyboard);
	keyboard->Acquire();
}
// инициализаци€ устройств
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
// »нициализаци€ игры
void InitialGame() {
	CreateCube();
	D3DXMATRIX m;
	D3DXQuaternionRotationMatrix(&fullCubeRotation, D3DXMatrixIdentity(&m));

	fullBound = BoundingOrientedBox(XMFLOAT3{ cubeRadius, cubeRadius, cubeRadius}, )
}

// инициализаци€ обща€
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

void RotateCube(D3DXVECTOR3 vec, float angle) {
	D3DXQUATERNION q;
	D3DXQuaternionRotationAxis(&q, &vec, D3DXToRadian(angle));
	fullCubeRotation *= q;
}

void AnimateRotateCube(D3DXVECTOR3 vec, float angle) {
	animatev = vec;
	animatea = angle;
	StartAnimation();
}

// Ќажатие клавиши
void KeyDown(BYTE key) {
	switch (key)
	{
	case DIK_ESCAPE:
		SendMessage(hwnd, WM_CLOSE, 0, 0);
		break;
	case DIK_A:
		AnimateRotateCube({ 0.0f, 1.0f, 0.0f }, -rotateSpeed);
		break;
	case DIK_D:
		AnimateRotateCube({ 0.0f, 1.0f ,0.0f }, rotateSpeed);
		break;
	case DIK_W:
		AnimateRotateCube({ 1.0f, 0.0f, 0.0f }, -rotateSpeed);
		break;
	case DIK_S:
		AnimateRotateCube({ 1.0f, 0.0f ,0.0f }, rotateSpeed);
		break;
	}
}

// отпускание клавиши
void KeyUp(BYTE key) {

}

// удержание клавиши
void KeyPress(BYTE key) {
	// KeyDown(key);
}

// обновление устройств ввода
void UpdateInput() {
	BYTE keys[256];
	keyboard->GetDeviceState(256, keys);
	if (memcmp(keys, keyStates, 256)) {
		for (size_t i = 0; i < 256; i++)
		{
			if (keys[i] != keyStates[i]) {
				if (keys[i] & 0x80) {
					KeyDown(i);
				}
				else {
					KeyUp(keys[i]);
				}
				keyStates[i] = keys[i];
			}
		}
	}
	else {
		for (size_t i = 0; i < 256; i++)
		{
			if (keys[i] & 0x80) {
				KeyPress(i);
			}
		}
	}
}

// цвет в Direct3D формат
D3DCOLOR toD3DColor(Colors color) {
	switch (color)
	{
	case Colors::None:
		return 0x00000000;
	case Colors::White:
		return 0xffffffff;
	case Colors::Yellow:
		return 0xffffff00;
	case Colors::Red:
		return 0xffff0000;
	case Colors::Green:
		return 0xff00ff00;
	case Colors::Blue:
		return 0xff0000ff;
	case Colors::Orange:
		return 0xffff7700;
	default:
		return 0x00000000;
	}
}
// рисует куб
void DrawCube(D3DXVECTOR3 pos, D3DXVECTOR3 rot) {
	// рисуем куб с центром pos
	D3DXVECTOR3 mini = pos - D3DXVECTOR3{ cubeRadius ,cubeRadius ,cubeRadius },
		maxi = pos + D3DXVECTOR3{ cubeRadius ,cubeRadius ,cubeRadius };
	int indices[]{
		0,1,2, 0,2,3,
		4,5,6, 4,6,7,
		8,9,10, 8,10,11,
		12,13,14, 12,14,15,
		16,17,18, 16,18,19,
		20,21,22, 20,22,23
	};

	Vertex vertices[]{
		{{mini.x, maxi.y, maxi.z}, toD3DColor(Colors::Orange)},
		{{mini.x, maxi.y, mini.z}, toD3DColor(Colors::Orange)},
		{{mini.x, mini.y, mini.z}, toD3DColor(Colors::Orange)},
		{{mini.x, mini.y, maxi.z}, toD3DColor(Colors::Orange)},

		{{mini.x, mini.y, mini.z}, toD3DColor(Colors::White)},
		{{maxi.x, mini.y, mini.z}, toD3DColor(Colors::White)},
		{{maxi.x, mini.y, maxi.z}, toD3DColor(Colors::White)},
		{{mini.x, mini.y, maxi.z}, toD3DColor(Colors::White)},

		{{mini.x, maxi.y, mini.z}, toD3DColor(Colors::Blue)},
		{{maxi.x, maxi.y, mini.z}, toD3DColor(Colors::Blue)},
		{{maxi.x, mini.y, mini.z}, toD3DColor(Colors::Blue)},
		{{mini.x, mini.y, mini.z}, toD3DColor(Colors::Blue)},

		{{maxi.x, maxi.y, mini.z}, toD3DColor(Colors::Red)},
		{{maxi.x, maxi.y, maxi.z}, toD3DColor(Colors::Red)},
		{{maxi.x, mini.y, maxi.z}, toD3DColor(Colors::Red)},
		{{maxi.x, mini.y, mini.z}, toD3DColor(Colors::Red)},

		{{mini.x, maxi.y, maxi.z}, toD3DColor(Colors::Yellow)},
		{{maxi.x, maxi.y, maxi.z}, toD3DColor(Colors::Yellow)},
		{{maxi.x, maxi.y, mini.z}, toD3DColor(Colors::Yellow)},
		{{mini.x, maxi.y, mini.z}, toD3DColor(Colors::Yellow)},

		{{maxi.x, maxi.y, maxi.z}, toD3DColor(Colors::Green)},
		{{mini.x, maxi.y, maxi.z}, toD3DColor(Colors::Green)},
		{{mini.x, mini.y, maxi.z}, toD3DColor(Colors::Green)},
		{{maxi.x, mini.y, maxi.z}, toD3DColor(Colors::Green)},

	};

	device3d->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	device3d->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 36, 12, indices, D3DFMT_INDEX32, vertices, sizeof(Vertex));
}
// обновление графики
void UpdateGraphics() {
	device3d->Clear(0, nullptr, D3DCLEAR_STENCIL | D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff404040, 1.0f, 0);
	device3d->BeginScene();

	D3DXMATRIX m;
	const D3DXVECTOR3 offset{ distance , distance , distance };
	device3d->SetTransform(D3DTS_VIEW, D3DXMatrixLookAtLH(&m, &(D3DXVECTOR3(0, 0, -20) /*+ offset*/), &(D3DXVECTOR3(0, 0, 0) /*+ offset*/), &D3DXVECTOR3(0, 1, 0)));
	device3d->SetTransform(D3DTS_PROJECTION, D3DXMatrixPerspectiveFovLH(&m, D3DX_PI*0.25f, (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f));

	device3d->SetTransform(D3DTS_WORLD, D3DXMatrixRotationQuaternion(&m, &fullCubeRotation));

	for (auto&& cx : parts) {
		for (auto&& cy : cx) {
			for (auto&& cz : cy) {
				D3DXVECTOR3 pos{ cz.indices[0] * distance, cz.indices[1] * distance, cz.indices[2] * distance };
				DrawCube(pos - offset, { 0.0f, 0.0f, 0.0f });
			}
		}
	}
	/*DrawCube({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f },
		{ Colors::White, Colors::Blue, Colors::Green, Colors::Orange, Colors::Red, Colors::Yellow });*/

	device3d->EndScene();
	device3d->Present(nullptr, nullptr, nullptr, nullptr);
}
// обновление игры
void UpdateGame(int elapsedTime) {
	// обновл€ем анимацию
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
	device3d = nullptr;
	direct3d = nullptr;
}

// точка входа
int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE, LPSTR, int) {
	::hinstance = hinstance;
	Initialize();
	Run();
	Release();
	return 0;
}