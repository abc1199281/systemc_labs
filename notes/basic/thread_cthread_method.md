# SystemC Process Types: SC_METHOD vs SC_THREAD vs SC_CTHREAD

In SystemC, processes are the fundamental units of concurrent execution. There are three main types of processes used to model hardware behavior.

## 1. SC_METHOD

`SC_METHOD` is the most basic process type, behaving similar to a combinational logic block or a simple sequential block in HDL (like an `always @(*)` block in Verilog).

*   **Execution**: It runs from beginning to end and returns immediately.
*   **Blocking**: It **cannot** use `wait()` statements. Calling `wait()` inside an `SC_METHOD` will causing a runtime error.
*   **State**: It does not maintain an execution state between calls (no implicit stack persistence across invocations). Local variables are lost when the function returns.
*   **Sensitivity**: It is sensitive to a list of signals/events. Whenever one of these events occurs, the method is executed.
*   **Use Case**: Ideal for combinational logic, simple sequential logic (like counters), and whenever you don't need to suspend execution.

```cpp
SC_MODULE(MyModule) {
    sc_in<bool> a, b;
    sc_out<bool> z;

    void do_logic() {
        z.write(a.read() && b.read());
    }

    SC_CTOR(MyModule) {
        SC_METHOD(do_logic);
        sensitive << a << b; // Trigger when 'a' or 'b' changes
    }
};
```

## 2. SC_THREAD

`SC_THREAD` is a general-purpose thread process.

*   **Execution**: It starts once at the beginning of the simulation (initialization phase). To keep it running, you typically place the code inside an infinite loop (`while(true)`).
*   **Blocking**: It **can** use `wait()` statements to suspend execution and yield control back to the simulation kernel.
*   **State**: It maintains its state (stack) across `wait()` calls. Local variables persist.
*   **Sensitivity**: It has a static sensitivity list, but can also use dynamic sensitivity via `wait(event)` or `wait(timeout)`.
*   **Performance**: Context switching is slower than `SC_METHOD` because it involves saving/restoring the thread stack.
*   **Use Case**: Testbenches, high-level behavioral modeling, and sequential logic requiring complex timing or state machines.

```cpp
SC_MODULE(MyModule) {
    sc_in_clk clk;
    
    void do_process() {
        while (true) {
            wait(); // Wait for sensitivity list (clk.pos())
            // Perform actions
        }
    }

    SC_CTOR(MyModule) {
        SC_THREAD(do_process);
        sensitive << clk.pos();
    }
};
```

## 3. SC_CTHREAD (Clocked Thread)

`SC_CTHREAD` is a special case of `SC_THREAD` optimized for synthesis (specifically High-Level Synthesis).

*   **Execution**: Similar to `SC_THREAD`, it runs in an infinite loop.
*   **Sensitivity**: It is static and strictly sensitive to **one specific clock edge** (positive or negative). You cannot change its sensitivity dynamically.
*   **Blocking**: Can use `wait()`.
*   **Legacy**: Historically used for synthesis flows. Modern HLS and SystemC usage often prefer `SC_METHOD` or `SC_THREAD` depending on the tool, but `SC_CTHREAD` explicitly enforces clocked behavior.
*   **Use Case**: Modeling synchronous logic for synthesis where reset behavior and clock edge are strictly defined.

```cpp
SC_MODULE(MyModule) {
    sc_in_clk clk;
    
    void do_clocked() {
        // Optional: Reset logic here
        while (true) {
            wait(); // Wait for next active clock edge
            // Synchronous logic
        }
    }

    SC_CTOR(MyModule) {
        SC_CTHREAD(do_clocked, clk.pos()); // Explicitly sensitive to clock edge
    }
};
```

## Comparison Table

| Feature | SC_METHOD | SC_THREAD | SC_CTHREAD |
| :--- | :--- | :--- | :--- |
| **Blocking** | No `wait()`, runs to completion | Can use `wait()` | Can use `wait()` |
| **Execution** | Triggered by event, runs once | Infinite loop (usually) | Infinite loop |
| **State** | No persistent execution state | Preserves stack/variables | Preserves stack/variables |
| **Sensitivity** | Static & Dynamic (next_trigger) | Static & Dynamic (wait) | Static (Clock Edge only) |
| **Performance** | Fast (function call) | Slower (context switch) | Slower (context switch) |
| **Primary Use** | Combinational logic, HW blocks | Testbenches, Behavioral Models | Synthesizable Synchronous Logic |

## Summary

*   Use **SC_METHOD** for most hardware logic (combinational or simple sequential) due to simulation speed.
*   Use **SC_THREAD** for writing testbenches where you need to sequence stimuli over time (e.g., "drive A, wait 10ns, drive B").
*   Use **SC_CTHREAD** only if you are specifically targeting a synthesis flow that requires it, or to explicitly model clocked behavior in a rigid way.
