#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>

#include "ipc_client.h"
#include "protocol.h"
#include "socket_io.h"
#include "unix_socket_config.h"

class UnixSocketClient : public IpcClient {

private:
    int socketFd = -1;

public:
    ~UnixSocketClient() override {
        disconnect();
    }

    bool connect() override {
        socketFd = socket(AF_UNIX, SOCK_STREAM, 0);

        if (socketFd < 0)
        {
            std::cerr << "Socket creation failed: "
                      << strerror(errno) << std::endl;
            return false;
        }

        if (!SocketIO::configureSocket(socketFd))
        {
            disconnect();
            return false;
        }

        sockaddr_un address{};

        address.sun_family = AF_UNIX;

        std::strncpy(
            address.sun_path,
            UNIX_SOCKET_PATH,
            sizeof(address.sun_path) - 1
        );
        
        if (::connect(
                socketFd,
                reinterpret_cast<sockaddr*>(&address),
                sizeof(address)) < 0)
        {
            std::cerr << "Connection failed: "
                      << strerror(errno) << std::endl;

            disconnect();
            return false;
        }

        return true;
    }

    bool sendRequest(
        const std::string& request,
        std::string& response) override
    {
        if (socketFd < 0)
            return false;

        if(request.size() > Protocol::MAX_MESSAGE_SIZE)
            return false;

        unsigned char header[Protocol::HEADER_SIZE];

        Protocol::encodeLength(
            static_cast<uint32_t>(request.size()),
            header
        );

        // Send request header
        if (!SocketIO::writeAll(socketFd, header, sizeof(header)))
            return false;

        // Send request payload
        if (!request.empty() &&
            !SocketIO::writeAll(
                socketFd,
                request.data(),
                request.size()))
        {
            return false;
        }

        // Receive response header
        unsigned char responseHeader[Protocol::HEADER_SIZE];

        if (!SocketIO::readExact(
                socketFd,
                responseHeader,
                sizeof(responseHeader)))
        {
            return false;
        }

        uint32_t responseSize =
            Protocol::decodeLength(responseHeader);

        if (!Protocol::isValidLength(responseSize))
            return false;
        
         // Receive response payload
        response.resize(responseSize);

        if (responseSize != 0 &&
            !SocketIO::readExact(
                socketFd,
                response.data(),
                responseSize))
        {
            response.clear();
            return false;
        }

        return true;
    }

    void disconnect() override
    {
        if (socketFd >= 0)
        {
            close(socketFd);
            socketFd = -1;
        }
    }
};

IpcClient* createIpcClient()
{
    return new UnixSocketClient();
}