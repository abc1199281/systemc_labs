# 軟體工程師的 RTL / 數位硬體設計學習路線

> 基於 `systemc_labs` repo 現有內容，規劃從軟體工程背景過渡到硬體 RTL 設計的學習路線。

---

## 概觀：你已經有什麼、還缺什麼

### 這個 repo 已涵蓋的能力

| 階段 | 對應 module | 學到的硬體概念 |
|------|------------|---------------|
| 入門 | `hello_counter` | clock/reset、register read/write、SC_METHOD |
| 基礎 | `uart_tx` | FSM (有限狀態機)、serial protocol、baud rate timing |
| 中階 | `apb_timer` | Bus protocol (APB)、memory-mapped register、interrupt generation |
| 中階 | `arbitrator` | 組合邏輯、fixed-priority arbitration、sc_vector |
| 中階 | `sync_fifo` | Circular buffer、pointer-based full/empty detection、SC_CTHREAD |
| 進階 | `simple_DMA` | Multi-state FSM、valid/ready handshake、master/slave interface、多 process 協作 |
| 規格 | `interrupt_controller` | 中斷聚合、masking、priority encoder (尚未實作) |

### 軟體工程師常見的認知斷層

1. **並行 vs 順序** — 軟體是逐行執行的；硬體是所有模組同時在跑
2. **時間的概念** — 軟體不太管 clock cycle；硬體每一個 cycle 都是設計的一部分
3. **資源是實體的** — 軟體隨時可以 `malloc`；硬體的每個 register、wire 都要佔面積
4. **沒有 OS** — 沒有 thread scheduler，所有的「並行」都是真正的硬體並行
5. **介面是 protocol** — function call 變成了多 cycle 的 handshake (valid/ready, APB, AXI)

---

## 學習路線：四個階段

### Phase 1: 鞏固 SystemC 基礎 (利用現有 repo)

**目標**：確保你真正理解 SystemC 的執行模型，而不只是「能跑」。

| 練習 | 說明 |
|------|------|
| 1.1 實作 `interrupt_controller` | repo 裡已經有完整的 spec (`interrupt_controller_spec.md`)，這是最好的起點。包含 APB slave、組合邏輯 masking、priority encoder。 |
| 1.2 為每個現有 module 加入 VCD trace | 用 `sc_trace_file` 產生波形，學會用 GTKWave 觀察信號時序。**看到波形是理解硬體行為最直觀的方式。** |
| 1.3 嘗試用 SC_CTHREAD 重寫 `uart_tx` | 原版用 SC_METHOD，改用 SC_CTHREAD 體會兩種寫法的差異，這直接對應到 synthesizable RTL 的風格。 |
| 1.4 把 `simple_DMA` 接上 `sync_fifo` + `apb_timer` | 做一個小型 SoC：Timer 產生 interrupt → DMA 開始搬資料 → 透過 FIFO 緩衝。練習模組間的信號連接。 |

**里程碑**: 能獨立寫出一個帶 APB interface 的 module，並用 VCD 波形驗證行為。

---

### Phase 2: 跨入真正的 RTL — 學 Verilog/SystemVerilog

**目標**：從 SystemC 的 C++ 抽象降到 RTL 層級，理解「硬體描述」而非「硬體模擬」。

**為什麼需要 Verilog？**
- SystemC 在業界主要用於 system-level modeling 和 verification
- 真正的晶片設計 (ASIC/FPGA) 使用 Verilog / SystemVerilog
- 你在 SystemC 學到的所有概念 (FSM, handshake, register map) 都會直接對應到 Verilog

| 練習 | 說明 |
|------|------|
| 2.1 用 Verilog 重寫 `hello_counter` | 最簡單的 module，直接比對 SystemC vs Verilog 的寫法差異 |
| 2.2 用 Verilog 重寫 `uart_tx` | 練習 `always @(posedge clk)` block、FSM encoding、shift register |
| 2.3 用 Verilog 重寫 `sync_fifo` | 理解 parameterized module (`parameter`)、pointer arithmetic |
| 2.4 用 Verilog 重寫 `apb_timer` | 完整的 APB slave，練習 bus protocol 在 RTL 層面的實作 |

