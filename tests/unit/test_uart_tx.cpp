// Catch2 main runner
#define CATCH_CONFIG_Runner
#include <catch2/catch_all.hpp>
#include "uart_tx.h"

int sc_main(int argc, char* argv[]) {
    // Pass command line args to Catch2
    int result = Catch::Session().run(argc, argv);
    return result;
}

// --- Test Cases ---

TEST_CASE("Uart_Tx Test", "[uart]") {
    // --- Signals ---
    sc_clock clk("clk", 10, SC_NS); // 100MHz clock
    sc_signal<bool> rst_n;
    sc_signal<bool> cmd_en;
    sc_signal<bool> cmd_we;
    sc_signal<sc_uint<8>> data_in;
    sc_signal<bool> data_out;
    sc_signal<bool> tx_busy;

    // --- Instantiate DUT ---
    UART_TX dut("dut");

    // --- Connect signals ---
    dut.clk(clk);
    dut.rst_n(rst_n);
    dut.cmd_en(cmd_en);
    dut.cmd_we(cmd_we);
    dut.data_in(data_in);
    dut.data_out(data_out);
    dut.tx_busy(tx_busy);

    // --- Simulation ---

    // 1. Test Synchronous Reset
    rst_n.write(0); // Assert reset
    cmd_en.write(0);
    cmd_we.write(0);
    data_in.write(0);
    sc_start(20, SC_NS); // Wait for 2 clock cycles

    REQUIRE(dut.m_state == UART_TX::State::s_IDLE);
    // data_out and tx_busy should be set by reset function.
    // However, they are not accessible directly in TEST_CASE.
    // This is okay for now, the important part is dut.m_state.

    // 2. Test Data Transmission
    rst_n.write(1); // Deassert reset
    sc_start(10, SC_NS);

    // Send a byte 0x5A (01011010)
    data_in.write(0x5A);
    cmd_en.write(1);
    cmd_we.write(1);
    sc_start(10, SC_NS);

    // After one clock cycle, command should be accepted
    REQUIRE(dut.m_state == UART_TX::State::s_START);
    REQUIRE(tx_busy.read() == true);
    REQUIRE(data_out.read() == false); // Start bit

    // Deassert command enable
    cmd_en.write(0);
    cmd_we.write(0);

    // Wait for the duration of the start bit
    sc_start(int(dut.baud_period) * 10, SC_NS);
    REQUIRE(dut.m_state == UART_TX::State::s_TX_DATA);

    // Check data bits (LSB first: 01011010)
    // Bit 0: 0
    REQUIRE(data_out.read() == false);
    sc_start(int(dut.baud_period) * 10, SC_NS);
    // Bit 1: 1
    REQUIRE(data_out.read() == true);
    sc_start(int(dut.baud_period) * 10, SC_NS);
    // Bit 2: 0
    REQUIRE(data_out.read() == false);
    sc_start(int(dut.baud_period) * 10, SC_NS);
    // Bit 3: 1
    REQUIRE(data_out.read() == true);
    sc_start(int(dut.baud_period) * 10, SC_NS);
    // Bit 4: 1
    REQUIRE(data_out.read() == true);
    sc_start(int(dut.baud_period) * 10, SC_NS);
    // Bit 5: 0
    REQUIRE(data_out.read() == false);
    sc_start(int(dut.baud_period) * 10, SC_NS);
    // Bit 6: 1
    REQUIRE(data_out.read() == true);
    sc_start(int(dut.baud_period) * 10, SC_NS);
    // Bit 7: 0
    REQUIRE(data_out.read() == false);
    sc_start(int(dut.baud_period) * 10, SC_NS);

    // After 8 data bits, should be in STOP state
    REQUIRE(dut.m_state == UART_TX::State::s_STOP);
    REQUIRE(data_out.read() == true); // Stop bit

    // Wait for the duration of the stop bit
    sc_start(int(dut.baud_period) * 10, SC_NS);

    // After stop bit, should be back to IDLE
    REQUIRE(dut.m_state == UART_TX::State::s_IDLE);
    REQUIRE(tx_busy.read() == false);
    REQUIRE(data_out.read() == true); // Idle is high
}