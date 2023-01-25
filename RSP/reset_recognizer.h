#pragma once

#include <string>
#include <vector>

namespace ResetRecognizer
{
    using Addresses = std::vector<uintptr_t>;
    enum class Type
    {
        UNKNOWN,
        END_EMULATION,
        RESET,
        SHUTDOWN,
        OPEN_ROM,
        SWITCH_PLUGIN,
    };

    Addresses stackAddresses();
    std::string toString(const Addresses&);
    Type recognize(const Addresses&);
};
