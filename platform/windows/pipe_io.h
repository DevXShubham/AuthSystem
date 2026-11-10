#pragma once

#include <windows.h>
#include <cstddef>

namespace PipeIO
{
    bool readExact(HANDLE pipe, void* data, size_t size);

    bool writeAll(HANDLE pipe, const void* data, size_t size);
}