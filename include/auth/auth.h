#pragma once 

#ifdef _WIN32 
    #ifdef AUTH_SDK_EXPORTS
        #define AUTH_API __declspec(dllexport)
    #else
        #define AUTH_API __declspec(dllimport)
    #endif 
#else 
    #define AUTH_API __attribute__((visibility("default")))
#endif 

#ifdef __cplusplus
extern "C" {
#endif 

AUTH_API int auth_login(
    const char* username, 
    const char* password
);


#ifdef __cplusplus
}
#endif