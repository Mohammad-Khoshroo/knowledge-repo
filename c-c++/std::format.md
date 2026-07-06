# C++ `std::format` Cheat Sheet (C++20 & C++23)

`std::format` is the modern C++ formatting facility introduced in **C++20**.  
It provides a safer, cleaner, and more extensible alternative to older approaches such as:

- `printf`
- `std::stringstream`
- manual string concatenation

Its syntax is similar to Python’s `str.format` / f-string formatting rules.

---

## Table of Contents

- [C++ `std::format` Cheat Sheet (C++20 \& C++23)](#c-stdformat-cheat-sheet-c20--c23)
  - [Table of Contents](#table-of-contents)
  - [1. Basic Usage (C++20)](#1-basic-usage-c20)
  - [2. Positional Arguments](#2-positional-arguments)
  - [3. Alignment, Width, and Fill](#3-alignment-width-and-fill)
  - [4. Integer and Floating-Point Formatting](#4-integer-and-floating-point-formatting)
  - [5. Advanced Number Features](#5-advanced-number-features)
  - [6. Dynamic Width and Precision](#6-dynamic-width-and-precision)
  - [7. Locale-Aware Formatting](#7-locale-aware-formatting)
  - [8. Escaping Braces](#8-escaping-braces)
  - [9. Formatting Containers / Ranges (C++23)](#9-formatting-containers--ranges-c23)
  - [10. Formatting Custom Types](#10-formatting-custom-types)
  - [11. Direct Output with `std::print` (C++23)](#11-direct-output-with-stdprint-c23)
  - [12. Common Format Specifiers Summary](#12-common-format-specifiers-summary)
  - [13. Notes and Gotchas](#13-notes-and-gotchas)
  - [Minimal Quick Reference](#minimal-quick-reference)
  - [Example: Putting It All Together](#example-putting-it-all-together)

---

## 1. Basic Usage (C++20)

Include the `<format>` header and use `std::format` to produce a formatted `std::string`.

```cpp
#include <format>
#include <string>
#include <iostream>

int main() {
    std::string s = std::format("Hello, {}! You have {} new messages.", "Alice", 5);
    std::cout << s << '\n';
}
```

### Output

```text
Hello, Alice! You have 5 new messages.
```

### Notes

- Each `{}` is a placeholder for one argument.
- Arguments are inserted in order.
- `std::format` returns a `std::string`.

---

## 2. Positional Arguments

You can explicitly choose which argument goes into which placeholder.

```cpp
#include <format>
#include <string>

int main() {
    std::string s = std::format("I prefer {1} over {0}.", "C++", "Python");
    // Output: I prefer Python over C++.
}
```

### Why this is useful

Positional arguments help when:

- reordering words in a sentence
- building localized strings
- reusing the same argument multiple times

```cpp
std::string s = std::format("{0} + {0} = {1}", 2, 4);
// Output: 2 + 2 = 4
```

> In practice, avoid mixing automatic `{}` indexing and manual `{0}`, `{1}` indexing in the same format string.

---

## 3. Alignment, Width, and Fill

General syntax:

```text
{:[fill][align][width]}
```

### Alignment options

- `<` : left align
- `>` : right align
- `^` : center align

### Examples

```cpp
#include <format>
#include <iostream>

int main() {
    std::cout << std::format("{:<10}", "C++") << '\n';   // "C++       "
    std::cout << std::format("{:>10}", "C++") << '\n';   // "       C++"
    std::cout << std::format("{:^10}", "C++") << '\n';   // "   C++    "
    std::cout << std::format("{:-^10}", "C++") << '\n';  // "---C++----"
    std::cout << std::format("{:*>10}", "C++") << '\n';  // "*******C++"
}
```

### Explanation

- `10` means the minimum field width is 10 characters.
- If the value is shorter, padding is added.
- By default, padding uses spaces.
- You can choose a custom fill character such as `-`, `*`, or `.`.

### More examples

```cpp
std::format("{:.<8}", "Hi");   // "Hi......"
std::format("{:^12}", "title"); // "   title    "
```

---

## 4. Integer and Floating-Point Formatting

### 4.1 Integer formatting

```cpp
#include <format>

std::format("{}", 42);     // "42"
std::format("{:d}", 42);   // "42" (decimal)
std::format("{:x}", 255);  // "ff" (hexadecimal)
std::format("{:X}", 255);  // "FF" (uppercase hexadecimal)
std::format("{:b}", 5);    // "101" (binary)
std::format("{:o}", 10);   // "12" (octal)
```

### 4.2 Zero padding

```cpp
std::format("{:05}", 42);  // "00042"
std::format("{:08}", 123); // "00000123"
```

### 4.3 Floating-point formatting

```cpp
std::format("{}", 3.14159);      // default representation
std::format("{:.2f}", 3.14159);  // "3.14"
std::format("{:.4f}", 3.14159);  // "3.1416"
std::format("{:.2e}", 3.14159);  // "3.14e+00"
std::format("{:.2E}", 3.14159);  // "3.14E+00"
std::format("{:.2g}", 3.14159);  // general format
```

### Common floating-point presentation types

- `f` : fixed-point notation
- `e` : scientific notation
- `E` : scientific notation with uppercase exponent
- `g` : general format
- `G` : general format uppercase
- `a` / `A` : hexadecimal floating-point notation

Example:

```cpp
std::format("{:a}", 3.5); // hexadecimal floating-point form
```

---

## 5. Advanced Number Features

### 5.1 Sign control

You can control how positive and negative signs appear.

```cpp
std::format("{:+}", 42);   // "+42"
std::format("{:+}", -42);  // "-42"
std::format("{:-}", 42);   // "42"
std::format("{: }", 42);   // " 42"
```

### Meaning

- `+` : always show sign
- `-` : show sign only for negatives (default behavior)
- space : prepend a space for positive values

---

### 5.2 Alternate form `#`

The `#` option enables alternate formatting.

```cpp
std::format("{:#x}", 255); // "0xff"
std::format("{:#X}", 255); // "0XFF"
std::format("{:#b}", 5);   // "0b101"
std::format("{:#o}", 10);  // "012"
```

For floating-point values, alternate form can preserve the decimal point in some cases.

---

### 5.3 Combining options

You can combine width, alignment, sign, and alternate form.

```cpp
std::format("{:#08x}", 255);  // "0x0000ff" or implementation-specific padding behavior
std::format("{:+10d}", 42);   // "       +42"
std::format("{:^#12x}", 255); // centered hex output with prefix
```

---

## 6. Dynamic Width and Precision

You can pass width and precision at runtime.

```cpp
#include <format>
#include <string>

int main() {
    int width = 10;
    int prec = 3;

    std::string s = std::format("{:{}.{}f}", 3.14159, width, prec);
    // Output: "     3.142"
}
```

### Another example

```cpp
int w = 8;
std::format("{:>{}}", 42, w); // right-aligned width 8
```

### Why it matters

Dynamic formatting is useful when:

- building tables
- user-configurable output
- reporting values with runtime precision

---

## 7. Locale-Aware Formatting

You can request locale-aware formatting using `L`.

```cpp
#include <format>

std::format("{:L}", 1000000); // e.g. "1,000,000" depending on locale
```

### Notes

- Output depends on the active locale and implementation support.
- Thousand separators and decimal separators may vary.
- Behavior is platform-dependent.

---

## 8. Escaping Braces

Since `{}` has special meaning, literal braces must be escaped by doubling them.

```cpp
#include <format>

std::format("{{}}");              // "{}"
std::format("Set = {{1, 2, 3}}"); // "Set = {1, 2, 3}"
```

This is important when you want to print JSON-like or code-like text.

---

## 9. Formatting Containers / Ranges (C++23)

C++23 adds support for formatting ranges and containers more directly.

```cpp
#include <vector>
#include <format>

int main() {
    std::vector<int> v = {1, 2, 3};
    std::string s = std::format("{}", v);
    // Output: [1, 2, 3]
}
```

### Other possible examples

```cpp
#include <list>
#include <set>
#include <format>

std::list<int> a = {4, 5, 6};
std::set<std::string> b = {"red", "green", "blue"};
```

Depending on library support, these can also be formatted as ranges.

### Important note

Although this is standardized in C++23, **compiler and standard library support may still vary**.  
Some environments may not yet fully implement all range-formatting features.

---

## 10. Formatting Custom Types

To format your own types, specialize `std::formatter`.

### Example: simple `Point` type

```cpp
#include <format>
#include <string>

struct Point {
    int x;
    int y;
};

template <>
struct std::formatter<Point> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const Point& p, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "({}, {})", p.x, p.y);
    }
};

int main() {
    Point p{10, 20};
    std::string s = std::format("Point is {}", p);
    // Output: "Point is (10, 20)"
}
```

### What `parse` does

`parse` reads any custom formatting options from the format string.

In this simple example, we ignore custom options and just return `ctx.begin()`.

### What `format` does

`format` writes the formatted representation into the output context.

### More advanced custom formatter idea

You could support formats like:

- `{}` → `(x, y)`
- `{:csv}` → `x,y`
- `{:braced}` → `[x, y]`

That requires parsing custom specifiers in `parse()`.

---

## 11. Direct Output with `std::print` (C++23)

C++23 introduces `std::print` and `std::println` in `<print>`.

These functions print directly to standard output without first creating a `std::string`.

```cpp
#include <print>

int main() {
    std::print("Hello, {}!\n", "World");
    std::println("Value: {:#x}", 255);
}
```

### Output

```text
Hello, World!
Value: 0xff
```

### Difference from `std::format`

- `std::format(...)` → returns a string
- `std::print(...)` → prints directly to output
- `std::println(...)` → prints and appends newline

---

## 12. Common Format Specifiers Summary

### Alignment and width

| Specifier | Meaning |
|----------|---------|
| `<` | Left align |
| `>` | Right align |
| `^` | Center align |
| `width` | Minimum field width |
| `fill` | Padding character |

Examples:

```cpp
std::format("{:<8}", "hi");
std::format("{:*>8}", "hi");
std::format("{:^10}", "hi");
```

---

### Sign and alternate form

| Specifier | Meaning |
|----------|---------|
| `+` | Always show sign |
| `-` | Show sign only for negative numbers |
| space | Leading space for positive numbers |
| `#` | Alternate form |

---

### Integer presentation types

| Type | Meaning |
|------|---------|
| `d` | Decimal |
| `x` | Hexadecimal lowercase |
| `X` | Hexadecimal uppercase |
| `b` | Binary |
| `B` | Binary uppercase style if supported by implementation |
| `o` | Octal |

---

### Floating-point presentation types

| Type | Meaning |
|------|---------|
| `f` | Fixed-point |
| `F` | Fixed-point uppercase |
| `e` | Scientific |
| `E` | Scientific uppercase |
| `g` | General |
| `G` | General uppercase |
| `a` | Hex float |
| `A` | Hex float uppercase |

---

## 13. Notes and Gotchas

### 1. Compiler/library support may vary

Even if a feature is part of C++20 or C++23, your compiler’s standard library may not fully support it yet.

For example:

- older GCC/libstdc++ versions had incomplete `std::format` support
- some C++23 features such as range formatting or `<print>` may not be available everywhere yet

---

### 2. `std::format` throws on invalid format strings

Invalid format strings can produce exceptions such as `std::format_error`.

```cpp
#include <format>
#include <iostream>

int main() {
    try {
        auto s = std::format("{:z}", 42); // invalid specifier
    } catch (const std::format_error& e) {
        std::cout << "Format error: " << e.what() << '\n';
    }
}
```

---

### 3. Literal braces must be escaped

Always use:

```cpp
"{{"
"}}"
```

if you need literal `{` or `}` in the output.

---

### 4. Custom types require `std::formatter`

Unlike built-in arithmetic types and some library types, user-defined types do not automatically work with `std::format`.

---

### 5. `std::print` is C++23, not C++20

If your project is limited to C++20, `<print>` may not exist.

---

## Minimal Quick Reference

```cpp
std::format("Hello, {}", name);
std::format("{1} then {0}", a, b);
std::format("{:<10}", text);
std::format("{:>10}", text);
std::format("{:^10}", text);
std::format("{:05}", 42);
std::format("{:.2f}", 3.14159);
std::format("{:#x}", 255);
std::format("{:+}", 42);
std::format("{:{}.{}f}", value, width, precision);
std::format("{{escaped braces}}");
```

---

## Example: Putting It All Together

```cpp
#include <format>
#include <iostream>
#include <vector>

int main() {
    std::cout << std::format("Name: {:<10} Score: {:>5}\n", "Alice", 93);
    std::cout << std::format("Hex: {:#x}, Bin: {:#b}, Oct: {:#o}\n", 255, 5, 10);
    std::cout << std::format("Pi: {:.3f}\n", 3.14159265);

    int width = 8;
    int prec = 2;
    std::cout << std::format("Dynamic: {:{}.{}f}\n", 12.34567, width, prec);
}
```

### Possible output

```text
Name: Alice      Score:    93
Hex: 0xff, Bin: 0b101, Oct: 012
Pi: 3.142
Dynamic:    12.35
```

---
