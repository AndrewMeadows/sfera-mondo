set(TARGET_NAME hello_server)

add_executable (${TARGET_NAME} main.cpp)

target_link_libraries (${TARGET_NAME}
    PUBLIC
    sferamondo_util
    fmt
)

