# SystemManager Unit Tests with doctes

# Try to find doctest if not already found (for standalone builds)
if(NOT TARGET doctest::doctest)
    find_package(doctest REQUIRED)
endif()

# Option for code coverage
option(ENABLE_COVERAGE "Enable code coverage" OFF)

# Strict compilation flags for all tests
set(STRICT_COMPILE_FLAGS "")
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(STRICT_COMPILE_FLAGS
        -Wall              # Enable all common warnings
        -Wextra            # Enable extra warnings
        -Wpedantic         # Strict ISO C++ compliance
        # -Werror          # Treat warnings as errors (disabled to allow compilation)
        -Wshadow           # Warn about variable shadowing
        -Wcast-align       # Warn about pointer cast alignment issues
        -Wunused           # Warn about unused variables/functions
        # -Wconversion     # Warn about implicit conversions (can be noisy)
        # -Wsign-conversion # Warn about sign conversions (can be noisy)
        -Wnull-dereference # Warn about null pointer dereferences
        -Wdouble-promotion # Warn about float->double promotions
        -Wformat=2         # Extra format string checks
    )

    # Additional flags for GCC
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        list(APPEND STRICT_COMPILE_FLAGS
            -Wduplicated-cond     # Warn about duplicated conditions
            -Wduplicated-branches # Warn about duplicated branches
            -Wlogical-op          # Warn about logical operations
        )
    endif()

    # Additional flags for Clang
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        list(APPEND STRICT_COMPILE_FLAGS
            -Wmost                # Most warnings
        )
    endif()
elseif(MSVC)
    set(STRICT_COMPILE_FLAGS
        /W4      # Warning level 4
        # /WX    # Treat warnings as errors (disabled to allow compilation)
        /permissive- # Strict standard conformance
    )
endif()

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

target_compile_options(system_manager_tests PRIVATE ${STRICT_COMPILE_FLAGS})

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

target_compile_options(component_signature_tests PRIVATE ${STRICT_COMPILE_FLAGS})

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

target_compile_options(entity_tests PRIVATE ${STRICT_COMPILE_FLAGS})

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

target_compile_options(component_storage_tests PRIVATE ${STRICT_COMPILE_FLAGS})

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

target_compile_options(entity_manager_tests PRIVATE ${STRICT_COMPILE_FLAGS})

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

target_compile_options(component_manager_tests PRIVATE ${STRICT_COMPILE_FLAGS})

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

target_compile_options(world_tests PRIVATE ${STRICT_COMPILE_FLAGS})

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
