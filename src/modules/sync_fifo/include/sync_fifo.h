#ifndef SYNC_FIFO_H
#define SYNC_FIFO_H

#include <systemc.h>

template <int DATA_WIDTH = 32, int FIFO_DEPTH = 16>
class SyncFIFO : public sc_module {
public:
  // --- Ports ---
  sc_in<bool> clk;
  sc_in<bool> rst_n;

  // Write Interface
  sc_in<bool> wr_en;
  sc_in<sc_uint<DATA_WIDTH>> din;
  sc_out<bool> full;

  // Read Interface
  sc_in<bool> rd_en;
  sc_out<sc_uint<DATA_WIDTH>> dout;
  sc_out<bool> empty;

  // --- Constructor ---
  SC_HAS_PROCESS(SyncFIFO);
  SyncFIFO(sc_module_name name) : sc_module(name) {
    SC_CTHREAD(fifo_process, clk.pos());
    reset_signal_is(rst_n, false);
  }

private:
  // --- Internal Memory ---
  sc_uint<DATA_WIDTH> mem[FIFO_DEPTH];

  // --- Pointers ---
  // Using N+1 bit pointers to distinguish Full/Empty states.
  // The lower N bits address the memory (0 to FIFO_DEPTH-1).
  // The MSB (bit N) is used to detect wrap-around.
  // We use sc_uint<32> for simplicity, relying on modulo arithmetic.
  sc_uint<32> w_ptr;
  sc_uint<32> r_ptr;

  void fifo_process() {
    // Reset Logic
    w_ptr = 0;
    r_ptr = 0;
    full.write(false);
    empty.write(true);
    dout.write(0);

    wait();

    while (true) {
      // Sample Inputs
      bool w_req = wr_en.read();
      bool r_req = rd_en.read();
      sc_uint<DATA_WIDTH> data_in = din.read();

      // Current state pointers
      uint32_t current_w = w_ptr.to_uint();
      uint32_t current_r = r_ptr.to_uint();

      // --- Status Flag Logic ---
      // Empty: Pointers are identical.
      bool is_empty = (current_w == current_r);

      // Full: Pointers differ by exactly FIFO_DEPTH (MSB differs, lower bits
      // match). Logic checked dynamically using Pointer Difference.
      uint32_t current_ptr_diff =
          (current_w >= current_r) ? (current_w - current_r)
                                   : (2 * FIFO_DEPTH - (current_r - current_w));
      bool is_full = (current_ptr_diff == FIFO_DEPTH);

      // --- Write Operation ---
      if (w_req && !is_full) {
        uint32_t w_addr = current_w % FIFO_DEPTH;
        mem[w_addr] = data_in;
        w_ptr = (current_w + 1) % (2 * FIFO_DEPTH);
      }

      // --- Read Operation ---
      if (r_req && !is_empty) {
        // Increment read pointer.
        r_ptr = (current_r + 1) % (2 * FIFO_DEPTH);
      }

      // --- Update Outputs for Next Cycle ---
      uint32_t next_w = w_ptr.to_uint();
      uint32_t next_r = r_ptr.to_uint();

      // Update Flags based on new state
      bool next_empty = (next_w == next_r);

      int count = (int)next_w - (int)next_r;
      if (count < 0)
        count += (2 * FIFO_DEPTH);
      bool next_full = (count == FIFO_DEPTH);

      full.write(next_full);
      empty.write(next_empty);

      // Look-ahead Output: Always show data at the current Read Pointer
      uint32_t r_addr = next_r % FIFO_DEPTH;
      dout.write(mem[r_addr]);

      wait();
    }
  }
};

#endif // SYNC_FIFO_H
