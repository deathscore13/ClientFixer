#pragma once
// hl2sdk-episode1
#include "hud_basechat.h"

// https://gitlab.com/evilcrydie/source-sdk-2013/-/blob/main/src/game/client/hud_basechat.h#L38
#define CHAT_HISTORY_IDLE_FADE_TIME 2.5f

// https://gitlab.com/evilcrydie/source-sdk-2013/-/blob/main/src/game/client/hud_basechat.h#L86
struct TextRange
{
	int start = 0;
	int end = 0;
	Color color = {255, 255, 255};
	bool preserveAlpha = false;
};

// https://gitlab.com/evilcrydie/source-sdk-2013/-/blob/main/src/game/client/hud_basechat.h#L72
enum TextColor
{
	COLOR_NORMAL = 1,
	COLOR_USEOLDCOLORS = 2,
	COLOR_PLAYERNAME = 3,
	COLOR_LOCATION = 4,
	COLOR_ACHIEVEMENT = 5,
	COLOR_CUSTOM = 6,		// Will use the most recently SetCustomColor()
	COLOR_HEXCODE = 7,		// Reads the color from the next six characters
	COLOR_HEXCODE_ALPHA = 8,// Reads the color and alpha from the next eight characters
	COLOR_MAX
};

// https://gitlab.com/evilcrydie/source-sdk-2013/-/blob/main/src/game/client/hud_basechat.h#L108
inline wchar_t* CloneWString(const wchar_t* str)
{
	const int nLen = V_wcslen(str) + 1;
	wchar_t* cloneStr = new wchar_t[nLen];
	const int nSize = nLen * sizeof(wchar_t);
	V_wcsncpy(cloneStr, str, nSize);
	return cloneStr;
}


extern void(__fastcall* CBaseHudChat_StartMessageModeA)(void*, void*, int);
extern void(__fastcall* CBaseHudChat_StopMessageModeA)(void*, void*);
extern void(__fastcall* CBaseHudChatLine_InsertAndColorizeTextA)(void*, void*, wchar_t*, int);
extern void(__thiscall* RichText_SetTextA)(void*, const char*);
extern void(__thiscall* RichText_InsertStringA)(void*, const char*);
extern void(__thiscall* RichText_InsertString2A)(void*, const wchar_t*);
extern void(__thiscall* RichText_InsertColorChangeA)(void*, Color);
extern void(__thiscall* RichText_InsertFadeA)(void*, float, float);

void __fastcall CBaseHudChat_StartMessageMode(void* ecx, void* edx, int iMessageModeType);
void __fastcall CBaseHudChat_StopMessageMode(void* ecx, void* edx);
void __fastcall CBaseHudChatLine_InsertAndColorizeText(void* ecx, void* edx, wchar_t* buf, int clientIndex);


#include "class_hack.h"

#define CBaseHudChat_GetParent(ptr) \
	__call<vgui::Panel* (__thiscall*)(void*), 144>(ptr)(ptr)
// return Color --> edx
#define CBaseHudChat_GetTextColorForClient(ptr, color, colorNum, clientIndex) \
	__call<void(__thiscall*)(void*, Color&, TextColor, int), 88>(ptr)(ptr, color, colorNum, clientIndex)



bool BaseChat_Load();
void BaseChat_Unload();
void BaseChat_Pause();
void BaseChat_UnPause();
