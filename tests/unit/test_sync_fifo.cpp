// Catch2 main runner
#define CATCH_CONFIG_RUNNER
#include "sync_fifo.h"
#include <catch2/catch_all.hpp>
#include <systemc.h>

// Define Clock Period
const sc_time CLK_PERIOD(10, SC_NS);

int sc_main(int argc, char *argv[]) {
  int result = Catch::Session().run(argc, argv);
  return result;
}

TEST_CASE("SyncFIFO Functionality", "[sync_fifo]") {
  // --- Signals ---
  sc_clock clk("clk", CLK_PERIOD);
  sc_signal<bool> rst_n("rst_n");

  // Write Interface
  sc_signal<bool> wr_en("wr_en");
  sc_signal<sc_uint<32>> din("din");
  sc_signal<bool> full("full");

  // Read Interface
  sc_signal<bool> rd_en("rd_en");
  sc_signal<sc_uint<32>> dout("dout");
  sc_signal<bool> empty("empty");

  // --- Instantiate DUT ---
  // Using depth=4 for easier testing of full/wrap conditions
  SyncFIFO<32, 4> dut("fifo_dut");

  // --- Connect DUT ---
  dut.clk(clk);
  dut.rst_n(rst_n);
  dut.wr_en(wr_en);
  dut.din(din);
  dut.full(full);
  dut.rd_en(rd_en);
  dut.dout(dout);
  dut.empty(empty);

  // --- Helper: Push Data ---
  auto fifo_push = [&](uint32_t data) {
    din.write(data);
    wr_en.write(true);
    sc_start(CLK_PERIOD);
    wr_en.write(false);
  };

  // --- Helper: Pop Data ---
  auto fifo_pop = [&]() {
    rd_en.write(true);
    sc_start(CLK_PERIOD);
    rd_en.write(false);
  };

  // --- Trace Setup ---
  sc_trace_file *tf = sc_create_vcd_trace_file("sync_fifo_wave");
  sc_trace(tf, clk, "clk");
  sc_trace(tf, rst_n, "rst_n");
  sc_trace(tf, wr_en, "wr_en");
  sc_trace(tf, din, "din");
  sc_trace(tf, full, "full");
  sc_trace(tf, rd_en, "rd_en");
  sc_trace(tf, dout, "dout");
  sc_trace(tf, empty, "empty");

  // --- Test Sequence ---

  // 1. Reset Phase
  rst_n.write(0);
  wr_en.write(0);
  rd_en.write(0);
  din.write(0);
  sc_start(CLK_PERIOD * 3);

  rst_n.write(1);
  sc_start(CLK_PERIOD); // Wait for reset release

  // Verify Initial State (Empty)
  REQUIRE(empty.read() == true);
  REQUIRE(full.read() == false);

  // 2. Write Operation (Push A)
  std::cout << "Test: Push A (0x10)" << std::endl;
  din.write(0x10);
  wr_en.write(true);
  sc_start(CLK_PERIOD);
  wr_en.write(false);

  // Verify State after single write
  REQUIRE(empty.read() == false);
  REQUIRE(full.read() == false);

  // Verify Look-ahead Output
  REQUIRE(dout.read() == 0x10);

  // 3. Fill FIFO (Push B, C, D)
  std::cout << "Test: Fill FIFO (Push B, C, D)" << std::endl;
  fifo_push(0x20); // B
  fifo_push(0x30); // C
  fifo_push(0x40); // D

  // Verify Full State (Depth 4: A, B, C, D)
  REQUIRE(full.read() == true);
  REQUIRE(empty.read() == false);
  REQUIRE(dout.read() == 0x10); // Head is still A

  // 4. Overflow Protection
  std::cout << "Test: Overflow Protection (Push E)" << std::endl;
  fifo_push(0x50); // Should be ignored

  // Verify state remains unchanged (still Full, Head is A)
  REQUIRE(full.read() == true);
  REQUIRE(dout.read() == 0x10);

  // 5. Read Operation (Pop A)
  std::cout << "Test: Pop A" << std::endl;
  fifo_pop();

  // Verify State after pop (Not Full, Head becomes B)
  REQUIRE(full.read() == false);
  REQUIRE(dout.read() == 0x20);

  // 6. Empty the FIFO
  fifo_pop();                   // Pop B
  REQUIRE(dout.read() == 0x30); // Head is C

  fifo_pop();                   // Pop C
  REQUIRE(dout.read() == 0x40); // Head is D

  fifo_pop(); // Pop D

  // Verify Empty State
  REQUIRE(empty.read() == true);
  REQUIRE(full.read() == false);

  // 7. Underflow Protection
  std::cout << "Test: Underflow Protection (Pop from Empty)" << std::endl;
  fifo_pop(); // Should be ignored

  // 8. Wrap-Around Logic
  std::cout << "Test: Circular Buffer Wrap-Around" << std::endl;
  // Write two items (0x60, 0x70) which wraps w_ptr
  fifo_push(0x60);
  fifo_push(0x70);

  // Pop one item
  fifo_pop();
  REQUIRE(dout.read() == 0x70); // Head should be 0x70

  sc_close_vcd_trace_file(tf);
}
