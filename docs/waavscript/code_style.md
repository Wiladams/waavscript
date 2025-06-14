# WAAVS C++ Coding Guidelines

These guidelines define the coding standards for all WAAVS-related projects, with a focus on clarity, performance, correctness, and consistency. They apply across system-level codebases including interpreters, graphics engines, and performance-sensitive tooling.

---

## Language Version

- **Preferred version**: C++11
- **Permitted**: C++11 to C++17
- **Disallowed in production**: C++20 and beyond (permitted only in examples or tests)

---

## General Principles

- Code must be **directly compilable** — no placeholders, no fictional constructs.
- All **code and comments must be ASCII-only**. No Unicode characters of any kind.
- Favor **minimalism**, **clarity**, and **low-overhead** design.
- Optimize for **zero heap allocations** and prefer **copy-on-write reuse** where possible.
- Avoid unnecessary use of **exceptions** and **dynamic polymorphism**.
- Minimize reliance on **complex STL features**. Use only what is needed.
- Design for **deterministic behavior** and **performance predictability**.

---

## API & Enum Usage

- All **API calls must exactly match real definitions**. No guessing or inventing names.
- All **enum values must be verified against actual API headers**.
  - Example: Use `BL_FILL_RULE_NON_ZERO`, not `kNonZero`.

---

## PostScript Integration Rules (for Interpreter Work)

- Match **PostScript semantics** precisely (e.g. winding rules, coordinate systems).
- Do not insert behavior not defined by the PostScript specification.
- Use **copy-on-write** for interpreter state where applicable.
- Enforce correct stack operations and dictionary behavior.

---

## Design Style

- Use **flat, modular design** where possible.
- Avoid deeply nested inheritance.
- Keep function scope **minimal and explicit**.
- Favor **stack-based structures** and **explicit lifecycle control**.
- Encapsulate reusable logic into headers with inline or static methods when appropriate.

---

## Code Formatting and Layout

- Keep line width sensible (80–100 columns recommended).
- Use consistent indentation (e.g. 4 spaces, no tabs).
- Group related functions and structures clearly.
- Leave whitespace around logical blocks to improve readability.

---

## Examples and Tests

- You may use C++20 features (e.g. `[[nodiscard]]`, structured bindings) in **tests and examples only**.
- Do **not** rely on these features in production or library code.

---

## ASCII-Only Rule

To ensure maximum compatibility with all environments and editors:

- Do not use Unicode arrows (`→`, `←`, etc.), smart quotes, em dashes, or special symbols.
- Use plain ASCII equivalents for all output, comments, and string literals.

---

## Naming Conventions

- Functions: `camelCase()`
- Types: `PascalCase`
- Constants/macros: `ALL_CAPS_WITH_UNDERSCORES`
- Avoid underscores in regular variable and function names unless necessary for clarity or consistency with external APIs.

---

## File Organization

- Prefer header-only utilities where feasible.
- Use `.h` for headers and `.cpp` for implementation files (no `.cc` or `.cxx`).
- Keep file names lowercase and descriptive: `psvm.h`, `blend_adapter.cpp`, etc.

---

## Testing Philosophy

- Each component should be testable in isolation.
- Unit tests should validate:
  - Semantics (e.g. correct interpreter output)
  - Edge cases and failure modes
  - API conformance

---

## Summary

The WAAVS C++ guidelines prioritize:

- Strict accuracy with external APIs and specifications
- Clean, maintainable, portable C++11/14/17 code
- Explicit, minimal, and performance-aware design
- ASCII-only, verifiable code that compiles cleanly and runs deterministically

---

**Version:** 1.0  
**Last Updated:** 2025-06-02  
