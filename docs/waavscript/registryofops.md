# PostScript Operator Registry

This registry lists all known PostScript operators (Level 1 and common Level 2/3), grouped by category. Each operator has a checkbox indicating implementation status.

Legend:
- ✅ Implemented
- ⬜ Not Implemented

---

## 🔹 Core Stack Operators

| Operator      | Description                           | Status |
| ------------- | ------------------------------------- | ------ |
| `pop`         | Remove top element from operand stack | ✅     |
| `dup`         | Duplicate top element                 | ✅     |
| `exch`        | Exchange top two elements             | ✅     |
| `copy`        | Duplicate multiple elements           | ✅     |
| `index`       | Copy nth element to top               | ✅     |
| `roll`        | Rotate top n elements                 | ✅     |
| `clear`       | Clear operand stack                   | ✅     |
| `count`       | Push number of elements on stack      | ✅     |
| `mark`        | Push mark object                      | ✅     |
| `cleartomark` | Pop to last mark                      | ✅     |
| `counttomark` | Count to last mark                    | ✅     |

---

## 🔹 Relational Operators

| Operator | Description         | Status |
| -------- | ------------------- | ------ |
| `eq`     | Test for equality   | ✅     |
| `ne`     | Test for inequality | ✅     |
| `gt`     | Greater than        | ✅     |
| `lt`     | Less than           | ✅     |
| `ge`     | Greater or equal    | ✅     |
| `le`     | Less or equal       | ✅     |

---

## 🔹 Logical Operators

| Operator | Description            | Status |
| -------- | ---------------------- | ------ |
| `and`    | Bitwise/logical AND    | ✅     |
| `or`     | Bitwise/logical OR     | ✅     |
| `xor`    | Bitwise/logical XOR    | ✅     |
| `not`    | Bitwise/logical NOT    | ✅     |

---

## 🔹 Arithmetic Operators

| Operator   | Description             | Status |
| ---------- | ----------------------- | ------ |
| `add`      | Addition                | ✅     |
| `sub`      | Subtraction             | ✅     |
| `mul`      | Multiplication          | ✅     |
| `div`      | Real division           | ✅     |
| `idiv`     | Integer division        | ✅     |
| `mod`      | Modulus                 | ✅     |
| `neg`      | Negate                  | ✅     |
| `abs`      | Absolute value          | ✅     |
| `ceiling`  | Round up                | ✅     |
| `floor`    | Round down              | ✅     |
| `round`    | Round to nearest        | ✅     |
| `truncate` | Discard fractional part | ✅     |
| `sqrt`     | Square root             | ✅     |
| `max`      | Maximum                 | ✅     |
| `min`      | Minimum                 | ✅     |

---

## 🔹 Exponential & Logarithmic

| Operator | Description       | Status |
| -------- | ----------------- | ------ |
| `exp`    | Exponentiation    | ✅     |
| `ln`     | Natural logarithm | ✅     |
| `log`    | Base-10 logarithm | ✅     |

---

## 🔹 Trigonometric Operators

| Operator | Description        | Status |
| -------- | ------------------ | ------ |
| `sin`    | Sine (degrees)     | ✅     |
| `cos`    | Cosine (degrees)   | ✅     |
| `atan`   | Arctangent (dy dx) | ✅     |

---

## 🔹 Random Number Operators

| Operator | Description   | Status |
| -------- | ------------- | ------ |
| `rand`   | Random number | ✅     |
| `srand`  | Set seed      | ✅     |
| `rrand`  | Current seed  | ✅     |

---

## 🔹 Type Conversion & Info

| Operator | Description                | Status |
| -------- | -------------------------- | ------ |
| `type`   | Return type name           | ✅     |
| `cvlit`  | Convert to literal form    | ✅     |
| `cvx`    | Convert to executable form | ✅     |
| `cvi`    | Convert to integer         | ✅     |
| `cvr`    | Convert to real            | ⬜     |
| `cvs`    | Convert to string          | ✅     |
| `cvrs`   | Convert number w/ radix    | ⬜     |
| `cvn`    | Convert to name            | ✅     |

---

## 🔹 Type and Access Checks

| Operator     | Description            | Status |
| ------------ | ---------------------- | ------ |
| `xcheck`     | Executable?            | ✅     |
| `rcheck`     | Readable?              | ⬜     |
| `wcheck`     | Writable?              | ⬜     |
| `readonly`   | Make readonly          | ⬜     |
| `executeonly`| Make execute-only      | ⬜     |
| `noaccess`   | Make no-access         | ⬜     |
| `writable`   | Make writable          | ⬜     |
| `executable` | Make executable        | ⬜     |

---

## 🔹 Control Flow Operators

| Operator  | Description                | Status |
| --------- | -------------------------- | ------ |
| `exec`    | Execute procedure          | ✅     |
| `if`      | Conditional execution      | ✅     |
| `ifelse`  | Conditional branching      | ✅     |
| `for`     | Counted loop               | ✅     |
| `repeat`  | Loop fixed times           | ✅     |
| `loop`    | Infinite loop              | ✅     |
| `exit`    | Exit from loop             | ✅     |
| `stop`    | Stop execution             | ✅     |
| `stopped` | Catch `stop`               | ✅     |

---

## 🔹 Dictionary Operators

