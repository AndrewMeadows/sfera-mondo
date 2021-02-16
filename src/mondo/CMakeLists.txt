set(TARGET_NAME mondo)

add_library(${TARGET_NAME} STATIC
    Server.cpp
    Server.h
)

#target_include_directories(${TARGET_NAME} PUBLIC ${lib_dir})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${TARGET_NAME} PRIVATE
    sferamondo_util
    #grpc
    #protobuf
)

#install(TARGETS ${TARGET_NAME} DESTINATION lib)
