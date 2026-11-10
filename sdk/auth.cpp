#include "auth/auth.h"
#include "ipc_client.h"
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int auth_login(const char* username, const char* password) {
    if(!username || !password) {
        return -1; // Invalid credentials
    }
    std::unique_ptr<IpcClient> client(createIpcClient());

    if(!client || !client->connect())
        return -2; // Client connection to server failed
 
    json requestJson = {
        {"action", "login"}, 
        {"username", username},
        {"password", password}
    };

    std::string request = requestJson.dump();

    std::string response;

    if(!client->sendRequest(request, response))
        return -3; // Sending request failed

    try
    {
        json responseJson = json::parse(response);

        if(!responseJson.contains("success") || !responseJson["success"].is_boolean()) {
            return -5; // Invalid response
        }

        return responseJson["success"].get<bool>() ? 0 : -6; // Login success / wrong credentials
    }
    catch(const json::exception&)
    {
        return -5; // Json parsing error
    }
}