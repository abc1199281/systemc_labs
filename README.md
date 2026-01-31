# SystemC Project Template - For Software Engineers

## Motivation: Bridging Software and Hardware Design

This repository provides a template for creating, building, and testing SystemC projects using CMake. Our main goal is to **bridge the gap between software development and hardware design**, making system-level modeling accessible and understandable for software engineers.

In today's world, where hardware and software are deeply intertwined, understanding the early stages of chip design and system architecture is becoming increasingly important for software professionals. SystemC allows us to use familiar C++ programming concepts to describe, simulate, and verify hardware systems. Think of it as a special "building block design software" that lets software engineers design hardware components using code and see how these components interact before any actual hardware is built. This approach is crucial for **Hardware-Software Co-design**, enabling better collaboration and earlier validation between hardware and software teams.

## Levels of Exploration: Your Learning Path

To help you navigate the world of SystemC, we've structured this repository into different levels, similar to how you might learn a new software framework or library. Each level builds upon the previous one, guiding you from fundamental concepts to more advanced system-level modeling techniques.

### [Basic]: Core Concepts & Software Analogies

*   **Goal:** Understand the fundamental components of SystemC and how they relate to familiar software concepts.
*   **What you'll learn:**
    *   **SystemC Modules:** Imagine these as your software `Classes` or `Components`. They encapsulate a specific piece of hardware functionality.
    *   **SystemC Processes:** These are like `Threads` or `Concurrent Tasks` in software. They define the behavior of your hardware module over time, running in parallel.
    *   **SystemC Signals:** Think of these as `Shared Variables` or `Message Queues` that allow different hardware modules (or software components) to communicate with each other.
    *   You'll start with simple examples, learning how to describe basic hardware behaviors using C++ code.

### [Middle]: Advanced Features & Design Patterns

*   **Goal:** Explore more complex communication mechanisms and structural aspects of SystemC, drawing parallels to software design patterns and protocols.
*   **What you'll learn:**
    *   **Ports, Channels, and Interfaces:** These are SystemC's way of defining how modules connect and communicate, much like `Interfaces` and `Communication Protocols` in software. They provide a structured way for components to talk without knowing each other's internal details.
    *   You'll work with more practical module examples, such as `uart_tx` (a simple serial data sender) or `apb_timer` (a basic timer), to understand how these communication elements bring a system to life in simulation.

### [Advanced]: System-Level Modeling & Performance Exploration

*   **Goal:** Dive into complex system-level modeling techniques, including Transaction-Level Modeling (TLM), and understand how SystemC can be used to evaluate system performance and architectural decisions.
*   **What you'll learn:**
    *   **Transaction-Level Modeling (TLM):** This is a powerful technique that allows you to model system interactions at a higher level of abstraction, focusing on "what" happens rather than "how" it happens. Think of it as designing a high-level API for hardware communication.
    *   You'll explore sophisticated modules like `simple_DMA` (a basic Direct Memory Access controller) or `arbitrator` (a component that manages access to shared resources), understanding how to simulate and analyze the behavior of complex hardware controllers. This helps in making early architectural choices for system optimization.

## Prerequisites: Your Development Environment Setup

Before you can build and run these SystemC projects, you'll need a few essential tools. Think of these as the compiler, build system, and testing frameworks you'd typically set up for a C++ software project.

*   **SystemC Library:** This is the core C++ library that enables you to write hardware-like descriptions. You need to have it installed, and an environment variable `SYSTEMC_HOME` should point to its installation path. (e.g., `export SYSTEMC_HOME=/usr/local/systemc-2.3.3`)
    *   **Analogy:** This is similar to needing a specific SDK or framework (like Qt or Boost) installed for a C++ application.
*   **CMake:** A cross-platform build system generator (Version 3.14 or higher). It helps manage the compilation process, much like `make` or `msbuild` in traditional software development.
*   **Catch2:** A C++ testing framework (Version 3.x). We use this to write and run unit tests for our SystemC modules, ensuring they behave as expected.
    *   **Installation (Debian/Ubuntu):** `sudo apt-get install catch2`
