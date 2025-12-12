#pragma once
#include "sigscan.h"

// hl2sdk-episode1
#include "Color.h"

#include <string>
#include <Windows.h>

// FIXME: Con_ColorPrintf
inline std::string WideToANSI(const wchar_t* wstr)
{
	int size = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
	std::string buffer(size, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &buffer[0], size, nullptr, nullptr);
	return buffer;
}

extern SigScanAddr Con_ColorPrintfA;

#define Con_ColorPrintf(clr, ...) \
    reinterpret_cast<void (__cdecl*)(const Color&, const char*, ...)>(Con_ColorPrintfA)(clr, __VA_ARGS__)

bool Functions_Load();
