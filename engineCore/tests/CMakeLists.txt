# SystemManager Unit Tests with doctest

find_package(doctest REQUIRED)

add_executable(system_manager_tests
    SystemManagerTests.cpp
)

target_link_libraries(system_manager_tests
    PRIVATE
        engineCore
        doctest::doctest
)

target_include_directories(system_manager_tests
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set_target_properties(system_manager_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
)

# Component Signature Tests
add_executable(component_signature_tests
    ComponentSignatureTests.cpp
)

target_link_libraries(component_signature_tests
    PRIVATE
        engineCore
        doctest::doctest
)

target_include_directories(component_signature_tests
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set_target_properties(component_signature_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
)

# Add tests to CTest
enable_testing()
add_test(NAME SystemManagerTests COMMAND system_manager_tests)
add_test(NAME ComponentSignatureTests COMMAND component_signature_tests)
