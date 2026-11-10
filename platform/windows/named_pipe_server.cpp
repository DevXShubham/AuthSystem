#include <windows.h>
#include <memory>
#include <iostream>

#include "ipc_server.h"
#include "pipe_io.h"

constexpr const char* PIPE_NAME = R"(\\.\pipe\AuthServicePipe)";

class NamedPipeConnection : public IpcConnection {
private:
    HANDLE pipe;

public:
    explicit NamedPipeConnection(HANDLE handle) 
        : pipe(handle) {}

    ~NamedPipeConnection() override {
        if(pipe != INVALID_HANDLE_VALUE) {
            FlushFileBuffers(pipe);
            DisconnectNamedPipe(pipe);
            CloseHandle(pipe);
        }
    }

    bool readExact(void* data, size_t size) override {
        return PipeIO::readExact(pipe, data, size);
    }

    bool writeAll(const void* data, size_t size) override {
        return PipeIO::writeAll(pipe, data, size);
    }
};

class NamedPipeServer : public IpcServer {
private:
    HANDLE pipe = INVALID_HANDLE_VALUE;

public:
    ~NamedPipeServer() override {
        if(pipe != INVALID_HANDLE_VALUE) 
            CloseHandle(pipe);
    }

    bool start() override {
        return true;
    }

    std::unique_ptr<IpcConnection> acceptClient() override {
        pipe = CreateNamedPipeA(PIPE_NAME, 
            PIPE_ACCESS_DUPLEX, 
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            1024, 1024, 0, nullptr);

        if(pipe == INVALID_HANDLE_VALUE) {
            std::cerr << "CreateNamedPipe failed: "
                      << GetLastError() << std::endl;
            return nullptr;
        }

        std::cout << "Waiting for client..." << std::endl;

        BOOL connected = ConnectNamedPipe(pipe, nullptr)
            ? TRUE
            : (GetLastError() == ERROR_PIPE_CONNECTED);

        if(!connected) {
            std::cerr << "Connection failed" 
                    << GetLastError() << std::endl;

            CloseHandle(pipe);
            pipe = INVALID_HANDLE_VALUE;

            return nullptr;
        }

        HANDLE connectedPipe = pipe;
        pipe = INVALID_HANDLE_VALUE;

        return std::make_unique<NamedPipeConnection>(connectedPipe);
    }
};

std::unique_ptr<IpcServer> createIpcServer() {
    return std::make_unique<NamedPipeServer>();
}