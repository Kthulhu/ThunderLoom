#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "woven_cloth.h"
void StartPatternEditor(HINSTANCE hinst,HWND parent_hwnd,
	wcWeaveParameters *param);