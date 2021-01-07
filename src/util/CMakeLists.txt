set(TARGET_NAME "${PROJECT_NAME}_util")

add_library( ${TARGET_NAME}
    IndexAllocator.h
    NetUtil.h
    NetUtil.cpp
)

target_include_directories(${TARGET_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
#target_include_directories(${TARGET_NAME} PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/..")

#set(CONAN_DEPENDENCIES
#    CONAN_PKG::fmt
#    CONAN_PKG::glm
#    CONAN_PKG::grpc
#    CONAN_PKG::nlohmann_json
#    CONAN_PKG::protobuf
#)

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
