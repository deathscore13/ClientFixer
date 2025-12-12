#include "signatures.h"
#include "vsp.h"

SigScanAddr Con_ColorPrintfA = nullptr;

bool Functions_Load()
{
    CSigScan_Find(engine, Con_ColorPrintf);
    return true;
}
