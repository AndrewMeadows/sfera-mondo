foreach(source_file
    IndexAllocator
    NetUtil
)
    set(test_file "test_${source_file}")
    add_executable("${test_file}" "${test_file}.cpp")
    target_link_libraries( "${test_file}"
        PUBLIC
        sferamondo_util
        gtest
        pthread
    )
    add_test("TEST_util_${source_file}" "${test_file}")
endforeach()
