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

*   `src/modules/`: Contains original Intellectual Property (IP) blocks and logic you've implemented.
*   `thirdparty/`: Contains external libraries or modified official examples.
    *   `accellera/`: Specifically for code derived from Accellera SystemC officially provided examples.
*   `tests/`: Contains test benches.
    *   `unit/`: Unit tests for your original modules in `src/modules/`.
    *   `thirdparty_verify/`: Verification tests for the code in `thirdparty/` to ensure they work as expected in your environment.

## Adding a New Module

To add a new hardware module or IP to the project:

1.  Create a new subdirectory for your module inside the `src/modules/` directory (e.g., `src/modules/my_new_ip`).
2.  Your module's subdirectory should contain:
    *   `include/`: For your module's header files.
    *   `src/`: For your module's source files.
    *   `CMakeLists.txt`: To define how to build your module as a library.
3.  In your module's `CMakeLists.txt`, define a library and link it against SystemC:
    ```cmake
    add_library(my_new_ip
        src/my_new_ip.cpp
        include/my_new_ip.h
    )

    target_include_directories(my_new_ip PUBLIC include)
    target_link_libraries(my_new_ip PUBLIC SystemC::systemc)
    ```
4.  Add the new module's directory to the root `CMakeLists.txt` file:
    ```cmake
    add_subdirectory(src/modules/my_new_ip)
    ```
5.  (Optional) Add a new test file for your module in `tests/unit/` and update its `CMakeLists.txt`.


## Credits
Some parts of this repository are derived from the official [Accellera SystemC](https://github.com/accellera-official/systemc) examples and source code, under the Apache License 2.0.