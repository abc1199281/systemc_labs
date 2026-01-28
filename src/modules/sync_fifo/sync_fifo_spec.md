# Synchronous FIFO Specification (v1.0)

## 1. Overview

The **Sync_FIFO** is a parameterized data buffering module designed for single-clock domain applications. It serves as a temporary storage between a data producer (e.g., DMA Read Engine) and a data consumer (e.g., DMA Write Engine) to handle data flow discrepancies and decouple the read/write operations.

**Key Features:**

* **Synchronous Operation:** Single clock source for both read and write.
* **Circular Buffer:** Implemented using a memory array with read/write pointers.
* **Protection:** Automatic overflow (write when full) and underflow (read when empty) protection.
* **Look-ahead Output:** The data output port always shows the data at the head of the queue (if not empty).

## 2. Parameters

The module should be designed to be configurable (e.g., via C++ template arguments or constructor parameters).

| Parameter Name | Default Value | Description |
| --- | --- | --- |
| `DATA_WIDTH` | 32 | Width of the data bus (in bits). |
| `FIFO_DEPTH` | 16 | Depth of the FIFO (number of words). Must be a power of 2. |

## 3. Interface Description

### 3.1 Global Signals

| Signal Name | Direction | Type | Description |
| --- | --- | --- | --- |
| `clk` | Input | `bool` | System Clock. All operations occur on the rising edge. |
| `rst_n` | Input | `bool` | Active-low asynchronous reset. |

### 3.2 Write Interface (Connected to Producer)

| Signal Name | Direction | Type | Description |
| --- | --- | --- | --- |
| `wr_en` | Input | `bool` | **Write Enable**. If High, data is written on the next rising edge. |
| `din` | Input | `uint32` | **Data Input**. Data to be pushed into the FIFO. |
| `full` | Output | `bool` | **Full Flag**. High when FIFO cannot accept new data. |

### 3.3 Read Interface (Connected to Consumer)

| Signal Name | Direction | Type | Description |
| --- | --- | --- | --- |
| `rd_en` | Input | `bool` | **Read Enable** (Pop). If High, the current data is removed (pointer increments) on the next rising edge. |
| `dout` | Output | `uint32` | **Data Output**. Shows the data at the current Read Pointer. |
| `empty` | Output | `bool` | **Empty Flag**. High when FIFO has no valid data. |

---

## 4. Functional Description

### 4.1 Reset Behavior

Upon assertion of `rst_n` (low):

* Write Pointer (`w_ptr`) and Read Pointer (`r_ptr`) are reset to `0`.
* `empty` flag is set to `1`.
* `full` flag is set to `0`.
* `dout` is undefined (or 0).

### 4.2 Write Operation

* Data is written to the memory array at the address pointed to by `w_ptr` when:
1. `clk` rising edge occurs.
2. `wr_en` is **High**.
3. `full` is **Low**.


* After a successful write, `w_ptr` increments by 1.
* **Protection:** If `wr_en` is High but `full` is High, the write request is ignored (no overwrite).

### 4.3 Read Operation

* The `dout` port always exposes the data stored at `r_ptr` (Look-ahead behavior).
* The read action (Popping data) occurs when:
1. `clk` rising edge occurs.
2. `rd_en` is **High**.
3. `empty` is **Low**.


* After a successful read, `r_ptr` increments by 1, and `dout` updates to the next data immediately (combinational) or on next clock (sequential), depending on implementation preference.
* **Protection:** If `rd_en` is High but `empty` is High, the read pointer does not move.

### 4.4 Flag Logic

* **Empty Generation:**
* `empty = 1` when `w_ptr == r_ptr`.


* **Full Generation:**
* `full = 1` when the FIFO is filled.
* Implementation Note: To distinguish "Full" from "Empty" (since `w_ptr == r_ptr` in both cases), use an **extra bit** (N+1 bits) for the pointers.
* If `MSB` is different but lower bits are same: **Full**.
* If `MSB` is same and lower bits are same: **Empty**.



---

## 5. Timing Diagram Concept

```text
Clock:      _/~~\_/~~\_/~~\_/~~\_/~~\_/~~\_
            
Reset:      \_________/~~~~~~~~~~~~~~~~~~~~
            
wr_en:      _______/~~~~~\___________/~~~~~
din:        -------< D1  >-----------< D2 >
full:       _______________________________

(FIFO Internal: Writes D1, then D2)

rd_en:      ___________________/~~~~~\_____
dout:       XXXXXXX< D1  >-----< D2  >-----
empty:      ~~~~~~~\___________/~~~~~\_____
             (1)      (2)        (3)

```

**Notes on Timing:**

1. **Reset**: Empty is High.
2. **Write D1**: Empty goes Low immediately (or next cycle). `dout` shows D1.
3. **Read D1**: `rd_en` pulses. On next edge, D1 is removed, `dout` shows D2 (or invalid if empty).
