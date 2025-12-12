#pragma once

#include <Windows.h>

typedef void* SigScanAddr;

class CSigScan
{
private:
    BYTE* addr;
    size_t len;

    CSigScan(BYTE* address, size_t length);

public:
    static CSigScan* Create(void* interfaceFn);
    SigScanAddr Find(const BYTE* sig, const BYTE* mask);
};