| Operator         | Description                        | Status |
| ---------------- | ---------------------------------- | ------ |
| `def`            | Define entry in current dictionary | ✅     |
| `dict`           | Create dictionary                  | ✅     |
| `begin`          | Push dictionary onto stack         | ✅     |
| `end`            | Pop dictionary stack               | ✅     |
| `load`           | Lookup value                       | ✅     |
| `store`          | Store into dictionary              | ✅     |
| `known`          | Is key present?                    | ✅     |
| `where`          | Find containing dictionary         | ✅     |
| `currentdict`    | Current dictionary                 | ✅     |
| `countdictstack` | Number of dictionaries             | ✅     |
| `maxlength`      | Maximum size of dictionary         | ✅     |
| `systemdict`     | System dictionary                  | ✅     |
| `userdict`       | User dictionary                    | ✅     |

---

## 🔹 Array Operators

| Operator      | Description         | Status |
| ------------- | ------------------- | ------ |
| `array`       | Create array        | ✅     |
| `length`      | Number of elements  | ✅     |
| `get`         | Retrieve element    | ✅     |
| `put`         | Store element       | ✅     |
| `getinterval` | Sub-array           | ✅     |
| `putinterval` | Store sub-array     | ✅     |
| `astore`      | Pop stack to array  | ✅     |
| `aload`       | Push array to stack | ✅     |
| `bind`        | Bind names to ops   | ✅     |
| `packedarray` | Create packed array | ⬜     |

---

## 🔹 String Operators

| Operator      | Description     | Status |
| ------------- | --------------- | ------ |
| `string`      | Create string   | ✅     |
| `length`      | String length   | ✅     |
| `get`         | Get byte        | ✅     |
| `put`         | Set byte        | ✅     |
| `getinterval` | Substring       | ✅     |
| `putinterval` | Copy bytes in   | ✅     |

---

## 🔹 File Operators

| Operator     | Description            | Status |
| ------------ | ---------------------- | ------ |
| `file`       | Open file              | ✅     |
| `closefile`  | Close file             | ⬜     |
| `read`       | Read one byte          | ⬜     |
| `write`      | Write one byte         | ⬜     |
| `readline`   | Read a line            | ✅     |
| `flush`      | Flush stdout           | ✅     |
| `flushfile`  | Flush file buffer      | ⬜     |
| `resetfile`  | Reset to beginning     | ⬜     |
| `readhexstring` | Read hex into string | ⬜     |
| `readstring` | Read string from file  | ⬜     |
| `writestring`| Write string to file   | ⬜     |
| `fileposition` | Get file position    | ⬜     |
| `setfileposition` | Set file position | ⬜     |
| `deletefile` | Delete a file          | ⬜     |
| `renamefile` | Rename file            | ⬜     |
| `status`     | File/device status     | ⬜     |

---

## 🔹 Graphics State & Path Operators

| Operator       | Description              | Status |
| -------------- | ------------------------ | ------ |
| `initgraphics` | Reset graphics state     | ✅     |
| `gsave`        | Save graphics state      | ✅     |
| `grestore`     | Restore graphics state   | ✅     |
| `grestoreall`  | Restore all states       | ⬜     |
| `save`         | Save VM state            | ⬜     |
| `restore`      | Restore VM state         | ⬜     |
| `currentpoint` | Get current point        | ✅     |
| `newpath`      | Begin new path           | ✅     |
| `moveto`       | Move to position         | ✅     |
| `lineto`       | Draw line                | ✅     |
| `curveto`      | Draw Bézier curve        | ✅     |
| `arc`          | Arc (CCW)                | ✅     |
| `arcn`         | Arc (CW)                 | ✅     |
| `arcto`        | Arc to tangent           | ✅     |
| `closepath`    | Close current subpath    | ✅     |
| `stroke`       | Stroke path              | ✅     |
| `fill`         | Fill path                | ✅     |
| `eofill`       | Even-odd fill            | ✅     |
| `clip`         | Set clip path            | ⬜     |
| `eoclip`       | Even-odd clip            | ⬜     |
| `initclip`     | Reset clip path          | ⬜     |

---

## 🔹 Matrix Operators

| Operator         | Description             | Status |
| ---------------- | ----------------------- | ------ |
| `matrix`         | Create identity matrix  | ✅     |
| `currentmatrix`  | Get CTM                 | ✅     |
| `setmatrix`      | Replace CTM             | ✅     |
| `concat`         | Concatenate CTM         | ✅     |
| `concatmatrix`   | Multiply matrices       | ✅     |
| `initmatrix`     | Reset CTM               | ✅     |
| `defaultmatrix`  | Get default CTM         | ✅     |
| `transform`      | Transform point         | ✅     |
| `dtransform`     | Transform vector        | ✅     |
| `idtransform`    | Inverse vector xform    | ✅     |
| `itransform`     | Inverse point xform     | ✅     |

---

## 🔹 Text Operators

| Operator     | Description             | Status |
| ------------ | ----------------------- | ------ |
| `show`       | Render string           | ⬜     |
| `stringwidth`| Measure string width    | ⬜     |
| `charpath`   | Append glyph path       | ⬜     |

---

## 🔹 Path Analysis Operators

| Operator      | Description             | Status |
| ------------- | ----------------------- | ------ |
| `flattenpath` | Flatten Béziers         | ⬜     |
| `reversepath` | Reverse path segments   | ⬜     |
| `pathbbox`    | Get bounding box        | ✅     |
| `pathforall`  | Iterate over segments   | ⬜     |

---

## 🔹 Debugging / Introspection

| Operator       | Description            | Status |
| -------------- | ---------------------- | ------ |
| `=`            | Print operand          | ✅     |
| `==`           | Print verbosely        | ✅     |
| `stack`        | Dump operand stack     | ✅     |
| `pstack`       | Pretty print stack     | ✅     |
| `print`        | Print string           | ✅     |
| `errordict`    | Error dictionary       | ✅     |
| `handleerror`  | Handle error trap      | ✅     |
| `$error`       | Current error object   | ⬜     |
