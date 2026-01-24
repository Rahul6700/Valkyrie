# minimum version of cmake needed
cmake_minimum_required(VERSION 3.16)

project(Valkyrie
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# the files we wanna execute
# creating a exe called valkyrie from the following .cpp files
add_executable(valkyrie
    main.cpp
    src/Connection.cpp
    src/RespProtocol.cpp
    src/ValkeyClient.cpp
)

# when building the exe valkyrie, add /include to the header search path
target_include_directories(valkyrie
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)




