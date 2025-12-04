# SystemC Project Template

This repository provides a template for creating, building, and testing SystemC projects using CMake.

## Prerequisites

Before you begin, ensure you have the following installed:

*   **SystemC:** A C++ library for system-level modeling. You need to have it installed and the `SYSTEMC_HOME` environment variable set to your SystemC installation path. For example:
    ```bash
    export SYSTEMC_HOME=/usr/local/systemc-2.3.3
    ```
*   **CMake:** A cross-platform build system generator. (Version 3.14 or higher)
*   **Catch2:** A C++ test framework. (Version 3.x)
    *   On Debian/Ubuntu, you can install it with: `sudo apt-get install catch2`
*   **A C++ Compiler:** A modern C++ compiler (e.g., GCC, Clang) that supports C++17.

## Building the Project

1.  **Clone the repository:**
    ```bash
    git clone <repository-url>
    cd <repository-name>
    ```

2.  **Create a build directory:**
    ```bash
    mkdir build
    cd build
    ```

3.  **Configure the project with CMake:**
    Make sure your `SYSTEMC_HOME` environment variable is set correctly.
    ```bash
    cmake ..
    ```

4.  **Compile the project:**
    ```bash
    make
    ```

## Running Tests

After a successful build, the test executables will be located in the `build/tests/unit_tests` directory.

To run the unit tests:
```bash
./build/tests/unit_tests/run_unit_tests
```

## Project Structure

The project is organized into the following directories:

*   `common/`: Contains common utilities, bus protocols, and other shared code.
*   `ips/`: Contains individual Intellectual Property (IP) blocks. Each IP has its own subdirectory with its source code and a `CMakeLists.txt` file.
*   `tests/`: Contains test benches.
    *   `unit_tests/`: Unit tests for the IPs.
    *   `system_tests/`: System-level tests.

## Adding a New IP

To add a new IP to the project:

1.  Create a new subdirectory for your IP inside the `ips/` directory (e.g., `ips/my_new_ip`).
2.  Your IP's subdirectory should contain:
    *   `include/`: For your IP's header files.
    *   `src/`: For your IP's source files.
    *   `CMakeLists.txt`: To define how to build your IP as a library.
3.  In your IP's `CMakeLists.txt`, define a library and link it against SystemC:
    ```cmake
    add_library(my_new_ip
        src/my_new_ip.cpp
        include/my_new_ip.h
    )

    target_include_directories(my_new_ip PUBLIC include)

    # Link against the SystemC library
    target_link_libraries(my_new_ip PUBLIC SystemC::systemc)
    ```
4.  Add the new IP's directory to the root `CMakeLists.txt` file:
    ```cmake
    # In the root CMakeLists.txt
    add_subdirectory(ips/my_new_ip)
    ```
5.  (Optional) Add a new test file for your IP in `tests/unit_tests/` and update `tests/unit_tests/CMakeLists.txt` to include it in the test build.
