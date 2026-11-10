#pragma once 

#include <string>

class IpcClient 
{
public:
    virtual ~IpcClient() = default;

    virtual bool connect() = 0; 

    virtual bool sendRequest(
        const std::string& request,
        std::string& response
    ) = 0;

    virtual void disconnect() = 0;
};

// implemented by each platform
IpcClient* createIpcClient();