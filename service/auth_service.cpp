#include <windows.h>
#include <stdexcept>
#include <iostream>
#include <string>

#include <nlohmann/json.hpp>

#include "protocol.h"
#include "ipc_server.h"

using json = nlohmann::json;

constexpr const char* PIPE_NAME = R"(\\.\pipe\AuthServicePipe)";

std::string createResponseData(const std::string& request)
{
    try
    {
        json requestJson = json::parse(request);

        if (!requestJson.is_object() ||
            !requestJson.contains("action") ||
            !requestJson.contains("username") ||
            !requestJson.contains("password") ||
            !requestJson["action"].is_string() ||
            !requestJson["username"].is_string() ||
            !requestJson["password"].is_string())
        {
            return json{
                {"success", false},
                {"message", "Invalid request"}
            }.dump();
        }

        std::string action = requestJson["action"];
        std::string username = requestJson["username"];
        std::string password = requestJson["password"];

        bool authenticated = false;

        if (action != "login") {
            return json{
                {"success", false},
                {"message", "Unsupported action"}
            }.dump();
        }

        authenticated = (username == "test_user" && password == "test_password");

        json responseJson = {
            {"success", authenticated},
            {"message", authenticated
                ? "Login successful"
                : "Authentication failed"}
        };

        return responseJson.dump();
    }
    catch (const std::exception&)
    {
        return json{
            {"success", false},
            {"message", "Invalid request"}
        }.dump();
    }
}

void handleClient(IpcConnection& connection)
{
    unsigned char header[Protocol::HEADER_SIZE];

    if (!connection.readExact(header, sizeof(header))) {
        std::cerr << "Couldn't read request header! "
                  << GetLastError() << std::endl;
        return;
    }

    uint32_t requestSize = Protocol::decodeLength(header);

    if (!Protocol::isValidLength(requestSize)) {
        std::cerr << "Invalid payload length!" << std::endl;
        return;
    }

    std::string request(requestSize, '\0');

    if (requestSize != 0 &&
        !connection.readExact(request.data(), requestSize)) {
        std::cerr << "Couldn't read request payload! "
                  << GetLastError() << std::endl;
        return;
    }

    std::cout << "Received authentication request. \n";

    std::string responseData = createResponseData(request);
    unsigned char responseHeader[Protocol::HEADER_SIZE];

    Protocol::encodeLength(
        static_cast<uint32_t>(responseData.size()),
        responseHeader
    );

    if (!connection.writeAll(responseHeader, sizeof(responseHeader))) {
        std::cerr << "Couldn't write response header! "
                << GetLastError() << std::endl;
        return;
    } 

    if (!responseData.empty() &&
        !connection.writeAll(responseData.data(),
                static_cast<DWORD>(responseData.size()))) {
        std::cerr << "Couldn't write response payload! "
                << GetLastError() << std::endl;
        return;
    }
}

int main()
{
    std::cout << "Auth service starting..." << std::endl;
    
    std::unique_ptr<IpcServer> server = createIpcServer();

    if(!server || !server->start()) 
        return 1;

    while (true) {
        // HANDLE pipe = CreateNamedPipeA(
        //     PIPE_NAME, 
        //     PIPE_ACCESS_DUPLEX,
        //     PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        //     PIPE_UNLIMITED_INSTANCES,
        //     1024, 
        //     1024, 
        //     0, 
        //     nullptr
        // );
        
        // std::cout << "Waiting for client..." << std::endl;
 
        // BOOL connected = ConnectNamedPipe(pipe, nullptr) 
        //                 ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

        // if(!connected) {
        //     std::cerr   << "Connection failed!" 
        //                 << GetLastError() << std::endl;
        //     CloseHandle(pipe);
        //     continue;
        // }

        // FlushFileBuffers(pipe);
        // DisconnectNamedPipe(pipe);
        // CloseHandle(pipe);

        auto client = server->acceptClient();
        
        if(!client) 
            continue;

        handleClient(*client);
    }

    return 0;
}