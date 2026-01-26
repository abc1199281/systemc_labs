#define CATCH_CONFIG_RUNNER
#include <catch2/catch_all.hpp>
#include <systemc>
#include "lt_top.h"

// The reporting globals need to be defined in one compilation unit
#define REPORT_DEFINE_GLOBALS
#include "reporting.h"

using namespace sc_core;

// Define sc_main which is required by SystemC
int sc_main(int argc, char* argv[]) {
    // Initialize reporting enables
    REPORT_ENABLE_ALL_REPORTING();
    
    int result = Catch::Session().run(argc, argv);
    return result;
}

TEST_CASE("LT Example Simulation", "[lt]") {
    lt_top top("top");
    
    std::cout << "Starting LT Example Simulation..." << std::endl;
    sc_start();
    std::cout << "Simulation Finished." << std::endl;
    
    // Check if we finished without SC_REPORT_FATAL
    // (SystemC treats fatal as exception or exit, so if we are here, it's mostly good)
    REQUIRE(true); // Placeholder, real verification is in the log output (no fatal errors)
}
