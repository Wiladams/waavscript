# QR-like PostScript control-flow diagnostic probes

Ten standalone non-rendering `.ps` files. Designed to mimic *algorithmic execution shapes* found in QR encoding, not a verbatim extraction of your particular BWIPP version. Load each separately in a clean VM. Each produces CHECK/PASS/FAIL and a final RESULT. All explicit `loop` calls have an iteration watchdog, but a broken execution scheduler can still hang before watchdog logic advances. Use a VM instruction budget or external timeout when testing.

01: shifting and bit termination. 02: bit extraction, nested repeat, cursor advancement. 03: GF(256) arithmetic including conditional XOR and dynamic termination. 04: in-place array mutation and loop termination. 05: nested matrix traversal and branch control. 06: `exit` within invoked procedure in `forall`. 07: dict `forall` containing array `forall`. 08: recursive lookahead, return order (not tail optimized). 09: stack operations interleaved with conditionals and `for`. 10: dynamic name lookup and dictionary scope.

**Debugging advice:** when one hangs, log the VM instruction count and top execution frames, compare repeated snapshots of `(frame kind, procedure identity, instruction index, loop count/cursor, operand stack depth)`. Infinite loops can be caused by a frame resuming at the wrong instruction or a mutation not completing before a test. These files should terminate normally under a correct PostScript Level 2 interpreter.

## Corrections
- Replaced nonstandard `max` in test 04 with standard PostScript `ifelse` and safe indexing.
- Corrected reversed watchdog arguments in tests 01, 02 and 04.
- Verified every standalone test under Ghostscript, with `RESULT PASS` required.
- These are algorithmic QR-style probes, not verbatim BWIPP extracts.
- A broken scheduler can still prevent the watchdog from running; use a VM instruction budget when needed.
