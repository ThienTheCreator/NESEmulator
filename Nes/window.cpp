#include "ppu2C02.h"

#include "window.h"

using namespace std;
uint32_t windowPixelColor[windowWidth * windowHeight] = {0};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	wchar_t msg[32];
	switch (uMsg)
	{
		// s d f enter 
		case WM_KEYDOWN:
			switch(wParam){
				case 0x46: // F - A
					controller |= 0x80;
					break;
				case 0x44: // D - B 
					controller |= 0x40;
					break;
				case 0x53: // S - Select
					controller |= 0x20;
					break;
				case 0x0D: // Enter - Start
					controller |= 0x10;
					break;
				case 0x26: // UP 
					controller |= 0x8;
					break;
				case 0x28: // Down
					controller |= 0x4;
					break;
				case 0x25: // Left
					controller |= 0x2;
					break;
				case 0x27: // Right
					controller |= 0x1;
					break;
			}
			break;

		case WM_KEYUP:
			switch(wParam){
				case 0x46: // F - A
					controller &= ~0x80;
					break;
				case 0x44: // D - B 
					controller &= ~0x40;
					break;
				case 0x53: // S - Select
					controller &= ~0x20;
					break;
				case 0x0D: // Enter - Start
					controller &= ~0x10;
					break;
				case 0x26: // UP 
					controller &= ~0x8;
					break;
				case 0x28: // Down
					controller &= ~0x4;
					break;
				case 0x25: // Left
					controller &= ~0x2;
					break;
				case 0x27: // Right
					controller &= ~0x1;
					break;
			}
			break;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			HDC hdcMem = CreateCompatibleDC(hdc);

			int w = 256;
			int h = 240;

			int32_t* pvBits = NULL;

			BITMAPINFO bmi;
			memset(&bmi, 0, sizeof(BITMAPINFO));
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = 256;
			bmi.bmiHeader.biHeight = -240;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biCompression = BI_RGB;
			
			HBITMAP bitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&pvBits, 0, 0);
			HGDIOBJ oldbmp = SelectObject(hdcMem, bitmap); 

			BitBlt(hdcMem, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
			
			for(int i = 0; i < w * h; i++){
				pvBits[i] = windowPixelColor[i];
			}
			BitBlt(hdc, 0, 0, w, h, hdcMem, 0, 0, SRCCOPY);

			SelectObject(hdcMem, oldbmp);
			DeleteObject(bitmap);
			DeleteDC(hdcMem);

			EndPaint(hwnd, &ps);

			return 0;
		}

		case WM_DESTROY:
			runProgram = false;
			PostQuitMessage(0);
			return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void updateScreen(){
	InvalidateRect( wind, NULL, FALSE );
}

DWORD WINAPI ep(void* data){
	HINSTANCE hInstance = GetModuleHandle(NULL);

	const wchar_t CLASSNAME[] = L"Sample Window Class";

	WNDCLASS wc = { 0 };

	wc.lpfnWndProc   = WindowProc;
	wc.hInstance     = hInstance;
	wc.lpszClassName = CLASSNAME;

	RegisterClass(&wc);

	RECT r;
	r.left = 0;
	r.top = 0;
	r.right = 256;
	r.bottom = 240;
	AdjustWindowRectEx(&r, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, FALSE, 0);

	// Create the window.
	wind = CreateWindowEx(
		0,
		CLASSNAME,
		L"Learn to Program Windows",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, 

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom-r.top,

		NULL,
		NULL,
		hInstance,
		NULL
		);

	if (wind == NULL)
	{
		return 0;
	}
	
	ShowWindow(wind, 1);

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	return 0;
}

