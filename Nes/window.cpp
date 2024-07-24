#define _HAS_STD_BYTE 0

#include "window.h"

using namespace std;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
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
			bmi.bmiHeader.biHeight = 240;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biCompression = BI_RGB;
			
			HBITMAP bitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&pvBits, 0, 0);
			HGDIOBJ oldbmp = SelectObject(hdcMem, bitmap); 

			BitBlt(hdcMem, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
			
			for(int i = 0; i < w *h; i++){
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
			PostQuitMessage(0);
			return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int CALLBACK getidle(){
	for(int i = 0; i < windowWidth * windowHeight; i++){
		windowPixelColor[i] = tempColor;
	}
	
	tempColor += 0x01010100;
	InvalidateRect(wind, NULL, 0);

	return 0;
}

DWORD WINAPI ep(void* data){
	HINSTANCE hInstance = GetModuleHandle(NULL);

	const wchar_t CLASS_NAME[] = L"Sample Window Class";

	WNDCLASS wc = { };

	wc.lpfnWndProc   = WindowProc;
	wc.hInstance     = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	// Create the window.
	
	wind = CreateWindowEx(
		0,
		CLASS_NAME,
		L"Learn to Program Windows",
		WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU,

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, 256, 240,

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

