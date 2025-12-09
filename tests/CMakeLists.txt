project(tests)

find_package(doctest REQUIRED)

add_executable(unit_tests
    test_server_concurrency.cpp
)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/network/include
    ${CMAKE_SOURCE_DIR}/common/include
)

target_link_libraries(unit_tests PRIVATE
    network
    doctest::doctest
)
