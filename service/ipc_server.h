#pragma once 

#include <cstddef>
#include <memory>

class IpcConnection {
public:
    virtual ~IpcConnection() = default;

    virtual bool readExact(void* data, size_t size) = 0;

    virtual bool writeAll(const void* data, size_t size) = 0;
};

class IpcServer {
public:
    virtual ~IpcServer() = default;

    virtual bool start() = 0;

    virtual std::unique_ptr<IpcConnection> acceptClient() = 0;
};

std::unique_ptr<IpcServer> createIpcServer();