*   **A C++ Compiler:** A modern C++ compiler (e.g., GCC, Clang) that supports C++17 standards. This is your standard C++ compiler used to turn your source code into executable programs.

## Building Your SystemC Project: A Software Engineer's Workflow

Building a SystemC project will feel familiar if you've worked with CMake-based C++ projects before. Here's a step-by-step guide:

1.  **Clone the repository:** (You've likely already done this!)
    ```bash
    git clone <repository-url>
    cd <repository-name>
    ```

2.  **Create a build directory:** It's good practice to keep your compiled files separate from your source code.
    ```bash
    mkdir build
    cd build
    ```

3.  **Configure the project with CMake:** This step checks for all necessary libraries and generates the build files (e.g., Makefiles). Make sure your `SYSTEMC_HOME` environment variable is set correctly before running this.
    ```bash
    cmake ..
    ```
    *   **Analogy:** This is similar to running `configure` scripts or setting up your IDE project for the first time.

4.  **Compile the project:** This command actually compiles your C++ code and links it with the SystemC library to create the executables.
    ```bash
    make
    ```
    *   **Analogy:** This is your `make` command, `g++` compilation, or building the project in your IDE.

## Running Tests: Ensuring Your Hardware Behaves Like Software

After a successful build, you'll want to verify that your SystemC modules are working correctly. Our tests are organized like unit tests in software development.

To run the unit tests:
```bash
./build/tests/unit_tests/run_unit_tests
```

## Project Structure: Where Everything Lives

The project is organized to keep things tidy and and understandable, much like a well-structured software repository:

*   `src/modules/`: This is where you'll find the **source code for your SystemC hardware modules** – essentially, the C++ code that describes the behavior of individual hardware blocks or components.
*   `thirdparty/`: Contains external libraries or modified official examples.
    *   `accellera/`: Specifically for code derived from Accellera SystemC officially provided examples.
*   `tests/`: This directory holds all the test benches and verification code for your SystemC modules.
    *   `unit/`: Contains **unit tests** for your original modules in `src/modules/`, ensuring each module functions as intended in isolation.
    *   `thirdparty_verify/`: Verification tests for the code in `thirdparty/` to ensure they work as expected in your environment.

## Adding a New Module: Extending Your Hardware-Software System

Adding a new SystemC module is like adding a new `Class` or `Component` to your software project. Here's how to integrate a new hardware module:

1.  Create a new subdirectory for your module inside the `src/modules/` directory (e.g., `src/modules/my_new_ip`).
2.  Your module's subdirectory should contain:
    *   `include/`: For your module's header files (`.h` or `.hpp`).
    *   `src/`: For your module's source files (`.cpp`).
    *   `CMakeLists.txt`: To define how to build your module as a library.
3.  In your module's `CMakeLists.txt`, define a library and link it against the SystemC library:
    ```cmake
    add_library(my_new_ip
        src/my_new_ip.cpp
        include/my_new_ip.h
    )

    target_include_directories(my_new_ip PUBLIC include)
    target_link_libraries(my_new_ip PUBLIC SystemC::systemc)
    ```
    *   **Analogy:** This is similar to defining a new library target in CMake and linking it to other dependencies.
4.  Add the new module's directory to the root `CMakeLists.txt` file (you'll find a section for `add_subdirectory` calls).
    ```cmake
    add_subdirectory(src/modules/my_new_ip)
    ```
    *   **Note:** For larger projects, you might automate this step using CMake's `file(GLOB ...)` commands, but for clarity and explicit control, listing them individually is often preferred.
5.  (Optional) Add a new test file for your module in `tests/unit/` and update its `CMakeLists.txt` to include your new test. This ensures your new module is thoroughly tested.


## Credits
Some parts of this repository are derived from the official [Accellera SystemC](https://github.com/accellera-official/systemc) examples and source code, under the Apache License 2.0.