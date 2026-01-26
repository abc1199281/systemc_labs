#include <catch2/catch_all.hpp>
#include <systemc.h>
#include "arbitrator.h"

int sc_main(int argc, char* argv[]) {
    int result = Catch::Session().run(argc, argv);
    return result;
}


TEST_CASE("Arbitrator Basic Functionality", "[arbitrator]") {
    arbitrator arb("arb", 3);
    
    // Signals
    sc_vector<sc_signal<bool>> req_sig("req_sig", 3);
    sc_vector<sc_signal<bool>> gnt_sig("gnt_sig", 3);

    // Connect signals
    arb.req(req_sig);
    arb.gnt(gnt_sig);

    // Start simulation
    sc_start(0, SC_NS);

    // SECTION("No requests")
    {
        req_sig[0].write(false);
        req_sig[1].write(false);
        req_sig[2].write(false);
        sc_start(1, SC_NS);

        REQUIRE(gnt_sig[0].read() == false);
        REQUIRE(gnt_sig[1].read() == false);
        REQUIRE(gnt_sig[2].read() == false);
    }

    // SECTION("Single request - Highest Priority")
    {
        req_sig[0].write(true);
        req_sig[1].write(false);
        req_sig[2].write(false);
        sc_start(1, SC_NS);

        REQUIRE(gnt_sig[0].read() == true);
        REQUIRE(gnt_sig[1].read() == false);
        REQUIRE(gnt_sig[2].read() == false);
    }

    // SECTION("Single request - Middle Priority")
    {
        req_sig[0].write(false);
        req_sig[1].write(true);
        req_sig[2].write(false);
        sc_start(1, SC_NS);

        REQUIRE(gnt_sig[0].read() == false);
        REQUIRE(gnt_sig[1].read() == true);
        REQUIRE(gnt_sig[2].read() == false);
    }

    // SECTION("Multiple requests - 0 and 1")
    {
        req_sig[0].write(true);
        req_sig[1].write(true);
        req_sig[2].write(false);
        sc_start(1, SC_NS);

        // Priority is 0 > 1 > 2
        REQUIRE(gnt_sig[0].read() == true);
        REQUIRE(gnt_sig[1].read() == false);
        REQUIRE(gnt_sig[2].read() == false);
    }

    // SECTION("Multiple requests - 1 and 2")
    {
        req_sig[0].write(false);
        req_sig[1].write(true);
        req_sig[2].write(true);
        sc_start(1, SC_NS);

        // Priority is 0 > 1 > 2
        REQUIRE(gnt_sig[0].read() == false);
        REQUIRE(gnt_sig[1].read() == true);
        REQUIRE(gnt_sig[2].read() == false);
    }
}
