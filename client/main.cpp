#include "auth/auth.h"
#include <iostream>

int main()
{
    std::string username = "test_user";
    std::string password = "test_password1";
    int result = auth_login(username.data(), password.data());
    
    if(result == 0) {
        std::cout << "Login successful! username: " << username << std::endl;
    } 
    else {
        std::cout << "Login failed. Error code: "<< result << std::endl;
    }
}