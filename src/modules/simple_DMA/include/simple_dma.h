#ifndef SIMPLE_DMA_H
#define SIMPLE_DMA_H

#include <systemc.h>

SC_MODULE(SimpleDMA) {
public:
  // --- Ports ---
  sc_in<bool> clk;
  sc_in<bool> rst_n;

  // APB Slave Interface
  sc_in<bool> psel;
  sc_in<bool> penable;
  sc_in<bool> pwrite;
  sc_in<sc_uint<32>> paddr;
  sc_in<sc_uint<32>> pwdata;
  sc_out<sc_uint<32>> prdata;
  sc_out<bool> pready; // Always 1 in this spec

  // DMA Master Interface (Request Channel)
  sc_out<bool> m_req_valid;
  sc_in<bool> m_req_ready;
  sc_out<sc_uint<32>> m_req_addr;
  sc_out<bool> m_req_op; // 0=Read, 1=Write

  // DMA Master Interface (Data Channel)
  sc_out<sc_uint<32>> m_data_out;
  sc_in<sc_uint<32>> m_data_in;
  sc_in<bool> m_data_valid; // For Read

  // Interrupt
  sc_out<bool> irq;

  // --- Constructor ---
  SC_CTOR(SimpleDMA);

  // --- Processes ---
  void apb_process();
  void dma_engine_process();

private:
  // --- Registers ---
  sc_uint<32> reg_csr;      // 0x00
  sc_uint<32> reg_src_addr; // 0x04
  sc_uint<32> reg_dst_addr; // 0x08
  sc_uint<32> reg_len;      // 0x0C

  // CSR Bits
  static const int CSR_START_BIT = 0;
  static const int CSR_BUSY_BIT = 1;
  static const int CSR_DONE_BIT = 2;

  // --- Internal FSM State ---
  enum State { IDLE, READ_REQ, WAIT_DATA, WRITE_REQ };
  sc_signal<int> current_state; // Using int for enum in sc_signal, or could use
                                // a custom type

  // Internal counters/buffers
  sc_uint<32> current_src;
  sc_uint<32> current_dst;
  sc_uint<32> words_left; // Spec says "Transfer Length (in Words)"
  sc_uint<32> data_buffer;

  void reset();
};

#endif // SIMPLE_DMA_H
