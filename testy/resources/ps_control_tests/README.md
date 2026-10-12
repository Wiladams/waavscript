# PostScript execution/continuation tests

Run each `.ps` file **in a fresh VM**, preferably using the same interpreter entry point as barcode.ps. Each is independent and uses `print`, `==`, `eq`, `def`, `if`, `ifelse`, and the operator under test. Each prints `=== PASS` or `=== FAIL` and compares observed values against expected outcomes. The harness checks the operand-stack depth before finishing.

Start with 01, 03, 04, 05, 07, 08, then 12. The rest cover stack effects, string/dictionary forall, mixed frames, and procedure scheduling. `13_forall_dict.ps` assumes `forall` on dictionaries is implemented; omit it if that implementation is still pending.

**Diagnostics:** iteration traces, branch/exit traces, values, counts, and checks. Test 12 emits progress every 10,000 iterations. Native call-stack depth, execution-stack frame depth, and wall time cannot be measured portably in standard PostScript: add those metrics to your VM instrumentation if required. On a hang, the most recent trace line identifies the phase and approximate iteration reached. A continuation bug could also prevent the test from ever reaching its summary.

These tests deliberately avoid any graphics, font resources, `showpage`, and interpreter-specific extensions. All files print their own names, summaries, and results. They test standard PostScript behavior and are designed to be useful for the schedule-only single-dispatch VM.

Potential caveat: `exit` inside a procedure called directly or indirectly by a looping body has subtleties involving `invalidexit`; that separate case is not included here. The current suite tests `exit` lexically in the loop body or inside an `if` nested within it, which is valid PostScript.