**工具鏈建議**:
- **模擬器**: Icarus Verilog (iverilog) — 免費開源
- **波形檢視**: GTKWave
- **Linting**: Verilator — 也可以當高速模擬器
- **線上練習**: HDLBits (https://hdlbits.01xz.net/) — 瀏覽器內直接寫 Verilog

**里程碑**: 能用 Verilog 從零寫一個 APB peripheral 並用 testbench 驗證。

---

### Phase 3: 建立硬體架構觀 — SoC 與匯流排

**目標**：從單一 module 擴展到整個系統的思維。

| 主題 | 學習內容 | 建議練習 |
|------|---------|---------|
| 3.1 匯流排協議 | AMBA APB → AHB → AXI4 的演進。理解 burst、pipeline、outstanding transaction | 在 repo 的 TLM examples 基礎上，用 Verilog 實作一個 AXI4-Lite slave |
| 3.2 Memory subsystem | SRAM controller、cache basics、memory map | 把 `ideal_memory` 替換成有 latency 的 SRAM model |
| 3.3 Interconnect | Bus fabric、crossbar、decoder | 寫一個簡單的 1-to-N APB decoder，連接 timer + DMA + INTC |
| 3.4 CPU integration | 了解 RISC-V ISA 基礎，觀察 CPU 如何透過 bus 存取 peripheral | 用開源 RISC-V core (如 PicoRV32) 接上你的 peripheral |

**推薦資源**:
- ARM AMBA specification (免費下載)
- "Computer Organization and Design RISC-V Edition" (Patterson & Hennessy)
- 開源 SoC: LiteX, PULP Platform

**里程碑**: 能組裝一個包含 CPU + Bus + 2-3 個 peripheral 的小型 SoC。

---

### Phase 4: 進階主題 — 往專業方向深入

根據你的興趣和職涯目標，選擇一到兩個方向深入：

#### 方向 A: FPGA 實作
- 學習 Xilinx Vivado 或 Intel Quartus 工具鏈
- 把你的 SoC 燒到 FPGA 開發板上 (推薦: Digilent Arty A7 或類似板子)
- 理解 timing constraint、synthesis report、resource utilization

#### 方向 B: ASIC / 驗證 (Verification)
- 學習 SystemVerilog Assertion (SVA)
- 學習 UVM (Universal Verification Methodology)
- 理解 coverage-driven verification
- 這個方向和你的 SystemC 經驗最接近，軟體背景的優勢最大

#### 方向 C: 高階綜合 (HLS)
- 利用你的 C++ / SystemC 基礎，學習 Vitis HLS
- 理解 pragma-driven optimization (#pragma HLS pipeline, unroll)
- 這是軟體工程師最容易切入的路徑，但要注意 HLS 不能完全取代手寫 RTL

#### 方向 D: 硬體安全
- Side-channel analysis, fault injection
- 適合有資安背景的軟體工程師

---

## 具體下一步行動 (建議順序)

基於 repo 的現有狀態，最合理的下一步：

```
1. [NOW]     實作 interrupt_controller (spec 已就緒)
2. [NOW]     為所有 module 加 VCD trace 輸出
3. [NEXT]    組裝 mini-SoC: timer + DMA + FIFO + INTC
4. [NEXT]    安裝 iverilog + GTKWave，開始用 Verilog 重寫簡單模組
5. [LATER]   學習 AXI4-Lite protocol，實作 AXI slave
6. [LATER]   接入 RISC-V core，跑第一個 bare-metal program
7. [FUTURE]  選擇 Phase 4 的方向深入
```

---

## 軟體 vs 硬體概念對照表

| 軟體概念 | 硬體對應 | 本 repo 中的範例 |
|---------|---------|-----------------|
| function call | module instantiation + port binding | 所有 test 中的 DUT 實例化 |
| if/else | MUX (多工器) | arbitrator 的 priority logic |
| switch/case | FSM state transition | uart_tx, simple_DMA |
| array | Register file / Memory | sync_fifo 的 `mem[]` |
| queue / ring buffer | FIFO | sync_fifo |
| pointer | Address bus | simple_DMA 的 src_addr, dst_addr |
| memcpy | DMA transfer | simple_DMA |
| mutex / semaphore | Arbitrator | arbitrator |
| interrupt handler (signal) | Hardware interrupt | apb_timer 的 irq, interrupt_controller |
| API / function signature | Bus protocol (APB, AXI) | apb_timer, simple_DMA 的 APB interface |
| unit test | Testbench | tests/unit/ 下的所有測試 |
| CI pipeline | Synthesis + Timing closure | (尚未涵蓋) |

---

## 推薦學習資源

### 書籍
- "Digital Design and Computer Architecture" (Harris & Harris) — 最佳入門書
- "Computer Organization and Design RISC-V Edition" (Patterson & Hennessy)
- "SystemVerilog for Verification" (Spear & Tumbush) — 如果走驗證方向

### 線上資源
- HDLBits (hdlbits.01xz.net) — 互動式 Verilog 練習
- Nand2Tetris (nand2tetris.org) — 從 NAND gate 蓋到整台電腦
- AMBA spec (ARM developer site) — 免費下載

### 開源工具
- Icarus Verilog — Verilog 模擬器
- Verilator — 高速 Verilog lint + simulator
- GTKWave — 波形檢視器
- Yosys — 開源綜合器
- OpenROAD — 開源 ASIC flow

---

*本文件根據 systemc_labs 的現有內容分析產出，可隨學習進度持續更新。*
