# Example executable for SystemManager demonstration

add_executable(SystemManagerExample
    SystemManagerExample.cpp
)

target_link_libraries(SystemManagerExample
    PRIVATE engineCore
)

target_include_directories(SystemManagerExample
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set_target_properties(SystemManagerExample PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
)
