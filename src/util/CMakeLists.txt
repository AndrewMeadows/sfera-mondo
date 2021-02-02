set(TARGET_NAME "sferamondo_util")

add_library( ${TARGET_NAME}
    IndexAllocator.h
    ConfigUtil.cpp
    ConfigUtil.h
    GrpcUtil.cpp
    GrpcUtil.h
    LogUtil.cpp
    LogUtil.h
    NetUtil.cpp
    NetUtil.h
    RandomUtil.cpp
    RandomUtil.h
    ThreadPool.h
    TimeUtil.cpp
    TimeUtil.h
    TraceMacros.h
    TraceUtil.cpp
    TraceUtil.h
    Uuid.cpp
    Uuid.h
)

target_include_directories(${TARGET_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")

target_link_libraries( ${TARGET_NAME}
    PUBLIC
    fmt
    grpc
)

add_subdirectory(tests)
