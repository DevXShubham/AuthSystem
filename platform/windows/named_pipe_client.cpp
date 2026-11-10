#include <windows.h>
#include <iostream>

#include "ipc_client.h"
#include "protocol.h"
#include "pipe_io.h"

constexpr const char* PIPE_NAME =  
    R"(\\.\pipe\AuthServicePipe)";

constexpr const int PIPE_PERMISSIONS = GENERIC_READ | GENERIC_WRITE;

class NamedPipeClient : public IpcClient
{
private: 
    HANDLE pipe = INVALID_HANDLE_VALUE;

public:
    bool connect() override {
        pipe = CreateFileA(
            PIPE_NAME,
            PIPE_PERMISSIONS,
            0,
            nullptr, 
            OPEN_EXISTING,
            0, 
            nullptr
        );

        return pipe != INVALID_HANDLE_VALUE;
    }

    bool sendRequest(
        const std::string& request, 
        std::string &response
    ) override 
    {
        DWORD bytesWritten = 0;

        if (pipe == INVALID_HANDLE_VALUE) {
            std::cerr << "sendRequest failed: named pipe is not connected" << std::endl;
            return false;
        }


        
        if (request.size() > Protocol::MAX_MESSAGE_SIZE) {
            std::cerr << "sendRequest failed: request exceeds maximum size" << std::endl;
            return false;
        }

        unsigned char header[Protocol::HEADER_SIZE];
        
        Protocol::encodeLength(
            static_cast<uint32_t>(request.size()),
            header
        );

        // Send message length
        if(!PipeIO::writeAll(pipe, header, sizeof(header))) {
            std::cerr << "sendRequest failed writing request header: "
                      << GetLastError() << std::endl;
            return false;
        }

        // Send message payload
        if(!PipeIO::writeAll(
            pipe, 
            request.data(),
            static_cast<DWORD>(request.size())
        )) {
            std::cerr << "sendRequest failed writing request payload: "
                      << GetLastError() << std::endl;
            return false;
        }

        // Receive response length
        unsigned char responseHeader[Protocol::HEADER_SIZE];

        if(!PipeIO::readExact(
            pipe, 
            responseHeader,
            sizeof(responseHeader)
        )) {
            std::cerr << "sendRequest failed reading response header: "
                      << GetLastError() << std::endl;
            return false;
        }

        uint32_t responseSize = Protocol::decodeLength(responseHeader);

        if(!Protocol::isValidLength(responseSize)) {
            std::cerr << "sendRequest failed: invalid response length "
                      << responseSize << std::endl;
            return false;
        }

        // Receive response payload
        response.resize(responseSize);

        if(responseSize > 0 && 
            !PipeIO::readExact(pipe, response.data(), responseSize)) 
        {
            std::cerr << "sendRequest failed reading response payload: "
                      << GetLastError() << std::endl;
            response.clear();
            return false;
        }

        return true;
    }

    void disconnect() override {
        if(pipe != INVALID_HANDLE_VALUE) {
            CloseHandle(pipe);
            pipe = INVALID_HANDLE_VALUE;
        }
    }

    ~NamedPipeClient() override {
        disconnect();
    }
};

IpcClient* createIpcClient() {
    return new NamedPipeClient();
}