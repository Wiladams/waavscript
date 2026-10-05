# Mathematical Illustrations - runnable PostScript examples

This archive collects standalone examples transcribed from the supplied PDF, **Mathematical Illustrations: A Manual of Geometry and PostScript**, plus the supplied `.inc` library files.

The examples are organized by chapter/appendix. I included examples that form sensible standalone programs and omitted fragments that are only syntax demonstrations, deliberately incomplete exercises (`...`), console error transcripts, and pseudocode.

## Running

From the root of this directory, for example:

    gs ch01/05_red_square_black_outline.ps

To render to PDF with Ghostscript:

    gs -dBATCH -dNOPAUSE -sDEVICE=pdfwrite -sOutputFile=out.pdf ch06/02_bezier_basic.ps

Examples that use a supplied library expect to be run with the archive root as the current directory, so `(lines.inc) run`, `(zoom.inc) run`, etc. resolve correctly.

## 3D note

The tutorial explicitly calls for a library named `ps3d.inc`. The uploaded `ps3d.ps` is a typeset PostScript document/manual, not that include library. Therefore `ch13/01_ps3d_square_requires_ps3d_inc.ps` is included as a faithful dependency example but is not runnable until the actual `ps3d.inc` is supplied. The supplied `polyhedra.inc` also expects vector/3D routines such as `vector-sub`, `cross-product`, `normalized`, and `dot-product`, which normally come from that 3D support library.

The supplied `hull.inc` references `sort.inc`, and `hs.inc` references `../../ps/stack.inc` and `../../ps/sort.inc`; those referenced files/paths were not all supplied, so they are preserved unchanged but are not used by the validated standalone examples.

## Validation

All examples except the explicitly dependency-limited 3D example were syntax/execution checked with Ghostscript's `nullpage` device.
