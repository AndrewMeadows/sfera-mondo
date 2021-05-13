add_proto_autogen_cpp_library(mondo_proto proto/mondo.proto)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../submodules/abseil-cpp)
add_subdirectory(util)
add_subdirectory(mondo)
