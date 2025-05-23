# PostScript Operator Registry

This is the list of PostScript operators.  There is a checkbox to indicate whether it is currently implemented or not.


---

## Core Stack Operators

| Operator      | Description                           | Implemented |
| ------------- | ------------------------------------- | ----------- |
| `pop`         | Remove top element from operand stack | \[ X]        |
| `dup`         | Duplicate top element                 | \[ X]        |
| `exch`        | Exchange top two elements             | \[ X]        |
| `copy`        | Duplicate multiple elements           | \[ X]        |
| `index`       | Copy nth element to top               | \[ X]        |
| `roll`        | Rotate top n elements                 | \[ X]        |
| `clear`       | Clear operand stack                   | \[ X]        |
| `count`       | Push number of elements on stack      | \[ X]        |
| `mark`        | Push mark object                      | \[ X]        |
| `cleartomark` | Pop to last mark                      | \[ X]        |
| `counttomark` | Count to last mark                    | \[ X]        |

## Relational Operators

| Operator | Description         | Implemented |
| -------- | ------------------- | ----------- |
| `eq`     | Test for equality   | \[ ]        |
| `ne`     | Test for inequality | \[ X]        |
| `gt`     | Greater than        | \[ X]        |
| `lt`     | Less than           | \[ X]        |
| `ge`     | Greater or equal    | \[ X]        |
| `le`     | Less or equal       | \[ X]        |

## Logical Operators

| Operator | Description            | Implemented |
| -------- | ---------------------- | ----------- |
| `and`    | Bitwise or logical AND | \[ X]        |
| `or`     | Bitwise or logical OR  | \[ X]        |
| `xor`    | Bitwise or logical XOR | \[ X]        |
| `not`    | Bitwise or logical NOT | \[ X]        |

## Arithmetic Operators

| Operator   | Description             | Implemented |
| ---------- | ----------------------- | ----------- |
| `add`      | Addition                | \[ X]        |
| `sub`      | Subtraction             | \[ X]        |
| `mul`      | Multiplication          | \[ X]        |
| `div`      | Real division           | \[ X]        |
| `idiv`     | Integer division        | \[ X]        |
| `mod`      | Integer modulus         | \[ X]        |
| `neg`      | Negate                  | \[ X]        |
| `abs`      | Absolute value          | \[ X]        |
| `ceiling`  | Round up                | \[ X]        |
| `floor`    | Round down              | \[ X]        |
| `round`    | Round to nearest        | \[ X]        |
| `truncate` | Discard fractional part | \[ X]        |
| `sqrt`     | Square root             | \[ X]        |
| `max`      | Maximum                 | \[ X]        |
| `min`      | Minimum                 | \[ X]        |

## Exponential & Logarithmic

| Operator | Description       | Implemented |
| -------- | ----------------- | ----------- |
| `exp`    | Exponentiation    | \[ X]        |
| `ln`     | Natural logarithm | \[ X]        |
| `log`    | Base-10 logarithm | \[ X]        |

## Trigonometric Operators

| Operator | Description        | Implemented |
| -------- | ------------------ | ----------- |
| `sin`    | Sine (degrees)     | \[ X]        |
| `cos`    | Cosine (degrees)   | \[ X]        |
| `atan`   | Arctangent (dy dx) | \[ X]        |

## Random Number Operators

| Operator | Description   | Implemented |
| -------- | ------------- | ----------- |
| `rand`   | Random number | \[ X]        |
| `srand`  | Set seed      | \[ X]        |
| `rrand`  | Current seed  | \[ X]        |

## Type Operators

| Operator | Description                | Implemented |
| -------- | -------------------------- | ----------- |
| `type`   | Return type name           | \[ X]        |
| `cvlit`  | Convert to literal form    | \[ ]        |
| `cvx`    | Convert to executable form | \[ ]        |
| `cvi`    | Convert to integer         | \[ ]        |
| `cvr`    | Convert to real            | \[ ]        |
| `cvs`    | Convert to string          | \[ X]        |

## Control Flow Operators

| Operator  | Description                | Implemented |
| --------- | -------------------------- | ----------- |
| `if`      | Conditional execution      | \[X]        |
| `ifelse`  | Conditional branching      | \[X]        |
| `for`     | Counted loop               | \[X]        |
| `repeat`  | Loop fixed number of times | \[X]        |
| `loop`    | Infinite loop              | \[X]        |
| `exit`    | Exit loop                  | \[X]        |
| `stop`    | Stop execution             | \[X]        |
| `stopped` | Catch stop                 | \[X]        |

## Dictionary Operators

