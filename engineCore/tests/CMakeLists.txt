# SystemManager Unit Tests with doctest

# Try to find doctest if not already found (for standalone builds)
if(NOT TARGET doctest::doctest)
    find_package(doctest REQUIRED)
endif()

# Option for code coverage
option(ENABLE_COVERAGE "Enable code coverage" OFF)

# Add coverage flags if enabled
if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(COVERAGE_FLAGS --coverage -fprofile-arcs -ftest-coverage)
        message(STATUS "Code coverage enabled")
    endif()
endif()

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

if(ENABLE_COVERAGE)
    target_compile_options(system_manager_tests PRIVATE ${COVERAGE_FLAGS})
    target_link_options(system_manager_tests PRIVATE ${COVERAGE_FLAGS})
endif()

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

if(ENABLE_COVERAGE)
    target_compile_options(component_signature_tests PRIVATE ${COVERAGE_FLAGS})
    target_link_options(component_signature_tests PRIVATE ${COVERAGE_FLAGS})
endif()

set_target_properties(component_signature_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
)

# Entity Tests
add_executable(entity_tests
    EntityTests.cpp
)

target_link_libraries(entity_tests
    PRIVATE
        engineCore
        doctest::doctest
)

target_include_directories(entity_tests
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

if(ENABLE_COVERAGE)
    target_compile_options(entity_tests PRIVATE ${COVERAGE_FLAGS})
    target_link_options(entity_tests PRIVATE ${COVERAGE_FLAGS})
endif()

set_target_properties(entity_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
)

# Component Storage Tests
add_executable(component_storage_tests
    ComponentStorageTests.cpp
)

target_link_libraries(component_storage_tests
    PRIVATE
        engineCore
        doctest::doctest
)

target_include_directories(component_storage_tests
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

if(ENABLE_COVERAGE)
    target_compile_options(component_storage_tests PRIVATE ${COVERAGE_FLAGS})
    target_link_options(component_storage_tests PRIVATE ${COVERAGE_FLAGS})
endif()

set_target_properties(component_storage_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
)

# Entity Manager Tests
add_executable(entity_manager_tests
    EntityManagerTests.cpp
)

target_link_libraries(entity_manager_tests
    PRIVATE
        engineCore
        doctest::doctest
)

target_include_directories(entity_manager_tests
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

if(ENABLE_COVERAGE)
    target_compile_options(entity_manager_tests PRIVATE ${COVERAGE_FLAGS})
    target_link_options(entity_manager_tests PRIVATE ${COVERAGE_FLAGS})
endif()

set_target_properties(entity_manager_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
)

# Component Manager Tests
add_executable(component_manager_tests
    ComponentManagerTests.cpp
)

target_link_libraries(component_manager_tests
    PRIVATE
        engineCore
        doctest::doctest
)

target_include_directories(component_manager_tests
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

if(ENABLE_COVERAGE)
    target_compile_options(component_manager_tests PRIVATE ${COVERAGE_FLAGS})
    target_link_options(component_manager_tests PRIVATE ${COVERAGE_FLAGS})
endif()

set_target_properties(component_manager_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
)

# World Tests
add_executable(world_tests
    WorldTests.cpp
)

target_link_libraries(world_tests
    PRIVATE
        engineCore
        doctest::doctest
)

target_include_directories(world_tests
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

if(ENABLE_COVERAGE)
    target_compile_options(world_tests PRIVATE ${COVERAGE_FLAGS})
    target_link_options(world_tests PRIVATE ${COVERAGE_FLAGS})
endif()

set_target_properties(world_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
)

# Add tests to CTest
enable_testing()
add_test(NAME SystemManagerTests COMMAND system_manager_tests)
add_test(NAME ComponentSignatureTests COMMAND component_signature_tests)
add_test(NAME EntityTests COMMAND entity_tests)
add_test(NAME ComponentStorageTests COMMAND component_storage_tests)
add_test(NAME EntityManagerTests COMMAND entity_manager_tests)
add_test(NAME ComponentManagerTests COMMAND component_manager_tests)
add_test(NAME WorldTests COMMAND world_tests)
