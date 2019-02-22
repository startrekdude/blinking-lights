#include "BlinkingLights.h"

#include <random>

#define WIN32_LEAN_AND_MEAN
#include <objbase.h>
#include <shellapi.h>
#include <shlobj.h>
#include <windows.h>
#include <windowsx.h>

#include <gdiplus.h>

using namespace Gdiplus;
using namespace std;

#define MAX_LOADSTRING 32
#define IDT_MAIN 12

WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

ATOM                RegisterWndClass(HINSTANCE hInstance);
BOOL CALLBACK       CreateWindowForMonitor(HMONITOR, HDC, LPRECT, LPARAM);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
					 _In_opt_ HINSTANCE hPrevInstance,
					 _In_ LPWSTR    lpCmdLine,
					 _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

#ifdef SCREENSAVER
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (argc == 1 || (argc == 2 && wcsstr(argv[1], L"/c") == argv[1])) {
		MessageBoxW(GetForegroundWindow(), L"The blinking lights screensaver does not have any settings.",
			L"Blinking Lights Screensaver", MB_ICONWARNING);
		return 0;
	}
	else if (argc == 3 && wcscmp(argv[1], L"/p") == 0) {
		return 0;
	}
#endif

	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	
	ULONG_PTR gdiToken;
	GdiplusStartupInput gdiInput;
	GdiplusStartup(&gdiToken, &gdiInput, NULL);

	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_BLINKINGLIGHTS, szWindowClass, MAX_LOADSTRING);
	RegisterWndClass(hInstance);

	EnumDisplayMonitors(NULL, NULL, CreateWindowForMonitor, (LPARAM)nCmdShow);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	GdiplusShutdown(gdiToken);

	CoUninitialize();
	return (int) msg.wParam;
}

ATOM RegisterWndClass(HINSTANCE hInstance)
{
	HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BLINKINGLIGHTS));

	WNDCLASSEXW wcex = {
		sizeof(WNDCLASSEXW), // cbSize
		0,                   // style
		WndProc,             // lpfnWndProc
		0,                   // cbClsExtra
		0,                   // cbWndExtra
		hInstance,           // hInstance
		hIcon,               // hIcon
		NULL,                // hCursor
		NULL,                // hbrBackground
		NULL,                // lpszMenuName
		szWindowClass,       // lpszClassName
		hIcon                // hIconSm
	};

	return RegisterClassExW(&wcex);
}

BOOL CALLBACK CreateWindowForMonitor(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	MONITORINFO mi = { sizeof(MONITORINFO) };
	GetMonitorInfo(hMonitor, &mi);
	HWND hWnd = CreateWindowW(
		szWindowClass,                          // lpClassName
		szTitle,                                // lpTitle
		WS_POPUP,                               // dwStyle
		mi.rcMonitor.left,                      // x
		mi.rcMonitor.top,                       // y
		mi.rcMonitor.right - mi.rcMonitor.left, // nWidth
		mi.rcMonitor.bottom - mi.rcMonitor.top, // nHeight
		NULL,                                   // hWndParent
		NULL,                                   // hMenu
		GetModuleHandle(NULL),                  // hInstance
		NULL                                    // lpParam
	);

	ShowWindow(hWnd, (int)dwData);
	UpdateWindow(hWnd);

	return TRUE;
}

GraphicsPath* CreateRoundedRect(RECT nRect) {
	Rect rect = Rect(nRect.left, nRect.top, nRect.right - nRect.left, nRect.bottom - nRect.top);
	GraphicsPath *path = new GraphicsPath();
	Rect corner = Rect(rect.X, rect.Y, 20, 20);
	path->AddArc(corner, 180, 90);
	corner.Width += 1;
	corner.Height += 1;
	rect.Width -= 1;
	rect.Height -= 1;
	corner.X += (rect.Width - 21);
	path->AddArc(corner, 270, 90);
	corner.Y += (rect.Height - 21);
	path->AddArc(corner, 0, 90);
	corner.X -= (rect.Width - 21);
	path->AddArc(corner, 90, 90);
	path->CloseFigure();
	return path;
}

RGB GenerateRandomColor(mt19937 *engine) {
	uniform_int_distribution<mt19937::result_type> dist(35, 255);
	BYTE r = dist(*engine);
	BYTE g = dist(*engine);
	BYTE b = dist(*engine);
	return{ r, g, b };
}

Color Darken(RGB color) {
	if (color.r > 50)
		color.r -= 50;
	else
		color.r = 0;
	if (color.g > 50)
		color.g -= 50;
	else
		color.g = 0;
	if (color.b > 50)
		color.b -= 50;
	else
		color.b = 0;
	return color;
}

