WaavScript PostScript scanner regression suite
===========================================
Run the files 01-07 and 13 separately in your engine, and compare with Ghostscript.
Every self-checking file prints PASS/FAIL and a TOTAL/FAILED count. Zero FAILED is expected.
Files 08-12 intentionally contain malformed PostScript and MUST report syntaxerror; they are not part of a standard combined run.
Example with Ghostscript installed:
  gs -q -dNODISPLAY -dBATCH -dNOPAUSE 01_numeric_values.ps

Notes:
- Tests emphasize scanner numeric classification, grayscale operands, immediate names, string escapes, delimiter semantics, and error vs EOF.
- Some tests exercise VM operations (token, eq, type, undef, currentgray, bind); failures can therefore come from those operators rather than lexical scanning.
- Verify the Ghostscript reference output first and adapt any interpreter-specific operator support separately.
- Run as individual files: some tests deliberately shadow dictionary entries or exercise error handling.
