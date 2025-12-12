#include "functions.h"
#include "hud_basechat_2013.h"
#include "signatures.h"
#include "vsp.h"

// hl2sdk-episode1
#include "strtools.cpp"

void(__fastcall* CBaseHudChat_StartMessageModeA)(void*, void*, int) = nullptr;
void(__fastcall* CBaseHudChat_StopMessageModeA)(void*, void*)       = nullptr;
void(__fastcall* CBaseHudChatLine_InsertAndColorizeTextA)(void*, void*, wchar_t*, int) = nullptr;
void(__thiscall* RichText_SetTextA)(void*, const char*)             = nullptr;
void(__thiscall* RichText_InsertStringA)(void*, const char*)        = nullptr;
void(__thiscall* RichText_InsertString2A)(void*, const wchar_t*)    = nullptr;
void(__thiscall* RichText_InsertColorChangeA)(void*, Color)         = nullptr;
void(__thiscall* RichText_InsertFadeA)(void*, float, float)         = nullptr;

CHook* CBaseHudChat_StartMessageModeH = nullptr;
CHook* CBaseHudChat_StopMessageModeH = nullptr;
CHook* CBaseHudChatLine_InsertAndColorizeTextH = nullptr;

ConVar* con_enable = nullptr, * hud_saytext_time = nullptr;
float con_enable_buffer = 0.0;

void con_enable_callback(ConVar* var, char const* pOldString);

// https://gitlab.com/evilcrydie/source-sdk-2013/-/blob/main/src/game/client/hud_basechat.h#L148
void Colorize(CBaseHudChat* pChat, wchar_t* m_text, CUtlVector<TextRange>& m_textRanges, int alpha = 255);

bool BaseChat_Load()
{
    CSigScan_Find(client, CBaseHudChat_StartMessageMode);
    CSigScan_Find(client, CBaseHudChat_StopMessageMode);

    CSigScan_Find(client, CBaseHudChatLine_InsertAndColorizeText);
    CSigScan_Find(client, RichText_SetText);
    CSigScan_Find(client, RichText_InsertString);
    CSigScan_Find(client, RichText_InsertString2);
    CSigScan_Find(client, RichText_InsertColorChange);
    CSigScan_Find(client, RichText_InsertFade);


    CHook_Init(CBaseHudChat_StartMessageMode);
    CHook_Init(CBaseHudChat_StopMessageMode);
    CHook_Init(CBaseHudChatLine_InsertAndColorizeText);


    con_enable = s_pCVar->FindVar("con_enable");
    if (con_enable == nullptr)
    {
        Report("ConVar 'con_enable' not found\n");
        return false;
    }

    con_enable->InstallChangeCallback(con_enable_callback);
    con_enable_buffer = con_enable->GetFloat();

    return true;
}

void BaseChat_Unload()
{
    if (CBaseHudChat_StartMessageModeH)
    {
        delete CBaseHudChat_StartMessageModeH;
        CBaseHudChat_StartMessageModeH = nullptr;
    }

    if (CBaseHudChat_StopMessageModeH)
    {
        delete CBaseHudChat_StopMessageModeH;
        CBaseHudChat_StopMessageModeH = nullptr;
    }

    if (CBaseHudChatLine_InsertAndColorizeTextH)
    {
        delete CBaseHudChatLine_InsertAndColorizeTextH;
        CBaseHudChatLine_InsertAndColorizeTextH = nullptr;
    }
}

void BaseChat_Pause()
{
    if (CBaseHudChat_StartMessageModeH->IsHooked() && !CBaseHudChat_StartMessageModeH->Unhook())
        Report("Pause: !CBaseHudChat_StartMessageModeH->Unhook()\n");

    if (CBaseHudChat_StopMessageModeH->IsHooked() && !CBaseHudChat_StopMessageModeH->Unhook())
        Report("Pause: !CBaseHudChat_StopMessageModeH->Unhook()\n");

    if (CBaseHudChatLine_InsertAndColorizeTextH->IsHooked() && !CBaseHudChatLine_InsertAndColorizeTextH->Unhook())
        Report("Pause: !CBaseHudChatLine_InsertAndColorizeTextH->Unhook()\n");
}

void BaseChat_UnPause()
{
    if (!CBaseHudChat_StartMessageModeH->IsHooked() && !CBaseHudChat_StartMessageModeH->Hook())
        Report("UnPause: !CBaseHudChat_StartMessageModeH->Hook()\n");

    if (!CBaseHudChat_StopMessageModeH->IsHooked() && !CBaseHudChat_StopMessageModeH->Hook())
        Report("UnPause: !CBaseHudChat_StopMessageModeH->Hook()\n");

    if (!CBaseHudChatLine_InsertAndColorizeTextH->IsHooked() && !CBaseHudChatLine_InsertAndColorizeTextH->Hook())
        Report("UnPause: !CBaseHudChatLine_InsertAndColorizeTextH->Hook()\n");

    con_enable_buffer = con_enable->GetFloat();
}

