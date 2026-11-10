#include <stdexcept>
#include <iostream>
#include <string>

#include <nlohmann/json.hpp>

#include "protocol.h"
#include "ipc_server.h"

using json = nlohmann::json;

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
        std::cerr << "Couldn't read request header! \n";
                //   << GetLastError() << std::endl;
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
        std::cerr << "Couldn't read request payload! \n";
        return;
    }

    std::cout << "Received authentication request. \n";

    std::string responseData = createResponseData(request);
    unsigned char responseHeader[Protocol::HEADER_SIZE];

    if(responseData.size() > Protocol::MAX_MESSAGE_SIZE) {
        std::cerr << "Response payload too large! \n";
    }

    Protocol::encodeLength(
        static_cast<uint32_t>(responseData.size()),
        responseHeader
    );

    if (!connection.writeAll(responseHeader, sizeof(responseHeader))) {
        std::cerr << "Couldn't write response header! \n";
        return;
    } 

    if (!responseData.empty() &&
        !connection.writeAll(responseData.data(),
                            static_cast<size_t>(responseData.size()))) {
        std::cerr << "Couldn't write response payload! \n";
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
        auto client = server->acceptClient();
        
        if(!client) 
            continue;

        handleClient(*client);
    }

    return 0;
}