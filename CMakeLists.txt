cmake_minimum_required(VERSION 3.10)

project(DocSync VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

file(GLOB_RECURSE MY_CLIENT_SOURCES
    src/client/client.cpp
    src/client/**/*cpp
)

file(GLOB_RECURSE MY_SERVER_SOURCES
    src/server/server.cpp
    src/server/**/*cpp
)

if(NOT MY_CLIENT_SOURCES OR NOT MY_SERVER_SOURCES)
    message(FATAL_ERROR "No source files found. Check the paths.")
else()
    add_executable(myClient ${MY_CLIENT_SOURCES})
    add_executable(myServer ${MY_SERVER_SOURCES})
endif()

add_custom_target(run_client
    COMMAND ${CMAKE_COMMAND} -E env ./myClient
    DEPENDS myClient
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    COMMENT "Building and running myClient"
)

add_custom_target(run_server
    COMMAND ${CMAKE_COMMAND} -E env ./myServer
    DEPENDS myServer
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    COMMENT "Building and running myServer"
)
