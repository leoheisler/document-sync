# Define a versão mínima do CMake
cmake_minimum_required(VERSION 3.10)

# Define o nome do projeto e a versão
project(DocSync VERSION 1.0)

# Define a versão do padrão C++
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Adiciona os diretórios de inclusão
include_directories(include)

# Adiciona os arquivos de origem
file(GLOB_RECURSE MY_CLIENT_SOURCES
    src/client/client.cpp
    src/client/**/*cpp
)

file(GLOB_RECURSE MY_SERVER_SOURCES
    src/server/server.cpp
)

# Verifica se as variáveis de fonte estão vazias
if(NOT MY_CLIENT_SOURCES OR NOT MY_SERVER_SOURCES)
    message(FATAL_ERROR "No source files found. Check the paths.")
else()
    # Cria o executável
    add_executable(myClient ${MY_CLIENT_SOURCES})
    add_executable(myServer ${MY_SERVER_SOURCES})
endif()

# Adiciona o target personalizado 'run_client' para compilar e executar o cliente
add_custom_target(run_client
    COMMAND ${CMAKE_COMMAND} -E env ./myClient
    DEPENDS myClient
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    COMMENT "Building and running myClient"
)

# Adiciona o target personalizado 'run_server' para compilar e executar o servidor
add_custom_target(run_server
    COMMAND ${CMAKE_COMMAND} -E env ./myServer
    DEPENDS myServer
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    COMMENT "Building and running myServer"
)
