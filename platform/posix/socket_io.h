#pragma once

#include <cstddef>

namespace SocketIO {
    bool configureSocket(int fd);

    bool readExact(int fd, void* data, size_t size);

    bool writeAll(int fd, const void* data, size_t size);
}