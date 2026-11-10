#include "pipe_io.h"

namespace PipeIO
{
    bool readExact(HANDLE pipe, void* data, size_t size)
    {
        char* buffer = static_cast<char*>(data);
        size_t totalRead = 0;

        while (totalRead < size)
        {
            DWORD bytesRead = 0;

            if (!ReadFile(
                    pipe,
                    buffer + totalRead,
                    static_cast<DWORD>(size - totalRead),
                    &bytesRead,
                    nullptr) || bytesRead == 0)
            {
                return false;
            }

            totalRead += bytesRead;
        }

        return true;
    }

    bool writeAll(HANDLE pipe, const void* data, size_t size)
    {
        const char* buffer = static_cast<const char*>(data);
        size_t totalWritten = 0;

        while (totalWritten < size)
        {
            DWORD bytesWritten = 0;

            if (!WriteFile(
                    pipe,
                    buffer + totalWritten,
                    static_cast<DWORD>(size - totalWritten),
                    &bytesWritten,
                    nullptr) || bytesWritten == 0)
            {
                return false;
            }

            totalWritten += bytesWritten;
        }

        return true;
    }
}