#pragma once

#include <vector>
#include <stdint.h>

namespace Backtrace
{
    using Addresses = std::vector<uintptr_t>;

    // Looks up the stack for any addresses
    Addresses collectStackAddresses();
}
