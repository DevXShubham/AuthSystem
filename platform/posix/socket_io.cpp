#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>

#include "socket_io.h"

namespace SocketIO {
    bool configureSocket(int fd) {

    #ifdef SO_NOSIGPIPE
        int enabled = 1;

        if(setsockopt(
            fd, 
            SOL_SOCKET,
            SO_NOSIGPIPE,
            &enabled,
            sizeof(enabled)) < 0
        ) return false;
    #endif

        return true;
    }

    bool readExact(int socket_fd, void* data, size_t size)
    {
        char* buffer = static_cast<char*>(data);
        size_t totalRead = 0;

        while (totalRead < size)
        {
            ssize_t bytesRead = recv(
                socket_fd,
                buffer + totalRead,
                size - totalRead,
                0
            );

            if (bytesRead < 0)
            {
                if (errno == EINTR)
                    continue;

                return false;
            }

            if (bytesRead == 0)
                return false;

            totalRead += static_cast<size_t>(bytesRead);
        }

        return true;
    }

    bool writeAll(int socket_fd, const void* data, size_t size)
    {
        const char* buffer = static_cast<const char*>(data);
        size_t totalWritten = 0;

        while (totalWritten < size)
        {
    #ifdef MSG_NOSIGNAL
            constexpr int flags = MSG_NOSIGNAL;
    #else
            constexpr int flags = 0;
    #endif

            ssize_t bytesWritten = send(
                socket_fd,
                buffer + totalWritten,
                size - totalWritten,
                flags
            );

            if (bytesWritten < 0)
            {
                if (errno == EINTR)
                    continue;

                return false;
            }

            if (bytesWritten == 0)
                return false;

            totalWritten += static_cast<size_t>(bytesWritten);
        }

        return true;
    }
}