// Catch2 main runner
#define CATCH_CONFIG_Runner
#include <catch2/catch_all.hpp>
#include "hello_counter.h"

int sc_main(int argc, char* argv[]) {
    // Pass command line args to Catch2
    int result = Catch::Session().run(argc, argv);
    return result;
}

// --- Test Cases ---

TEST_CASE("Hello_Counter Test", "[counter]") {
    // --- Signals ---
    sc_clock clk("clk", 10, SC_NS); // Use a proper clock
    sc_signal<bool> rst_n;
    sc_signal<bool> cmd_en;
    sc_signal<bool> cmd_we;
    sc_signal<sc_uint<8>> addr;
    sc_signal<sc_uint<32>> wdata;
    sc_signal<sc_uint<32>> rdata;

    // --- Instantiate DUT ---
    Hello_Counter dut("dut");
    
    // --- Connect signals ---
    dut.clk(clk);
    dut.rst_n(rst_n);
    dut.cmd_en(cmd_en);
    dut.cmd_we(cmd_we);
    dut.addr(addr);
    dut.wdata(wdata);
    dut.rdata(rdata);

    // --- Simulation ---
    
    // 1. Test Synchronous Reset
    rst_n.write(0); // Assert reset
    cmd_en.write(0);
    cmd_we.write(0);
    sc_start(20, SC_NS); // Wait for 2 clock cycles with reset asserted

    REQUIRE(dut.m_count == 0); // Check that counter is held at 0

    // 2. Test Counting
    rst_n.write(1); // Deassert reset
    sc_start(10, SC_NS); // Wait for 1 clock cycle

    // After the first clock edge post-reset, counter should be 1
    REQUIRE(dut.m_count == 1);

    sc_start(10, SC_NS); // Wait for another clock cycle
    
    // After the second clock edge, counter should be 2
    REQUIRE(dut.m_count == 2);
    
    sc_start(30, SC_NS); // Wait for 3 more clock cycles
    
    REQUIRE(dut.m_count == 5); // Check final count
    
    // 3. Test RW Operation
    addr.write(0x00);
    cmd_en.write(1);
    cmd_we.write(1);
    wdata.write(0xa);
    sc_start(10, SC_NS);     
    REQUIRE(dut.m_count == 0xa);

    cmd_en.write(1);
    cmd_we.write(0);    
    sc_start(10, SC_NS);     
    REQUIRE(rdata.read() == 0xa);

    // 4. Test Write Operation to Invalid Address  
    addr.write(0x10); // Invalid address
    cmd_en.write(1);
    cmd_we.write(1);
    wdata.write(0xABCD1234);
    sc_start(10, SC_NS);     
    REQUIRE(dut.m_count == 0xB); // Count should remain unchanged (0XA) + 1

    cmd_en.write(1);
    cmd_we.write(0);    
    sc_start(10, SC_NS);     
    REQUIRE(rdata.read() == 0xDEADBEEF);    
}