// https://gitlab.com/evilcrydie/SourceEngine2007/-/blob/master/src_main/game/client/hud_basechat.cpp#L1130
void __fastcall CBaseHudChat_StartMessageMode(void* ecx, void* edx, int iMessageModeType)
{
    CBaseHudChat_StartMessageModeH->Unhook();

    con_enable_buffer = con_enable->GetFloat();
    con_enable->SetValue(0);
    CBaseHudChat_StartMessageModeA(ecx, edx, iMessageModeType);

    CBaseHudChat_StartMessageModeH->Hook();
}

// https://gitlab.com/evilcrydie/SourceEngine2007/-/blob/master/src_main/game/client/hud_basechat.cpp#L1182
void __fastcall CBaseHudChat_StopMessageMode(void* ecx, void* edx)
{
    CBaseHudChat_StopMessageModeH->Unhook();

    con_enable->SetValue(con_enable_buffer);
    CBaseHudChat_StopMessageModeA(ecx, edx);

    CBaseHudChat_StopMessageModeH->Hook();
}


// https://gitlab.com/evilcrydie/source-sdk-2013/-/blob/main/src/game/client/hud_basechat.cpp#L1403
void __fastcall CBaseHudChatLine_InsertAndColorizeText(void* ecx, void* edx, wchar_t* buf, int clientIndex)
{
    // create property
    int& m_iNameLength = __property<int, 93>(ecx);
    int& m_iNameStart = __property<int, 104>(ecx);


    wchar_t* m_text = CloneWString(buf);
    CUtlVector<TextRange> m_textRanges;
    m_textRanges.RemoveAll();
    CBaseHudChat* pChat = dynamic_cast<CBaseHudChat*>(CBaseHudChat_GetParent(ecx));
    if (pChat == nullptr)
    {
        delete[] m_text;
        return;
    }

    wchar_t* txt = m_text;
    int lineLen = wcslen(m_text);
    Color colCustom;
    if (m_text[0] == COLOR_PLAYERNAME || m_text[0] == COLOR_LOCATION ||
        m_text[0] == COLOR_NORMAL || m_text[0] == COLOR_ACHIEVEMENT ||
        m_text[0] == COLOR_CUSTOM || m_text[0] == COLOR_HEXCODE ||
        m_text[0] == COLOR_HEXCODE_ALPHA)
    {
        while (txt && *txt)
        {
            TextRange range;
            bool bFoundColorCode = false;
            bool bDone = false;
            int nBytesIn = txt - m_text;

            switch (*txt)
            {
            case COLOR_CUSTOM:
            case COLOR_PLAYERNAME:
            case COLOR_LOCATION:
            case COLOR_ACHIEVEMENT:
            case COLOR_NORMAL:
            {
                // save this start
                range.start = nBytesIn + 1;
                //range.color = pChat->GetTextColorForClient((TextColor)(*txt), clientIndex);
                CBaseHudChat_GetTextColorForClient(pChat, range.color, (TextColor)(*txt), clientIndex);
                range.end = lineLen;
                bFoundColorCode = true;
            }
            ++txt;
            break;
            case COLOR_HEXCODE:
            case COLOR_HEXCODE_ALPHA:
            {
                bool bReadAlpha = (*txt == COLOR_HEXCODE_ALPHA);
                const int nCodeBytes = (bReadAlpha ? 8 : 6);
                range.start = nBytesIn + nCodeBytes + 1;
                range.end = lineLen;
                range.preserveAlpha = bReadAlpha;
                ++txt;

                if (range.end > range.start)
                {
                    int r = V_nibble(txt[0]) << 4 | V_nibble(txt[1]);
                    int g = V_nibble(txt[2]) << 4 | V_nibble(txt[3]);
                    int b = V_nibble(txt[4]) << 4 | V_nibble(txt[5]);
                    int a = 255;

                    if (bReadAlpha)
                    {
                        a = V_nibble(txt[6]) << 4 | V_nibble(txt[7]);
                    }

                    range.color = Color(r, g, b, a);
                    bFoundColorCode = true;

                    txt += nCodeBytes;
                }
                else
                {
                    // Not enough characters remaining for a hex code. Skip the rest of the string.
                    bDone = true;
                }
            }
            break;
            default:
                ++txt;
            }

            if (bDone)
            {
                break;
            }

            if (bFoundColorCode)
            {
                int count = m_textRanges.Count();
                if (count)
                {
                    m_textRanges[count - 1].end = nBytesIn;
                }

                m_textRanges.AddToTail(range);
            }
        }
    }

    if (!m_textRanges.Count() && m_iNameLength > 0 &&
        m_text[0] == COLOR_USEOLDCOLORS)
    {
        TextRange range;
        range.start = 0;
        range.end = m_iNameStart;
        //range.color = pChat->GetTextColorForClient(COLOR_NORMAL, clientIndex);
        CBaseHudChat_GetTextColorForClient(pChat, range.color, COLOR_NORMAL, clientIndex);
        m_textRanges.AddToTail(range);

        range.start = m_iNameStart;
        range.end = m_iNameStart + m_iNameLength;
        //range.color = pChat->GetTextColorForClient(COLOR_PLAYERNAME, clientIndex);
        CBaseHudChat_GetTextColorForClient(pChat, range.color, COLOR_PLAYERNAME, clientIndex);
        m_textRanges.AddToTail(range);

        range.start = range.end;
        range.end = wcslen(m_text);
        // range.color = pChat->GetTextColorForClient(COLOR_NORMAL, clientIndex);
        CBaseHudChat_GetTextColorForClient(pChat, range.color, COLOR_NORMAL, clientIndex);
        m_textRanges.AddToTail(range);
    }

    if (!m_textRanges.Count())
    {
        TextRange range;
        range.start = 0;
        range.end = wcslen(m_text);
        // range.color = pChat->GetTextColorForClient( COLOR_NORMAL, clientIndex );
        CBaseHudChat_GetTextColorForClient(pChat, range.color, COLOR_NORMAL, clientIndex);
        m_textRanges.AddToTail(range);
    }

    for (int i = 0; i < m_textRanges.Count(); ++i)
    {
        wchar_t* start = m_text + m_textRanges[i].start;
        if (*start > 0 && *start < COLOR_MAX)
        {
            //Assert(*start != COLOR_HEXCODE && *start != COLOR_HEXCODE_ALPHA);
            m_textRanges[i].start += 1;
        }
    }

    // Colorize();
    Colorize(pChat, m_text, m_textRanges);
}

