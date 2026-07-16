# SystemC Object Naming: `name()` vs. `basename()`

In SystemC, understanding how objects (modules, ports, signals, etc.) are named and structured hierarchically is crucial for debugging, tracing (VCD), and logging. This document explains the key differences between `name()` and `basename()`, along with other related naming methods and concepts.

---

## Naming Methods Overview

Every major SystemC class (like `sc_module`, `sc_port`, `sc_signal`, etc.) inherits from the base class `sc_object`. This base class provides several member functions to query the object's hierarchical or local name.

| Method | Return Type | Description | Example Output |
| :--- | :--- | :--- | :--- |
| **`name()`** | `const char*` | Returns the **fully qualified hierarchical path** from the top-level testbench down to this object. | `"tb.dut.ExclusiveBUS"` |
| **`basename()`** | `const char*` | Returns **only the local name** of the object, stripping away all parent hierarchical paths. | `"ExclusiveBUS"` |
| **`kind()`** | `const char*` | Returns a string representing the **class type/kind** of the SystemC object. | `"sc_module"`, `"sc_signal"` |

---

## Detailed Comparison

### 1. `sc_object::name()`
The `name()` method returns the absolute path of the object in the SystemC design hierarchy. The levels of hierarchy are delimited by a dot (`.`).

* **When to use:** Use this when you need a globally unique identifier for an object, such as when registering signals globally in a trace file or printing unique error logs.
* **Gotcha:** If you pass `this->name()` into custom tracing wrappers that append their own prefixes, you will end up with highly redundant, nested trees in your VCD waveform viewer (e.g., `tb.dut.tb.dut.ExclusiveBUS`).
```cpp
// Inside sc_module "ExclusiveBUS" nested under "tb.dut"
std::cout << this->name(); 
// Output: "tb.dut.ExclusiveBUS"

### 2. `sc_object::basename()`
The `basename()` method extracts only the leaf-level name specified during the object's construction. It strips out all parent names and the dot (`.`) separators.

* **When to use:** Use this when you only care about the local module's identity, or when grouping signals under a clean, non-redundant prefix in wave viewers.
* **Safety:** Unlike parsing `name()` manually using `std::string::find_last_of('.')`, `basename()` is a highly optimized, built-in SystemC API.

cpp
// Inside sc_module "ExclusiveBUS" nested under "tb.dut"
std::cout << this->basename(); 
// Output: "ExclusiveBUS"

---

## Other Related SystemC Naming Concepts

### `sc_core::sc_module_name`
This class is a special parameter type used exclusively in `sc_module` constructors. 
* It registers the module in the SystemC kernel's structural hierarchy during elaboration.
* It implicitly converts to `const char*` (which returns the local name passed to it), but does **not** automatically convert to `std::string` without explicit casting.

cpp
SC_MODULE(MyModule) {
MyModule(sc_module_name name) : sc_module(name) {
// 'name' here represents the local string passed by the parent during instantiation
}
};

### `sc_object::kind()`
This method tells you what kind of SystemC object you are dealing with at runtime.
cpp
sc_signal<bool> clk("clk");
std::cout << clk.kind(); 
// Output: "sc_signal"

### Hierarchical Path Delimiter (`.`)
SystemC strictly uses the dot (`.`) symbol to separate hierarchical levels. When creating custom trace wrappers, avoid manually stitching names with extra dots if you are already using `name()`, as it will introduce empty hierarchy levels in VCD files.

---

## Best Practices for VCD Tracing
* **Use `basename()`** if your trace manager manually prefixes the path with parent names to avoid duplicated trees (e.g., `tb.dut.tb.dut...`).
* **Use `name()`** if you are calling the raw `sc_trace()` function directly, because `sc_trace` relies on fully qualified absolute names to draw the correct tree in GTKWave/ModelSim.
