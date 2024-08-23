#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <iostream>
#include <stdlib.h>
#include <cstdint>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static uint32_t tempColor = 0;
void updateScreen(uint32_t arr[]);

inline HWND wind = NULL;

const int windowWidth = 256;
const int windowHeight = 240;
static uint32_t windowPixelColor[windowWidth * windowHeight];

DWORD WINAPI ep(void* data);
