cmake_minimum_required(VERSION 3.14)
project(MySystemC_Repo)

enable_testing()

set(CMAKE_CXX_STANDARD 17)

if (NOT DEFINED ENV{SYSTEMC_HOME})
    message(FATAL_ERROR "SYSTEMC_HOME environment variable is not set. Please set it to your SystemC installation directory.")
endif()

set(SYSTEMC_INCLUDE_DIR "$ENV{SYSTEMC_HOME}/include")
set(SYSTEMC_LIBRARY_DIR "$ENV{SYSTEMC_HOME}/lib-linux64")
set(SYSTEMC_LIBRARY_FILE "${SYSTEMC_LIBRARY_DIR}/libsystemc.so") # Assuming shared library

if (NOT EXISTS "${SYSTEMC_INCLUDE_DIR}")
    message(FATAL_ERROR "SystemC include directory not found: ${SYSTEMC_INCLUDE_DIR}")
endif()

if (NOT EXISTS "${SYSTEMC_LIBRARY_FILE}")
    message(FATAL_ERROR "SystemC library file not found: ${SYSTEMC_LIBRARY_FILE}")
endif()

add_library(SystemC::systemc UNKNOWN IMPORTED)
set_target_properties(SystemC::systemc PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${SYSTEMC_INCLUDE_DIR}"
    IMPORTED_LOCATION "${SYSTEMC_LIBRARY_FILE}"
)

message(STATUS "Found SystemC at $ENV{SYSTEMC_HOME}")
message(STATUS "SystemC include directory: ${SYSTEMC_INCLUDE_DIR}")
message(STATUS "SystemC library file: ${SYSTEMC_LIBRARY_FILE}")

add_subdirectory(src/modules/hello_counter)
add_subdirectory(src/modules/uart_tx)
add_subdirectory(src/modules/apb_timer)
add_subdirectory(src/modules/arbitrator)
add_subdirectory(tests/unit)
add_subdirectory(tests/thirdparty_verify)