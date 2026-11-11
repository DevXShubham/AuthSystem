#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <memory>

#include "ipc_server.h"
#include "socket_io.h"
#include "unix_socket_config.h"

class UnixSocketConnection : public IpcConnection {

private:
    int socketFd;

public:
    explicit UnixSocketConnection(int socketFd) 
        : socketFd(socketFd) {}

    ~UnixSocketConnection() override {
        if(socketFd > 0) 
            close(socketFd);
    }

    bool readExact(void* data, size_t size) override {
        return SocketIO::readExact(socketFd, data, size);
    }

    bool writeAll(const void* data, size_t size) override {
        return SocketIO::writeAll(socketFd, data, size);
    }
};

class UnixSocketServer : public IpcServer {

private: 
    int serverFd = -1;

public:
    ~UnixSocketServer() override {
        if(serverFd >= 0) 
            close(serverFd);

        unlink(UNIX_SOCKET_PATH);
    }

    bool start() override {
        serverFd = socket(AF_UNIX, SOCK_STREAM, 0);
 
        if(serverFd < 0) {
            std::cerr << "Socket creation failed: "
                    << strerror(errno) << std::endl;

            return false;
        }

        sockaddr_un address{};

        address.sun_family = AF_UNIX;

        std::strncpy(
            address.sun_path,
            UNIX_SOCKET_PATH,
            sizeof(address.sun_path) - 1
        );

        unlink(UNIX_SOCKET_PATH);

        if(bind(
            serverFd,
            reinterpret_cast<sockaddr*>(&address),
            sizeof(address)) < 0) 
        {
            std::cerr << "Bind failed: "
                    << strerror(errno) << std::endl;
            return false;
        }

        if (chmod(UNIX_SOCKET_PATH, 0600) < 0)
        {
            std::cerr << "Socket permission setup failed: "
                      << strerror(errno) << std::endl;
            return false;
        }

        if (listen(serverFd, SOMAXCONN) < 0)
        {
            std::cerr << "Listen failed: "
                      << strerror(errno) << std::endl;
            return false;
        }

        std::cout << "Unix socket server started. "
                  << "Waiting for clients..." << std::endl;

        return true;
    }

    std::unique_ptr<IpcConnection> acceptClient() override {
        int clientFd;

        do
        {
            clientFd = accept(serverFd, nullptr, nullptr);
        }
        while (clientFd < 0 && errno == EINTR);
        
        if(clientFd < 0) {
             std::cerr << "Accept failed: "
                      << strerror(errno) << std::endl;
            return nullptr;
        }

        if (!SocketIO::configureSocket(clientFd))
        {
            close(clientFd);
            return nullptr;
        }

        return std::make_unique<UnixSocketConnection>(clientFd);
    }
};

std::unique_ptr<IpcServer> createIpcServer() {
    return std::make_unique<UnixSocketServer>();
}