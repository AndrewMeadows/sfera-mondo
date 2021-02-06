foreach(source_file
    ConfigUtil
    IndexAllocator
    NetUtil
    RecentHistory
)
    set(test_file "test_${source_file}")
    add_executable("${test_file}" "${test_file}.cpp")
    target_link_libraries( "${test_file}"
        PUBLIC
        gtest
        pthread
        sferamondo_util
    )
    add_test("TEST_util_${source_file}" "${test_file}")
endforeach()