// https://gitlab.com/evilcrydie/source-sdk-2013/-/blob/main/src/game/client/hud_basechat.cpp#L1547
void Colorize(CBaseHudChat* pChat, wchar_t* m_text, CUtlVector<TextRange>& m_textRanges, int alpha)
{
    // clear out text
    // SetText("");
    //RichText_SetText(ecx, "");

    //CBaseHudChat* pChat = dynamic_cast<CBaseHudChat*>(CBaseHudChat__GetParent(ecx));

    // pChat->GetChatHistory()
    void* m_pChatHistory = nullptr;

    if (pChat && (m_pChatHistory = *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(pChat) + 252)))
    {
        // pChat->GetChatHistory()->InsertString("\n");
        RichText_InsertStringA(m_pChatHistory, "\n");
    }

    wchar_t wText[4096];
    Color color;
    for (int i = 0; i < m_textRanges.Count(); ++i)
    {
        wchar_t* start = m_text + m_textRanges[i].start;
        int len = m_textRanges[i].end - m_textRanges[i].start + 1;
        if (len > 1 && len <= ARRAYSIZE(wText))
        {
            wcsncpy(wText, start, len);
            wText[len - 1] = 0;
            color = m_textRanges[i].color;
            if (!m_textRanges[i].preserveAlpha)
            {
                color[3] = alpha;
            }

            // InsertColorChange(color);
            //RichText_InsertColorChange(ecx, color);
            // InsertString(wText);
            //RichText_InsertString2(ecx, wText);
            Con_ColorPrintf(color, "%s", WideToANSI(wText).c_str());

            if (pChat && m_pChatHistory)
            {
                // pChat->GetChatHistory()->InsertColorChange(color);
                RichText_InsertColorChangeA(m_pChatHistory, color);
                // pChat->GetChatHistory()->InsertString(wText);
                RichText_InsertString2A(m_pChatHistory, wText);

                if (hud_saytext_time == nullptr)
                {
                    if ((hud_saytext_time = s_pCVar->FindVar("hud_saytext_time")) == nullptr)
                    {
                        Report("ConVar 'hud_saytext_time' not found\n");
                    }
                    else
                    {
                        // pChat->GetChatHistory()->InsertFade(hud_saytext_time.GetFloat(), CHAT_HISTORY_IDLE_FADE_TIME);
                        RichText_InsertFadeA(m_pChatHistory, hud_saytext_time->GetFloat(),
                            CHAT_HISTORY_IDLE_FADE_TIME);
                    }
                }

                if (i == m_textRanges.Count() - 1)
                {
                    // pChat->GetChatHistory()->InsertFade(-1, -1);
                    RichText_InsertFadeA(m_pChatHistory, -1, -1);
                }
            }

        }
    }
    Msg("\n");

    // InvalidateLayout(true);
    // https://gitlab.com/evilcrydie/source-sdk-2013/-/blob/main/src/public/vgui_controls/Panel.h#L323
    //__call<void(__thiscall*)(void*, bool, bool), 240>(ecx)(ecx, true, false)
    delete[] m_text;
}

void con_enable_callback(ConVar* var, char const* pOldString)
{
    if (!CBaseHudChat_StartMessageModeH || !CBaseHudChat_StopMessageModeH ||
        !(CBaseHudChat_StartMessageModeH->IsHooked() && CBaseHudChat_StopMessageModeH->IsHooked()))
        return;

    con_enable_buffer = var->GetFloat();
}
