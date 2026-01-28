# Interrupt Controller Specification (v1.0)

## 1. Overview

The **Interrupt Controller (INTC)** is a peripheral responsible for aggregating multiple interrupt request (IRQ) lines from various hardware modules (e.g., Timer, DMA, UART) into a single interrupt line for the CPU.

It provides:

* **Masking capability**: The CPU can selectively enable or disable specific interrupt sources.
* **Prioritization**: It resolves simultaneous interrupts based on a fixed priority scheme.
* **Status Reporting**: It allows the CPU to query which peripheral caused the interrupt.

## 2. Block Diagram

The INTC logic flow is:

1. **Raw Interrupts** (`irq_sources`) come in from peripherals.
2. **Mask Logic**: `Pending = Raw & Enable_Mask`.
3. **Priority Encoder**: Finds the highest priority bit in `Pending`.
4. **Output Generation**: Asserts `irq_out` if any pending bit is high, and outputs the `irq_id`.

## 3. Interface Description

### 3.1 Global Signals

| Signal Name | Direction | Width | Description |
| --- | --- | --- | --- |
| `clk` | Input | 1 | System Clock |
| `rst_n` | Input | 1 | Active-low asynchronous reset |

### 3.2 Interrupt Interfaces

| Signal Name | Direction | Width | Description |
| --- | --- | --- | --- |
| `irq_sources` | Input | 32 | Vector of interrupt lines from peripherals. (Bit 0 = High Priority) |
| `irq_out` | Output | 1 | Master Interrupt Request to CPU. (Level Triggered) |
| `irq_id` | Output | 5 | ID of the highest priority pending interrupt (0-31). |

### 3.3 APB Slave Interface (Control Plane)

Used by the CPU to configure the INTC.

| Signal Name | Direction | Width | Description |
| --- | --- | --- | --- |
| `psel` | Input | 1 | APB Select signal |
| `penable` | Input | 1 | APB Enable signal |
| `paddr` | Input | 32 | Register Address |
| `pwrite` | Input | 1 | Write Control (1=Write, 0=Read) |
| `pwdata` | Input | 32 | Write Data |
| `prdata` | Output | 32 | Read Data |

---

## 4. Register Map

Base Address: Defined by System Interconnect (e.g., `0x4000_1000`)

| Offset | Name | Access | Width | Description |
| --- | --- | --- | --- | --- |
| `0x00` | **IER** | RW | 32 | **Interrupt Enable Register**. 1 = Enable, 0 = Mask (Disable). |
| `0x04` | **IPR** | RO | 32 | **Interrupt Pending Register**. Shows effective interrupts (`Raw & Enable`). |
| `0x08` | **ISR** | RO | 32 | **Interrupt Raw Status Register**. Shows raw input status. |

### 4.1 IER (Interrupt Enable Register) - Offset 0x00

* **Reset Value**: `0x00000000` (All interrupts disabled by default)
* **Behavior**: CPU writes a '1' to a bit position to allow that interrupt to pass through.

### 4.2 IPR (Interrupt Pending Register) - Offset 0x04

* **Reset Value**: `0x00000000`
* **Behavior**: `IPR[i] = irq_sources[i] & IER[i]`
* This register tells the CPU **"Who is effectively calling me right now?"**.

### 4.3 ISR (Raw Status Register) - Offset 0x08

* **Reset Value**: `0x00000000`
* **Behavior**: `ISR[i] = irq_sources[i]`
* Used for debugging to see if a peripheral is asserting IRQ even if it's masked.

---

## 5. Functional Description

### 5.1 Interrupt Masking

The controller filters input interrupts using the `IER`.

* If `IER[n] == 0`, the corresponding `irq_sources[n]` is ignored (Masked).
* If `IER[n] == 1`, the `irq_sources[n]` signal propagates to the Pending logic.

### 5.2 Priority Logic (Fixed Priority)

When multiple interrupts are pending (multiple bits in `IPR` are 1), the controller must decide which ID to output.

* **Scheme**: **LSB First** (Least Significant Bit has highest priority).
* Bit 0 > Bit 1 > ... > Bit 31.


* **Example**:
* `IPR` = `0x0000_0005` (Binary `...0101`) -> Both Bit 0 and Bit 2 are pending.
* **Result**: `irq_id` = `0` (because Bit 0 > Bit 2).



### 5.3 Output Generation

* **`irq_out`**:
* Logic: `irq_out = |IPR` (Bitwise OR of all Pending bits).
* If `IPR` is non-zero, `irq_out` is High.


* **`irq_id`**:
* Outputs the index of the highest priority bit set in `IPR`.
* If `IPR` is 0, `irq_id` holds a default value (e.g., 0 or 31, usually irrelevant because `irq_out` is low).



### 5.4 Example Operation Flow

1. **Initial**: `IER`=0, `irq_sources`=0. `irq_out`=0.
2. **Setup**: CPU writes `0x03` to `IER` (Enable Bit 0 and Bit 1).
3. **Event A**: **Timer** (connected to Bit 1) fires. `irq_sources[1]`=1.
* `IPR` becomes `0x02`.
* `irq_out` goes High.
* `irq_id` becomes `1`.


4. **CPU Action**: CPU reads `irq_id` (sees 1), jumps to Timer ISR.
5. **Event B**: While handling Timer, **DMA** (connected to Bit 0) fires. `irq_sources[0]`=1.
* `IPR` becomes `0x03` (Both 0 and 1 are high).
* **Priority Logic**: Bit 0 > Bit 1.
* `irq_id` immediately changes to `0`.


6. **CPU Action**: CPU sees `irq_id` is now 0 (higher priority), it may choose to preempt the current ISR or handle it next.

---