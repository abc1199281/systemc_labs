# DMA Controller Specification (v1.0)

## 1. Overview

The **Direct Memory Access (DMA)** controller is a hardware block designed to offload memory copy tasks from the CPU. It supports memory-to-memory data transfer with a programmable source address, destination address, and transfer length.

Key Features:

* **APB (Advanced Peripheral Bus)** Slave interface for configuration (CSR).
* **Custom Ready/Valid** Master interface for high-speed data transfer.
* Supports programmable transfer length.
* Interrupt generation upon transfer completion.
* Decoupled architecture (Control Logic + Data Engine).

## 2. Block Diagram

The design consists of two main sub-modules:

1. **APB Slave:** Handles register reads/writes from the CPU.
2. **DMA Engine:** Manages the Finite State Machine (FSM) for data movement.

## 3. Interface Description

### 3.1 Global Signals

| Signal Name | Direction | Width | Description |
| --- | --- | --- | --- |
| `clk` | Input | 1 | System Clock |
| `rst_n` | Input | 1 | Active-low asynchronous reset |

### 3.2 APB Slave Interface (Control Plane)

Used by the CPU to configure the DMA.

| Signal Name | Direction | Width | Description |
| --- | --- | --- | --- |
| `psel` | Input | 1 | APB Select signal |
| `penable` | Input | 1 | APB Enable signal |
| `paddr` | Input | 32 | Register Address |
| `pwrite` | Input | 1 | Write Control (1=Write, 0=Read) |
| `pwdata` | Input | 32 | Write Data |
| `prdata` | Output | 32 | Read Data |
| `pready` | Output | 1 | Slave Ready (Always 1 in this simplified version) |

### 3.3 DMA Master Interface (Data Plane)

Used to access Memory. Implements a **Ready/Valid** handshake protocol.

| Signal Name | Direction | Width | Description |
| --- | --- | --- | --- |
| **Request Channel** |  |  |  |
| `m_req_valid` | Output | 1 | Indicates valid request (Read or Write) |
| `m_req_ready` | Input | 1 | Memory is ready to accept request |
| `m_req_addr` | Output | 32 | Target Memory Address |
| `m_req_op` | Output | 1 | Operation Type (0=Read, 1=Write) |
| **Data Channel** |  |  |  |
| `m_data_out` | Output | 32 | Data to be written to memory |
| `m_data_in` | Input | 32 | Data read from memory |
| `m_data_valid` | Input | 1 | Indicates `m_data_in` is valid (for Read) |

### 3.4 Interrupt

| Signal Name | Direction | Width | Description |
| --- | --- | --- | --- |
| `irq` | Output | 1 | High level indicates transfer complete |

---

## 4. Register Map

Base Address: Defined by System Interconnect (e.g., `0x4000_0000`)

| Offset | Name | Access | Width | Description |
| --- | --- | --- | --- | --- |
| `0x00` | **CSR** | RW | 32 | Control & Status Register |
| `0x04` | **SRC_ADDR** | RW | 32 | Source Memory Address |
| `0x08` | **DST_ADDR** | RW | 32 | Destination Memory Address |
| `0x0C` | **LENGTH** | RW | 32 | Transfer Length (in Words) |

### 4.1 CSR (Control & Status Register) - Offset 0x00

| Bit | Name | R/W | Description |
| --- | --- | --- | --- |
| 0 | `START` | W/O | Write '1' to start the DMA transfer. Self-clearing. |
| 1 | `BUSY` | R | 1 = DMA is actively transferring data. |
| 2 | `DONE` | R/W1C | 1 = Transfer complete. Write '1' to clear (W1C). |
| 31:3 | `RSVD` | - | Reserved. |

---

## 5. Functional Description

### 5.1 Operation Flow

1. **Configuration:**
* CPU writes `SRC_ADDR`, `DST_ADDR`, and `LENGTH` via the APB interface.
* CPU writes `1` to `CSR[0]` (START bit).


2. **Execution (DMA Engine FSM):**
* DMA enters `BUSY` state (`CSR[1]=1`).
* **Step A (Read):** DMA asserts `m_req_valid` with `m_req_op=0` (Read) and `SRC` address. Waits for `m_req_ready`.
* **Step B (Wait Data):** DMA waits for `m_data_valid` from memory. Latches data into internal buffer.
* **Step C (Write):** DMA asserts `m_req_valid` with `m_req_op=1` (Write), `DST` address, and `m_data_out`. Waits for `m_req_ready`.
* Repeat Step A-C until `LENGTH` words are transferred.


3. **Completion:**
* DMA clears `BUSY` bit.
* DMA sets `DONE` bit (`CSR[2]=1`) and asserts `irq`.
* CPU handles interrupt and writes `1` to `CSR[2]` to clear the status.



### 5.2 Handshake Protocol (Ready/Valid)

* **Source (DMA)** asserts `Valid` when data/address is stable.
* **Destination (Memory)** asserts `Ready` when it can accept.
* Transfer occurs **only** when `Valid=1` AND `Ready=1` at the rising edge of `clk`.