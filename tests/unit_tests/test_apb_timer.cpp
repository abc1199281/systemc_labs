// Catch2 main runner
#define CATCH_CONFIG_RUNNER
#include <catch2/catch_all.hpp>
#include <systemc.h>
#include "apb_timer.h"

// Define Clock Period
const sc_time CLK_PERIOD(10, SC_NS);

int sc_main(int argc, char* argv[]) {
    int result = Catch::Session().run(argc, argv);
    return result;
}

// --- Test Cases ---

TEST_CASE("APB_TIMER Functionality", "[timer]") {
    // --- Signals ---
    sc_clock clk("clk", CLK_PERIOD);
    sc_signal<bool> rst_n("rst_n");
    
    // APB Signals
    sc_signal<bool> psel("psel");
    sc_signal<bool> penable("penable");
    sc_signal<bool> pwrite("pwrite");
    sc_signal<sc_uint<32>> paddr("paddr");
    sc_signal<sc_uint<32>> pwdata("pwdata");
    sc_signal<sc_uint<32>> prdata("prdata");
    
    // Interrupt Output
    sc_signal<bool> irq("irq");

    // --- Instantiate DUT ---
    APB_TIMER dut("dut");

    // --- Connect signals ---
    dut.clk(clk);
    dut.rst_n(rst_n);
    dut.PSEL(psel);
    dut.PENABLE(penable);
    dut.PWRITE(pwrite);
    dut.PADDR(paddr);
    dut.PWDATA(pwdata);
    dut.PRDATA(prdata);
    dut.irq(irq);

    // --- Helper Lambda for APB Write ---
    auto apb_write = [&](uint32_t addr, uint32_t data) {
        // 1. Setup Phase
        paddr.write(addr);
        pwdata.write(data);
        pwrite.write(true); // Write mode
        psel.write(true);
        penable.write(false);
        sc_start(CLK_PERIOD); // Wait 1 cycle

        // 2. Access Phase
        penable.write(true);
        sc_start(CLK_PERIOD); // Wait 1 cycle (Write happens here)

        // 3. Idle
        psel.write(false);
        penable.write(false);
        sc_start(SC_ZERO_TIME); // Advance delta cycle to ensure idle state propagates
    };

    // --- Helper Lambda for APB Read ---
    auto apb_read = [&](uint32_t addr) -> uint32_t {
        // 1. Setup Phase
        paddr.write(addr);
        pwrite.write(false); // Read mode
        psel.write(true);
        penable.write(false);
        sc_start(CLK_PERIOD); 

        // 2. Access Phase
        penable.write(true);
        sc_start(CLK_PERIOD); // Data valid at the end of this cycle

        // Capture Data
        uint32_t read_val = prdata.read().to_uint();

        // 3. Idle
        psel.write(false);
        penable.write(false);
        sc_start(SC_ZERO_TIME); // Advance delta cycle to ensure idle state propagates
        
        return read_val;
    };

    // --- Trace File (Optional, for waveform viewing) ---
    sc_trace_file *tf = sc_create_vcd_trace_file("apb_timer_wave");
    sc_trace(tf, clk, "clk");
    sc_trace(tf, rst_n, "rst_n");
    sc_trace(tf, paddr, "paddr");
    sc_trace(tf, pwdata, "pwdata");
    sc_trace(tf, prdata, "prdata");
    sc_trace(tf, irq, "irq");
    // Trace internal variable if needed (requires friend class or public)
    sc_trace(tf, dut.r_val, "r_val"); 

    // --- Simulation Start ---

    // 1. Reset Phase
    rst_n.write(0);
    psel.write(0);
    penable.write(0);
    sc_start(CLK_PERIOD * 2);
    
    rst_n.write(1);
    sc_start(CLK_PERIOD);

    // Initial check: IRQ should be low
    REQUIRE(irq.read() == false);

    // 2. Test "Prime" Logic (The Corner Case)
    // Scenario: Write LOAD = 10. Check if VAL becomes 10 immediately.
    // Addresses: CTRL=0x00, LOAD=0x04, VAL=0x08
    
    std::cout << "Testing LOAD priming logic..." << std::endl;
    apb_write(0x04, 10); // Write 10 to LOAD register

    // Read back VAL register (0x08)
    uint32_t current_val = apb_read(0x08);
    
    // CRITICAL CHECK: VAL should be 10, not 0!
    REQUIRE(current_val == 10); 
    
    // Also read back LOAD register to be sure
    REQUIRE(apb_read(0x04) == 10);


    // 3. Test Countdown and IRQ
    // Scenario: Set LOAD=3, Enable Timer. 
    // Sequence should be: 3 -> 2 -> 1 -> 0 -> (IRQ + Reload to 3) -> 2...
    
    std::cout << "Testing Countdown and IRQ..." << std::endl;

    // A. Update LOAD to 3 (small number for quick testing)
    apb_write(0x04, 3);
    REQUIRE(apb_read(0x08) == 3); // Verify Prime again

    // B. Enable Timer (Write 1 to CTRL)
    apb_write(0x00, 1);

    // Now we step cycle by cycle manually to watch the counter
    // Current VAL is 3.

    // Cycle 1: 3 -> 2
    sc_start(CLK_PERIOD); 
    REQUIRE(dut.get_r_val() == 1); // Directly check r_val, bypassing APB read logic
    // Wait! apb_read advances time, so it interferes with "cycle by cycle" checking if the timer is running.
    // Since the timer is running, every sc_start inside apb_read advances the timer.
    
    // Let's Disable Timer to Reset and do a pure step test without APB reads in between
    apb_write(0x00, 0); // Disable
    apb_write(0x04, 3); // Reload 3 (Prime). Internal counter r_val is now 3.
    apb_write(0x00, 1); // Enable timer. Countdown starts on the next cycle.

    // State: r_val=3, IRQ=0.
    
    // [Cycle 1] Timer logic sees r_val=3. At the end of this cycle, r_val becomes 2.
    sc_start(CLK_PERIOD); 
    REQUIRE(dut.get_r_val() == 2);
    
    // [Cycle 2] Timer logic sees r_val=2. At the end of this cycle, r_val becomes 1.
    sc_start(CLK_PERIOD);
    REQUIRE(dut.get_r_val() == 1);

    // [Cycle 3] Timer logic sees r_val=1. At the end of this cycle, r_val becomes 0.
    sc_start(CLK_PERIOD);
    REQUIRE(dut.get_r_val() == 0);
    
    // [Cycle 4] Timer logic sees r_val=0. THIS IS THE MOMENT.
    // Hardware should fire IRQ and reload r_val to 3 in this cycle.
    sc_start(CLK_PERIOD);
    REQUIRE(irq.read() == true); // IRQ should be high now!
    
    // Step 5: Logic sees IRQ condition handled. 
    // Should clear IRQ and decrement 3 -> 2.
    sc_start(CLK_PERIOD);
    
    REQUIRE(irq.read() == false); // IRQ should be cleared (Pulse)

    // 4. Test Disable
    apb_write(0x00, 0); // Write 0 to CTRL
    sc_start(CLK_PERIOD * 5); // Wait a bit
    REQUIRE(irq.read() == false); // Should produce no more IRQs

    // sc_close_vcd_trace_file(tf);
}