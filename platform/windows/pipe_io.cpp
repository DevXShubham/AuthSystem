#include "protocol_io.h"

bool writeAll(
    HANDLE pipe, 
    const void* data, 
    DWORD size
) {
    const char* buffer = static_cast<const char*>(data);

    DWORD totalWritten = 0;

    while(totalWritten < size) {
        DWORD written = 0;

        if(!WriteFile(
            pipe, 
            buffer + totalWritten, 
            size - totalWritten, 
            &written,
            nullptr)) 
        {
            return false;
        }

        if (written == 0) 
            return false;

        totalWritten += written;
    }

    return true;
}

bool readExact(
    HANDLE pipe,
    void* data,
    DWORD size)
{
    char* buffer = static_cast<char*>(data);

    DWORD totalRead = 0;

    while (totalRead < size)
    {
        DWORD received = 0;

        if (!ReadFile(
                pipe,
                buffer + totalRead,
                size - totalRead,
                &received,
                nullptr))
        {
            return false;
        }

        if (received == 0)
            return false;

        totalRead += received;
    }

    return true;
}
