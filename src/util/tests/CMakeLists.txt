foreach(source_file
    IndexAllocator
)
    set(test_file "test_${source_file}")
    add_executable("${test_file}" "${test_file}.cpp")
    target_link_libraries( "${test_file}"
        PUBLIC
        "${PROJECT_NAME}_util"
        ${CONAN_LIBS}
    )
    add_test("TEST_util_${source_file}" "${test_file}")
endforeach()
