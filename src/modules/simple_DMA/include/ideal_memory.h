#ifndef IDEAL_MEMORY_H
#define IDEAL_MEMORY_H

#include <map>
#include <systemc.h>

SC_MODULE(IdealMemory) {
public:
  sc_in<bool> clk;
  sc_in<bool> rst_n;

  // Interface matching DMA Master
  sc_in<bool> req_valid;
  sc_out<bool> req_ready;
  sc_in<sc_uint<32>> req_addr;
  sc_in<bool> req_op;          // 0=Read, 1=Write
  sc_in<sc_uint<32>> data_out; // Data from DMA (Write)

  sc_out<sc_uint<32>> data_in; // Data to DMA (Read)
  sc_out<bool> data_valid;     // Data valid for Read

  std::map<uint32_t, uint32_t> mem;

  SC_CTOR(IdealMemory) {
    SC_CTHREAD(mem_process, clk.pos());
    reset_signal_is(rst_n, false);
  }

  void mem_process() {
    req_ready.write(0);
    data_valid.write(0);
    data_in.write(0);
    wait();

    while (true) {
      req_ready.write(1); // Always ready to accept request in this cycle

      if (req_valid.read()) {
        bool is_write = req_op.read();
        sc_uint<32> addr = req_addr.read();

        if (is_write) {
          sc_uint<32> wdata = data_out.read();
          mem[addr] = wdata;
          // Write doesn't need data_valid response in this protocol.
          // No acl signal in this simplified protocol.
          data_valid.write(0);
        } else {
          // Read
          // Respond next cycle to simulate latency 1.

          data_valid.write(
              0); // Cannot respond immediately if we are just accepting now.
          // Actually, if we accepted (Ready=1, Valid=1), we process.

          // Simple Delay: 1 cycle
          wait();
          req_ready.write(0); // Busy processing? or Pipelined?
          // Let's say we are busy sending data.
          // Process only one data at a time.

          if (mem.find(addr) != mem.end()) {
            data_in.write(mem[addr]);
          } else {
            data_in.write(0); // Default 0
          }
          data_valid.write(1);

          wait();
          data_valid.write(0);
          req_ready.write(1); // Ready for next

          // Note: This logic consumes `wait()` calls, so `req_ready` toggling
          // needs care. The loop `wait()` is at the end. If we do `wait()`
          // inside, we need to handle the next loop iteration correctly.
          continue; // Skip the end-of-loop wait
        }
      } else {
        data_valid.write(0);
      }

      wait();
    }
  }

  // Helper to debug/preload
  void write_mem(uint32_t addr, uint32_t data) { mem[addr] = data; }

  uint32_t read_mem(uint32_t addr) {
    if (mem.find(addr) != mem.end())
      return mem[addr];
    return 0;
  }
};

#endif // IDEAL_MEMORY_H