BOOL OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct) {
	BlinkingLights *data = (BlinkingLights*)malloc(sizeof(BlinkingLights));

	RECT rect;
	GetClientRect(hWnd, &rect);
	LONG middle = (rect.right - rect.left) / 2;

	const static REAL intensities[] = { 0.0f, 0.5f, 1.0f };
	const static REAL positions[] = { 0.0f, 0.7f, 1.0f };

	LinearGradientBrush *background = new LinearGradientBrush(
		Point(middle, rect.top),
		Point(middle, rect.bottom),
		Color(255, 0, 0, 0),
		Color(255, 80, 80, 80)
	);
	background->SetBlend(intensities, positions, 3);
	data->background = background;

	data->rng = new mt19937();
	data->rng->seed(GetTickCount());

	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
#pragma warning(disable: 4244)
	int spacing = ((width % WIDTH) / (width / WIDTH)) * SPACING;
#pragma warning(default: 4244)
	int xRects = width / (WIDTH + spacing);
	int yRects = height / (HEIGHT + spacing);
	int cRects = xRects * yRects;
	RECT *rects = (RECT*)malloc(cRects * sizeof(RECT));
	RGB *colors = (RGB*)malloc(cRects * sizeof(RGB));
	int c = 0;
	for (int i = 0; i < xRects; i++) {
		for (int j = 0; j < yRects; j++) {
			rects[c] = {
				(i*(WIDTH+spacing)) + spacing,          // left
				(j*(HEIGHT+spacing)) + spacing,         // top
				(i*(WIDTH+spacing)) + spacing + WIDTH,  // right
				(j*(HEIGHT+spacing)) + spacing + HEIGHT // bottom
			};
			colors[c] = GenerateRandomColor(data->rng);
			c += 1;
		}
	}
	data->cRects = cRects;
	data->rects = rects;
	data->colors = colors;

#ifdef SCREENSAVER
	GetCursorPos(&data->cursorPos);
#endif

	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) data);

	SetTimer(hWnd, IDT_MAIN, 750, NULL);

	ITaskbarList2 *taskbarList;
	CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_ALL, IID_ITaskbarList2,
		reinterpret_cast<void**>(&taskbarList));
	taskbarList->MarkFullscreenWindow(hWnd, TRUE);
	taskbarList->Release();
	return TRUE;
}

BOOL OnSetCursor(HWND hWnd, HWND hWndCursor, UINT codeHitTest, UINT msg) {
	if (codeHitTest == HTCLIENT)
		SetCursor(NULL);
	return FALSE;
}

void OnTimer(HWND hWnd, UINT id) {
	if (id == IDT_MAIN) {
		BlinkingLights *data = (BlinkingLights*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

		int bound = data->cRects - 1;
		uniform_int_distribution<mt19937::result_type> dist(0, bound);
		for (int i = 0; i < 5; i++) {
			int chosen = dist(*data->rng);
			RGB color = GenerateRandomColor(data->rng);
			data->colors[chosen] = color;
		}
		InvalidateRect(hWnd, NULL, NULL);
	}
}

BOOL OnEraseBackground(HWND hWnd, HDC hdc) {
	BlinkingLights *data = (BlinkingLights*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	RECT rect;
	GetClientRect(hWnd, &rect);
	Graphics *graphics = new Graphics(hdc);
	graphics->FillRectangle(data->background, rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top);
	delete graphics;

	return TRUE;
}

void OnPaint(HWND hWnd) {
	BlinkingLights *data = (BlinkingLights*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	const static REAL intensities[] = { 0.0f, 0.7f, 1.0f };
	const static REAL positions[] = { 0.0f, 0.3f, 1.0f };

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);
	Graphics *graphics = new Graphics(hdc);
	for (int i = 0; i < data->cRects; i++) {
		RECT rect = data->rects[i];
		GraphicsPath *path = CreateRoundedRect(rect);
		PathGradientBrush *brush = new PathGradientBrush(path);
		brush->SetCenterColor(data->colors[i]);

		Color surround[] = { Darken(data->colors[i]) };
		int count = 1;

		brush->SetSurroundColors(surround, &count);
		brush->SetBlend(intensities, positions, 3);
		graphics->FillPath(brush, path);

		delete brush;
		delete path;
	}
	delete graphics;
	EndPaint(hWnd, &ps);
}

void OnKeyDown(HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT flags) {
#ifndef SCREENSAVER
	if (vk == VK_ESCAPE)
#endif
		DestroyWindow(hWnd);
}

void OnDestroy(HWND hWnd) {
	BlinkingLights *data = (BlinkingLights*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	delete data->background;
	delete data->rng;
	free(data->rects);
	free(data->colors);
	free(data);

	KillTimer(hWnd, IDT_MAIN);

	HWND next = FindWindow(szWindowClass, NULL);
	if (next && IsWindowVisible(next))
		DestroyWindow(next);

	PostQuitMessage(0);
}

#ifdef SCREENSAVER
void OnMouseButtonDown(HWND hWnd, BOOL fDoubleClick, int x, int y, UINT keyFlags) {
	DestroyWindow(hWnd);
}

void OnMouseMove(HWND hWnd, int x, int y, UINT keyFlags) {
	BlinkingLights *data = (BlinkingLights*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	// Windows seems to like sending garbage WM_MOUSEMOVE's. Check if its legit
	if (data->cursorPos.x != x || data->cursorPos.y != y)
		DestroyWindow(hWnd);
}
#endif

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hWnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hWnd, WM_SETCURSOR, OnSetCursor);
		HANDLE_MSG(hWnd, WM_TIMER, OnTimer);
		HANDLE_MSG(hWnd, WM_ERASEBKGND, OnEraseBackground);
		HANDLE_MSG(hWnd, WM_PAINT, OnPaint);
		HANDLE_MSG(hWnd, WM_KEYDOWN, OnKeyDown);
		HANDLE_MSG(hWnd, WM_DESTROY, OnDestroy);
#ifdef SCREENSAVER
		HANDLE_MSG(hWnd, WM_LBUTTONDOWN, OnMouseButtonDown);
		HANDLE_MSG(hWnd, WM_RBUTTONDOWN, OnMouseButtonDown);
		HANDLE_MSG(hWnd, WM_MBUTTONDOWN, OnMouseButtonDown);
		HANDLE_MSG(hWnd, WM_MOUSEMOVE, OnMouseMove);
#endif
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}