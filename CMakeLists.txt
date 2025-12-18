cmake_minimum_required(VERSION 3.26)
project(grpc-server)


find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(utf8_range REQUIRED)

set(GENPROTO_LIB "genproto_lib")
set(GENPROTO_DIR "${PROJECT_SOURCE_DIR}/genproto")

file(GLOB_RECURSE PROTO_SRC "${GENPROTO_DIR}/*.pb.cc")
file(GLOB_RECURSE PROTO_HDR "${GENPROTO_DIR}/*.pb.h")

add_library(${GENPROTO_LIB} ${PROTO_SRC} ${PROTO_HDR})

target_link_libraries(${GENPROTO_LIB}
    PUBLIC
        protobuf::libprotobuf
        gRPC::grpc++
        utf8_range::utf8_range
        utf8_range::utf8_validity
)

target_include_directories(${GENPROTO_LIB}
    PUBLIC
        ${GENPROTO_DIR}
)

#linking source files here
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/service/order_service.cpp
    src/server/server.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${GENPROTO_LIB}
)
# we would do it with headers here (include dir>>)

target_include_directories(${PROJECT_NAME} 
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# cleint here>>

add_executable(client
    src/client.cpp
)

target_link_libraries(client
    PRIVATE
        ${GENPROTO_LIB}
)
# we would do it with headers here (include dir>>)

target_include_directories(client 
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
