#include "signatures.h"
#include "sv_plugin.h"
#include "vsp.h"

// hl2sdk-episode1
#include "engine/iserverplugin.h"

#include <stdlib.h>

#define PLUGIN_PAUSE_INDEX_OFFSET 0x1A		// 0x2017B30A
#define PLUGIN_UNPAUSE_INDEX_OFFSET 0x1A	// 0x2017B36A

void* plugin_pauseA = nullptr;
void* plugin_unpauseA = nullptr;

void plugin_patch(void* address);

bool sv_plugin_Load()
{
    CSigScan_Find(engine, plugin_pause);
    CSigScan_Find(engine, plugin_unpause);

	plugin_patch(reinterpret_cast<BYTE*>(plugin_pauseA) + PLUGIN_PAUSE_INDEX_OFFSET);
    plugin_patch(reinterpret_cast<BYTE*>(plugin_unpauseA) + PLUGIN_UNPAUSE_INDEX_OFFSET);

	return true;
}

void plugin_patch(void* address)
{
    DWORD oldProtect;
    if (!VirtualProtect(address, 1, PAGE_EXECUTE_READWRITE, &oldProtect) ||
        *reinterpret_cast<BYTE*>(address) != 0x02)
        return;

    *reinterpret_cast<BYTE*>(address) = 0x01;

    VirtualProtect(address, 1, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), address, 1);
}
