project(tests)

option(FETCH_KCOV_IF_MISSING "Download kcov via FetchContent when kcov is not available" OFF)

find_package(doctest REQUIRED)

add_executable(unit_tests
    Test_server_concurrency.cpp
)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/network/include
    ${CMAKE_SOURCE_DIR}/common/include
)

target_link_libraries(unit_tests PRIVATE
    network
    doctest::doctest
)

find_program(KCOV_PATH kcov)

if(NOT KCOV_PATH AND FETCH_KCOV_IF_MISSING)
    message(STATUS "kcov not found on system. Attempting to fetch and build kcov via CMake...")
    include(FetchContent)
    FetchContent_Declare(
        kcov
        GIT_REPOSITORY https://github.com/SimonKagstrom/kcov.git
        GIT_TAG v42
    )
    FetchContent_GetProperties(kcov)
    if(NOT kcov_POPULATED)
        FetchContent_Populate(kcov)
        add_subdirectory(${kcov_SOURCE_DIR} ${kcov_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()

    if(TARGET kcov)
        set(KCOV_PATH $<TARGET_FILE:kcov>)
    endif()
elseif(NOT KCOV_PATH)
    message(STATUS "kcov not found and FETCH_KCOV_IF_MISSING is OFF; coverage target will emit a warning")
endif()

if(KCOV_PATH)
    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
        COMMAND ${KCOV_PATH} --include-pattern=${CMAKE_SOURCE_DIR} --exclude-pattern=${CMAKE_BINARY_DIR},${CMAKE_SOURCE_DIR}/tests,${CMAKE_SOURCE_DIR}/conan ${CMAKE_BINARY_DIR}/coverage $<TARGET_FILE:unit_tests>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running kcov..."
    )
else()
    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} -E echo "Error: kcov could not be found or built. Please install dependencies (libdw-dev, libcurl4-openssl-dev) or install kcov manually."
        COMMAND ${CMAKE_COMMAND} -E false
    )
endif()
