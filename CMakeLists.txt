cmake_minimum_required(VERSION 3.26)
project(grpc-server)

#linking source files here
add_executable(${PROJECT_NAME}
    src/main.cpp
)

# we would do it with headers here (include dir>>)

target_include_directories(${PROJECT_NAME} 
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)