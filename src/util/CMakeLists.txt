set(TARGET_NAME "util")

add_library( ${TARGET_NAME}
    IndexAllocator.h
    LogUtil.cpp
    LogUtil.h
    NetUtil.cpp
    NetUtil.h
    RandomUtil.cpp
    RandomUtil.h
    TraceMacros.h
    TraceUtil.cpp
    TraceUtil.h
    TimeUtil.cpp
    TimeUtil.h
    Uuid.cpp
    Uuid.h
)

target_include_directories(${TARGET_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")

if (${APPLE})
    target_link_libraries(${TARGET_NAME}
        "-framework CoreFoundation"
        ${CONAN_LIBS}
    )
else()
    target_link_libraries( ${TARGET_NAME}
        PUBLIC
        ${CONAN_LIBS}
    )
endif()

add_subdirectory(tests)
