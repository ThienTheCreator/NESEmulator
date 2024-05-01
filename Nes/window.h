#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <iostream>
#include <stdlib.h>
#include <cstdint>

using namespace std;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

inline uint32_t tempColor = 0;
int CALLBACK getidle();

inline HWND wind = NULL;

const int windowWidth = 256;
const int windowHeight = 240;
inline uint32_t windowPixelColor[windowWidth * windowHeight];

DWORD WINAPI ep(void* data);
