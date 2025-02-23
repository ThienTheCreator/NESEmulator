#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <iostream>
#include <stdlib.h>
#include <cstdint>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void updateScreen();

inline HWND wind = NULL;

inline uint8_t controller;
inline bool runProgram = true;

const int windowWidth = 256;
const int windowHeight = 240;
extern uint32_t windowPixelColor[windowWidth * windowHeight];

DWORD WINAPI ep(void* data);
