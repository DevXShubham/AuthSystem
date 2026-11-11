cmake_minimum_required(VERSION 3.20)

project(AuthSystem LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)

FetchContent_MakeAvailable(nlohmann_json)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Authentication library
add_library(auth_sdk SHARED
    sdk/auth.cpp
)

# Authentication service
add_executable(auth_service
    service/auth_service.cpp
)

# Authentication client
add_executable(auth_client
    client/main.cpp
)

target_include_directories(auth_service PRIVATE 
    common 
    service
)

target_include_directories(auth_sdk
    PUBLIC include
    PRIVATE common sdk
)

target_include_directories(auth_client PRIVATE include)

if(WIN32) 

    target_sources(auth_sdk PRIVATE
        platform/windows/named_pipe_client.cpp
        platform/windows/pipe_io.cpp
    )

    target_sources(auth_service PRIVATE
        platform/windows/named_pipe_server.cpp 
        platform/windows/pipe_io.cpp
    )

    target_include_directories(auth_service 
        PRIVATE platform/windows
    )

    target_include_directories(auth_sdk
        PRIVATE platform/windows
    )

elseif(UNIX) 

    target_sources(auth_sdk PRIVATE
        platform/posix/unix_socket_client.cpp
        platform/posix/socket_io.cpp
    )

    target_sources(auth_service PRIVATE
        platform/posix/unix_socket_server.cpp
        platform/posix/socket_io.cpp
    )

    target_include_directories(auth_sdk PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/platform/posix
    )

    target_include_directories(auth_service PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/platform/posix
    )
endif()

target_link_libraries(auth_client PRIVATE auth_sdk)
target_link_libraries(auth_sdk PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(auth_service PRIVATE nlohmann_json::nlohmann_json)

target_compile_definitions(auth_sdk PRIVATE AUTH_SDK_EXPORTS)