| Operator         | Description                        | Implemented |
| ---------------- | ---------------------------------- | ----------- |
| `dict`           | Create dictionary                  | \[X]        |
| `begin`          | Push dictionary onto stack         | \[X]        |
| `end`            | Pop dictionary stack               | \[X]        |
| `def`            | Define entry in current dictionary | \[X]        |
| `load`           | Lookup value                       | \[X]        |
| `store`          | Store into dictionary              | \[ ]        |
| `known`          | Is key present?                    | \[X]        |
| `where`          | Find dictionary containing key     | \[X]        |
| `currentdict`    | Current dictionary                 | \[X]        |
| `countdictstack` | Number of dictionaries             | \[X]        |
| `maxlength`      | Maximum size of dictionary         | \[X]        |

## Array Operators

| Operator      | Description         | Implemented |
| ------------- | ------------------- | ----------- |
| `array`       | Create array        | \[ X]        |
| `length`      | Number of elements  | \[ X]        |
| `get`         | Retrieve element    | \[ X]        |
| `put`         | Store element       | \[ X]        |
| `getinterval` | Sub-array           | \[ X]        |
| `putinterval` | Store sub-array     | \[ X]        |
| `astore`      | Pop stack to array  | \[ X]        |
| `aload`       | Push array to stack | \[ X]        |

## String Operators

| Operator      | Description     | Implemented |
| ------------- | --------------- | ----------- |
| `string`      | Create string   | \[ ]        |
| `length`      | String length   | \[X]        |
| `get`         | Get byte        | \[X]        |
| `put`         | Set byte        | \[X]        |
| `getinterval` | Substring       | \[ ]        |
| `putinterval` | Copy bytes in   | \[ ]        |
| `cvn`         | Convert to name | \[ ]        |

## File Operators

| Operator    | Description  | Implemented |
| ----------- | ------------ | ----------- |
| `file`      | Open file    | \[ ]        |
| `closefile` | Close file   | \[ ]        |
| `read`      | Read a byte  | \[ ]        |
| `write`     | Write a byte | \[ ]        |
| `readline`  | Read a line  | \[ ]        |
| `flush`     | Flush output | \[ ]        |
| `flushfile` | Flush file   | \[ ]        |
| `resetfile` | Reset file   | \[ ]        |

## Graphics Operators

| Operator       | Description              | Implemented |
| -------------- | ------------------------ | ----------- |
| `initgraphics` | Reset graphics state     | \[ ]        |
| `gsave`        | Save graphics state      | \[ ]        |
| `grestore`     | Restore graphics state   | \[ ]        |
| `grestoreall`  | Restore all states       | \[ ]        |
| `currentpoint` | Current drawing point    | \[ ]        |
| `newpath`      | Begin new path           | \[ ]        |
| `moveto`       | Move pen                 | \[ ]        |
| `lineto`       | Draw line                | \[ ]        |
| `curveto`      | Draw Bézier curve        | \[ ]        |
| `arc`          | Arc by angle             | \[ ]        |
| `arcn`         | Counter arc              | \[ ]        |
| `arcto`        | Tangent arcs             | \[ ]        |
| `closepath`    | Close subpath            | \[ ]        |
| `stroke`       | Stroke path              | \[ ]        |
| `fill`         | Fill path                | \[ ]        |
| `eofill`       | Fill using even-odd rule | \[ ]        |
| `clip`         | Set clip path            | \[ ]        |
| `eoclip`       | Even-odd clip            | \[ ]        |
| `initclip`     | Reset clip               | \[ ]        |
| `show`         | Render string            | \[ ]        |
| `stringwidth`  | Measure string           | \[ ]        |
| `charpath`     | Path of glyph            | \[ ]        |
| `flattenpath`  | Flatten curves           | \[ ]        |
| `reversepath`  | Reverse segments         | \[ ]        |
| `pathbbox`     | Path bounds              | \[ ]        |
| `pathforall`   | Enumerate path           | \[ ]        |

## Debugging / Introspection

| Operator | Description           | Implemented |
| -------- | --------------------- | ----------- |
| `==`     | Print top of stack    | \[X]        |
| `stack`  | Dump operand stack    | \[X]        |
| `pstack` | Pretty print stack    | \[X]        |
| `=only`  | Print without newline | \[X]        |
| `print`  | Print string          | \[X]        |

## PostScript Level 2/3

| Operator   | Description           | Implemented |
| ---------- | --------------------- | ----------- |
| `usertime` | Time since VM startup | \[ ]        |
| `realtime` | Wall clock time       | \[ ]        |
| `true`     | Boolean               | \[ ]        |
| `false`    | Boolean               | \[ ]        |
| `version`  | Interpreter version   | \[ ]        |
| `revision` | Build revision        | \[ ]        |

---

