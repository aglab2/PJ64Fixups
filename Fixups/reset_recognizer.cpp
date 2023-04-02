#include "reset_recognizer.h"

#include <stdint.h>

#include <sstream>
#include <unordered_map>

namespace ResetRecognizer
{
    constexpr int ScanningDepth = 100;

    std::unordered_map<uintptr_t, Type> MagicAddresses
    {
        { 0x40d4d5, Type::END_EMULATION }, // Main_Proc case ID_FILE_ENDEMULATION
        { 0x40d545, Type::RESET },         // Main_Proc case ID_CPU_RESET
        { 0x40fd6d, Type::SHUTDOWN },      // ShutdownApplication
        { 0x449ef5, Type::OPEN_ROM },      // OpenChosenFile
        { 0x417f41, Type::SWITCH_PLUGIN }, // PluginSelectProc
    };

    Addresses stackAddresses()
    {
        volatile void* stackVar{};
        auto esp = (uintptr_t*)&stackVar;

        Addresses pointers;
        for (int i = 0; i < ScanningDepth; i++)
        {
            auto cmd = esp[i];
            if (0x400000 <= cmd && cmd <= 0x4f0000)
            {
                pointers.emplace_back(cmd);
            }
        }

        return pointers;
    }

    std::string toString(const Addresses& addresses)
    {
        std::ostringstream ss;
        bool first = true;
        for (auto& address : addresses)
        {
            if (first)
            {
                ss << std::hex << "0x" << address;
                first = false;
            }
            else
            {
                ss << ", " << std::hex << "0x" << address;
            }
        }
        return ss.str();
    }

    Type recognize(const Addresses& addresses)
    {
        for (auto& address : addresses)
        {
            auto it = MagicAddresses.find(address);
            if (it != MagicAddresses.end())
                return it->second;
        }

        return Type::UNKNOWN;
    }
}
