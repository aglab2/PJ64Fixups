#include "backtrace.h"

#include <Windows.h>

namespace Backtrace
{
    constexpr int BacktraceDepth = 10;
    constexpr int ScanningDepth = 1000;

    // clang is being particularly stupid about this, do not optimize
    __attribute__((optnone))
	Addresses collectStackAddresses()
	{
        volatile void* stackVar{};
        auto esp = (uintptr_t*)&stackVar;

        Addresses pointers;
        pointers.reserve(BacktraceDepth);
        for (int i = 0; i < ScanningDepth && pointers.size() < BacktraceDepth; i++)
        {
            auto cmd = esp[i];
            if (0x400000 <= cmd && cmd <= 0x4f0000)
            {
                pointers.emplace_back(cmd);
            }
        }

        return pointers;
	}
}
