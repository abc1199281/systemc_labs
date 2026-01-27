#include "simple_dma.h"

SimpleDMA::SimpleDMA(sc_module_name name) : sc_module(name) {
  SC_CTHREAD(apb_process, clk.pos());
  reset_signal_is(rst_n, false);

  SC_CTHREAD(dma_engine_process, clk.pos());
  reset_signal_is(rst_n, false);

  // Initial values
  reg_csr = 0;
  reg_src_addr = 0;
  reg_dst_addr = 0;
  reg_len = 0;
}

void SimpleDMA::apb_process() {
  // Reset Logic
  pready.write(1); // Always ready
  prdata.write(0);
  irq.write(0);

  // Wait for reset release
  wait();

  while (true) {
    // Sample inputs
    bool sel = psel.read();
    bool en = penable.read();
    bool wr = pwrite.read();
    sc_uint<32> addr = paddr.read();
    sc_uint<32> wdata = pwdata.read();

    // APB logic
    // PSEL=1 and PENABLE=1 indicates a valid transaction phase
    if (sel && en) {
      // Address decoding (Lower 8 bits usually enough for this small map)
      // 0x00: CSR
      // 0x04: SRC
      // 0x08: DST
      // 0x0C: LEN

      sc_uint<32> offset = addr & 0xFF;

      if (wr) {
        // WRITE
        switch (offset) {
        case 0x00: // CSR (Control and Status Register)
          // Bit 0: START (Self-clearing, handled by FSM or Pulse? Spec says
          // "Write 1 to start") We will set a flag or just write it. FSM will
          // check it. Bit 2: DONE (W1C). If wdata[2] is 1, clear DONE.
          {
            sc_uint<32> current_csr = reg_csr;
            if (wdata[CSR_START_BIT]) {
              current_csr[CSR_START_BIT] = 1;
            }
            if (wdata[CSR_DONE_BIT]) {
              current_csr[CSR_DONE_BIT] = 0; // Clear DONE
            }
            // Bit 1 is BUSY (Read Only), ignore write
            // Keep BUSY bit from current state (handled by FSM actually, but
            // for register view) The FSM updates the actual register bits
            // usually. Here we need to be careful about multiple drivers.
            // Better approach: APB writes to "command" signals or shadow regs,
            // FSM updates status. For simplicity in SystemC TLM/RTL: Let's
            // assume shared variable usage is safe within module if carefully
            // managed, or use signals. Since this is CTHREAD, variables are
            // module member variables. We need to coordinate.

            // Let's update reg_csr.
            // FSM reads START, Sets BUSY, Sets DONE.
            // APB writes START, Clears DONE.
            reg_csr = current_csr;
          }
          break;
        case 0x04:
          reg_src_addr = wdata;
          break;
        case 0x08:
          reg_dst_addr = wdata;
          break;
        case 0x0C:
          reg_len = wdata;
          break;
        default:
          break;
        }
      } else {
        // READ
        sc_uint<32> rdata = 0;
        switch (offset) {
        case 0x00:
          rdata = reg_csr;
          break;
        case 0x04:
          rdata = reg_src_addr;
          break;
        case 0x08:
          rdata = reg_dst_addr;
          break;
        case 0x0C:
          rdata = reg_len;
          break;
        default:
          rdata = 0;
          break;
        }
        prdata.write(rdata);
      }
    }

    // Update IRQ based on DONE bit
    irq.write(reg_csr[CSR_DONE_BIT]);

    wait();
  }
}

void SimpleDMA::dma_engine_process() {
  // Reset Logic
  m_req_valid.write(0);
  m_req_addr.write(0);
  m_req_op.write(0);
  m_data_out.write(0);

  current_state.write(IDLE);

  wait();

  while (true) {
    int state = current_state.read();

    switch (state) {
    case IDLE:
      // Ensure request signals are cleared
      m_req_valid.write(0);
      // Check for START bit
      if (reg_csr[CSR_START_BIT]) {
        // Clear START, Set BUSY
        reg_csr[CSR_START_BIT] = 0;
        reg_csr[CSR_BUSY_BIT] = 1;
        reg_csr[CSR_DONE_BIT] = 0; // Clear DONE just in case

        // Load counters
        current_src = reg_src_addr;
        current_dst = reg_dst_addr;
        words_left = reg_len; // It's words actually

        if (reg_len > 0) {
          current_state.write(READ_REQ);
        } else {
          // Zero length transfer, arguably just done
          reg_csr[CSR_BUSY_BIT] = 0;
          reg_csr[CSR_DONE_BIT] = 1;
        }
      }
      break;

    case READ_REQ:
      m_req_valid.write(1);
      m_req_op.write(0); // Read
      m_req_addr.write(current_src);

      // Check if ready
      if (m_req_ready.read()) {
        current_state.write(WAIT_DATA);
        // Deassert valid in next cycle (or keep if we could pipeline, but
        // simple FSM does one by one) We will deassert valid at start of loop
        // if we change state? No, outputs are sticky in CTHREAD until changed.
        // We should deassert valid after handshake.
      }
      break;

    case WAIT_DATA:
      m_req_valid.write(0); // Deassert request

      if (m_data_valid.read()) {
        data_buffer = m_data_in.read();
        current_state.write(WRITE_REQ);
      }
      break;

    case WRITE_REQ:
      m_req_valid.write(1);
      m_req_op.write(1); // Write
      m_req_addr.write(current_dst);
      m_data_out.write(data_buffer);

      if (m_req_ready.read()) {
        // Increment/Decrement pointers and counter
        current_src = current_src + 4; // Assuming 32-bit words
        current_dst = current_dst + 4;
        words_left = words_left - 1;

        if (words_left == 0) {
          // Transfer complete
          reg_csr[CSR_BUSY_BIT] = 0;
          reg_csr[CSR_DONE_BIT] = 1;
          current_state.write(IDLE);
        } else {
          current_state.write(READ_REQ);
        }
      }
      break;
    }

    // Handle Request Valid deassertion if we transitioned out of REQ states
    // Actually handled inside the case blocks:
    // READ_REQ -> WAIT_DATA: m_req_valid set to 0 in WAIT_DATA next cycle.
    // WRITE_REQ -> READ/IDLE: m_req_valid set to 1 or 0.
    // But need to be careful: CTHREAD `wait()` is the clock boundary.
    // If I set `m_req_valid.write(0)` in WAIT_DATA, it happens at the clock
    // edge entering WAIT_DATA. Correct.

    wait();
  }
}
