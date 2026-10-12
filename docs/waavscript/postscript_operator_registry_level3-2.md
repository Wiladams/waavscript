# PostScript Operator Registry — LanguageLevel 1 through 3

This expanded registry is based primarily on the Adobe *PostScript Language Reference, Third Edition* operator set.
It includes the standard system operators through LanguageLevel 3 plus the important standard Level 3 procedure-set operators used by CID fonts, bitmap fonts, and trapping.

Implementation status is conservative:

- 🟩 **Implemented / confirmed** — present as implemented in the supplied WaavScript registry, or explicitly confirmed in recent WaavScript work.
- 🟥 **Explicitly not implemented** — the supplied registry specifically marked this operator as not implemented.
- ⬜ **Not confirmed** — added from the PostScript reference, but I do not currently have positive evidence that WaavScript implements it. This does **not** necessarily mean it is absent.

> Level note: `1`, `2`, and `3` indicate the formal language generation in which the operator is standard. Some Level 2 facilities also appeared historically as Level 1 product extensions. `ext` means a WaavScript/non-PLRM extension retained from your original registry.

> Scope note: this is a language/interpreter registry, not a list of every product-specific `statusdict` compatibility name, every device parameter, every resource key, or every filter name. Those are separate namespaces/features rather than the core operator surface.

## Core Stack Operators

| Operator      | Level | Description                                 | Status        |
| ------------- | :---: | ------------------------------------------- | ------------- |
| `pop`         | 1     | Discard top operand                         | 🟩 Implemented |
| `exch`        | 1     | Exchange top two operands                   | 🟩 Implemented |
| `dup`         | 1     | Duplicate top operand                       | 🟩 Implemented |
| `copy`        | 1     | Duplicate/copy objects or composite objects | 🟩 Implemented |
| `index`       | 1     | Copy indexed stack element to top           | 🟩 Implemented |
| `roll`        | 1     | Rotate top n stack elements                 | 🟩 Implemented |
| `clear`       | 1     | Clear operand stack                         | 🟩 Implemented |
| `count`       | 1     | Count operand stack                         | 🟩 Implemented |
| `mark`        | 1     | Push mark object                            | 🟩 Implemented |
| `cleartomark` | 1     | Discard through nearest mark                | 🟩 Implemented |
| `counttomark` | 1     | Count objects above nearest mark            | 🟩 Implemented |

## Arithmetic, Math & Random Operators

| Operator   | Level | Description                    | Status        |
| ---------- | :---: | ------------------------------ | ------------- |
| `add`      | 1     | Addition                       | 🟩 Implemented |
| `sub`      | 1     | Subtraction                    | 🟩 Implemented |
| `mul`      | 1     | Multiplication                 | 🟩 Implemented |
| `div`      | 1     | Real division                  | 🟩 Implemented |
| `idiv`     | 1     | Integer division               | 🟩 Implemented |
| `mod`      | 1     | Integer remainder              | 🟩 Implemented |
| `abs`      | 1     | Absolute value                 | 🟩 Implemented |
| `neg`      | 1     | Negate                         | 🟩 Implemented |
| `ceiling`  | 1     | Round toward +infinity         | 🟩 Implemented |
| `floor`    | 1     | Round toward -infinity         | 🟩 Implemented |
| `round`    | 1     | Round to nearest integer       | 🟩 Implemented |
| `truncate` | 1     | Discard fractional part        | 🟩 Implemented |
| `sqrt`     | 1     | Square root                    | 🟩 Implemented |
| `atan`     | 1     | Arctangent in degrees          | 🟩 Implemented |
| `cos`      | 1     | Cosine in degrees              | 🟩 Implemented |
| `sin`      | 1     | Sine in degrees                | 🟩 Implemented |
| `exp`      | 1     | Exponentiation                 | 🟩 Implemented |
| `ln`       | 1     | Natural logarithm              | 🟩 Implemented |
| `log`      | 1     | Base-10 logarithm              | 🟩 Implemented |
| `rand`     | 1     | Generate pseudo-random integer | 🟩 Implemented |
| `srand`    | 1     | Set random seed                | 🟩 Implemented |
| `rrand`    | 1     | Return current random seed     | 🟩 Implemented |

## Array & Packed Array Operators

| Operator         | Level | Description                        | Status            |
| ---------------- | :---: | ---------------------------------- | ----------------- |
| `[`              | 1     | Begin array construction           | 🟩 Implemented     |
| `]`              | 1     | End array construction             | 🟩 Implemented     |
| `array`          | 1     | Create array                       | 🟩 Implemented     |
| `length`         | 1     | Return composite object length     | 🟩 Implemented     |
| `get`            | 1     | Get indexed/keyed element          | 🟩 Implemented     |
| `put`            | 1     | Store indexed/keyed element        | 🟩 Implemented     |
| `getinterval`    | 1     | Return subarray/sub-string         | 🟩 Implemented     |
| `putinterval`    | 1     | Replace interval                   | 🟩 Implemented     |
| `astore`         | 1     | Pop operands into array            | 🟩 Implemented     |
| `aload`          | 1     | Push array elements                | 🟩 Implemented     |
| `forall`         | 1     | Execute procedure for each element | 🟩 Implemented     |
| `packedarray`    | 2     | Create packed array                | 🟥 Not implemented |
| `setpacking`     | 2     | Set procedure packing mode         | ⬜ Not confirmed   |
| `currentpacking` | 2     | Return procedure packing mode      | ⬜ Not confirmed   |

## Dictionary Operators

| Operator         | Level | Description                          | Status          |
| ---------------- | :---: | ------------------------------------ | --------------- |
| `<<`             | 2     | Begin dictionary construction        | 🟩 Not confirmed |
| `>>`             | 2     | End dictionary construction          | 🟩 Not confirmed |
| `dict`           | 1     | Create dictionary                    | 🟩 Implemented   |
| `maxlength`      | 1     | Return dictionary capacity           | 🟩 Implemented   |
| `begin`          | 1     | Push dictionary on dictionary stack  | 🟩 Implemented   |
| `end`            | 1     | Pop dictionary stack                 | 🟩 Implemented   |
| `def`            | 1     | Define key in current dictionary     | 🟩 Implemented   |
| `load`           | 1     | Look up key on dictionary stack      | 🟩 Implemented   |
| `store`          | 1     | Replace topmost definition or define | 🟩 Implemented   |
| `known`          | 1     | Test key membership                  | 🟩 Implemented   |
| `where`          | 1     | Find dictionary containing key       | 🟩 Implemented   |
| `currentdict`    | 1     | Return current dictionary            | 🟩 Implemented   |
| `undef`          | 2     | Remove dictionary entry              | ⬜ Not confirmed |
| `countdictstack` | 1     | Count dictionary stack               | 🟩 Implemented   |
| `dictstack`      | 1     | Copy dictionary stack to array       | 🟩 Not confirmed |
| `cleardictstack` | 1     | Pop nonpermanent dictionaries        | ⬜ Not confirmed |

## String Operators

| Operator       | Level | Description                   | Status          |
| -------------- | :---: | ----------------------------- | --------------- |
| `string`       | 1     | Create string                 | 🟩 Implemented   |
| `anchorsearch` | 1     | Search at beginning of string | 🟩 Implemented   |
| `search`       | 1     | Search for substring          | 🟩 Implemented   |
| `token`        | 1     | Scan one PostScript token     | 🟩 Implemented   |

## Relational, Boolean & Bitwise Operators

| Operator   | Level | Description           | Status          |
| ---------- | :---: | --------------------- | --------------- |
| `eq`       | 1     | Equal                 | 🟩 Implemented   |
| `ne`       | 1     | Not equal             | 🟩 Implemented   |
| `ge`       | 1     | Greater than or equal | 🟩 Implemented   |
| `gt`       | 1     | Greater than          | 🟩 Implemented   |
| `le`       | 1     | Less than or equal    | 🟩 Implemented   |
| `lt`       | 1     | Less than             | 🟩 Implemented   |
| `and`      | 1     | Logical/bitwise AND   | 🟩 Implemented   |
| `or`       | 1     | Logical/bitwise OR    | 🟩 Implemented   |
| `xor`      | 1     | Logical/bitwise XOR   | 🟩 Implemented   |
| `not`      | 1     | Logical/bitwise NOT   | 🟩 Implemented   |
| `bitshift` | 1     | Bitwise shift         | ⬜ Not confirmed |
| `true`     | 1     | Push boolean true     | ⬜ Not confirmed |
| `false`    | 1     | Push boolean false    | ⬜ Not confirmed |

## Control Flow Operators

| Operator         | Level | Description                   | Status          |
| ---------------- | :---: | ----------------------------- | --------------- |
| `exec`           | 1     | Execute arbitrary object      | 🟩 Implemented   |
| `if`             | 1     | Conditional execution         | 🟩 Implemented   |
| `ifelse`         | 1     | Conditional branch            | 🟩 Implemented   |
| `for`            | 1     | Numeric for loop              | 🟩 Implemented   |
| `repeat`         | 1     | Repeat procedure              | 🟩 Implemented   |
| `loop`           | 1     | Indefinite loop               | 🟩 Implemented   |
| `exit`           | 1     | Exit innermost loop           | 🟩 Implemented   |
| `stop`           | 1     | Terminate stopped context     | 🟩 Implemented   |
| `stopped`        | 1     | Catch stop                    | 🟩 Implemented   |
| `countexecstack` | 1     | Count execution stack         | ⬜ Not confirmed |
| `execstack`      | 1     | Copy execution stack          | ⬜ Not confirmed |
| `quit`           | 1     | Terminate interpreter         | ⬜ Not confirmed |
| `start`          | 1     | Interpreter startup procedure | ⬜ Not confirmed |

## Type, Attribute & Conversion Operators

| Operator      | Level | Description                          | Status            |
| ------------- | :---: | ------------------------------------ | ----------------- |
| `type`        | 1     | Return object type                   | 🟩 Implemented     |
| `cvlit`       | 1     | Make object literal                  | 🟩 Implemented     |
| `cvx`         | 1     | Make object executable               | 🟩 Implemented     |
| `xcheck`      | 1     | Test executable attribute            | 🟩 Implemented     |
| `executeonly` | 1     | Reduce access to execute-only        | 🟩 Implemented     |
| `noaccess`    | 1     | Disallow access                      | 🟩 Implemented     |
| `readonly`    | 1     | Reduce access to read-only           | 🟩 Implemented     |
| `rcheck`      | 1     | Test read access                     | 🟩 Implemented     |
| `wcheck`      | 1     | Test write access                    | 🟩 Implemented     |
| `cvi`         | 1     | Convert to integer                   | 🟩 Implemented     |
| `cvn`         | 1     | Convert string to name               | 🟩 Implemented     |
| `cvr`         | 1     | Convert to real                      | 🟩 Implemented     |
| `cvrs`        | 1     | Convert number to string using radix | 🟩 Implemented     |
| `cvs`         | 1     | Convert object to string             | 🟩 Implemented     |

## File, Stream & Console Operators

| Operator              | Level | Description                          | Status            |
| --------------------- | :---: | ------------------------------------ | ----------------- |
| `file`                | 1     | Open named file                      | 🟩 Implemented     |
| `filter`              | 2     | Create filtered file/stream          | ⬜ Not confirmed   |
| `closefile`           | 1     | Close file                           | 🟩 Implemented     |
| `read`                | 1     | Read one byte                        | 🟥 Not implemented |
| `write`               | 1     | Write one byte                       | 🟥 Not implemented |
| `readhexstring`       | 1     | Read hex bytes into string           | 🟩 Implemented     |
| `writehexstring`      | 1     | Write string as hex                  | ⬜ Not confirmed   |
| `readstring`          | 1     | Read bytes into string               | 🟩 Implemented     |
| `writestring`         | 1     | Write string                         | 🟥 Not implemented |
| `readline`            | 1     | Read line                            | 🟩 Implemented     |
| `token`               | 1     | Read token from file/string          | ⬜ Not confirmed   |
| `bytesavailable`      | 1     | Return bytes available               | ⬜ Not confirmed   |
| `flush`               | 1     | Flush standard output                | 🟩 Implemented     |
| `flushfile`           | 1     | Flush output or consume input to EOF | 🟥 Not implemented |
| `resetfile`           | 1     | Reset buffered state                 | 🟩 Implemented     |
| `status`              | 1     | Return file/device status            | 🟥 Not implemented |
| `run`                 | 1     | Execute named file                   | 🟩 Implemented     |
| `currentfile`         | 1     | Return current input file            | ⬜ Not confirmed   |
| `deletefile`          | 1     | Delete named file                    | 🟥 Not implemented |
| `renamefile`          | 1     | Rename file                          | 🟥 Not implemented |
| `filenameforall`      | 1     | Enumerate matching filenames         | ⬜ Not confirmed   |
| `setfileposition`     | 1     | Seek file                            | 🟩 Implemented     |
| `fileposition`        | 1     | Return file position                 | 🟩 Implemented     |
| `print`               | 1     | Write string to stdout               | 🟩 Implemented     |
| `=`                   | 1     | Print text representation            | 🟩 Implemented     |
| `==`                  | 1     | Print syntactic representation       | 🟩 Implemented     |
| `stack`               | 1     | Print operand stack                  | 🟩 Implemented     |
| `pstack`              | 1     | Pretty-print operand stack           | 🟩 Implemented     |
| `printobject`         | 2     | Write binary object                  | ⬜ Not confirmed   |
| `writeobject`         | 2     | Write binary object to file          | ⬜ Not confirmed   |
| `setobjectformat`     | 2     | Set binary object format             | ⬜ Not confirmed   |
| `currentobjectformat` | 2     | Return binary object format          | ⬜ Not confirmed   |

## Named Resource Operators

| Operator           | Level | Description                     | Status          |
| ------------------ | :---: | ------------------------------- | --------------- |
| `defineresource`   | 2     | Register named resource         | 🟩 Implemented   |
| `undefineresource` | 2     | Remove resource registration    | ⬜ Not confirmed |
| `findresource`     | 2     | Find/load named resource        | 🟩 Implemented   |
| `resourcestatus`   | 2     | Query named resource            | ⬜ Not confirmed |
| `resourceforall`   | 2     | Enumerate resources in category | 🟩 Implemented   |

## Virtual Memory & User Object Operators

| Operator             | Level | Description                           | Status            |
| -------------------- | :---: | ------------------------------------- | ----------------- |
| `save`               | 1     | Create VM snapshot                    | 🟥 Not implemented |
| `restore`            | 1     | Restore VM snapshot                   | 🟥 Not implemented |
| `currentglobal`      | 2     | Return global/local allocation mode   | ⬜ Not confirmed   |
| `setglobal`          | 2     | Set allocation mode                   | ⬜ Not confirmed   |
| `gcheck`             | 2     | Test whether object is global         | ⬜ Not confirmed   |
| `currentshared`      | 2     | Compatibility alias for currentglobal | ⬜ Not confirmed   |
| `setshared`          | 2     | Compatibility alias for setglobal     | ⬜ Not confirmed   |
| `scheck`             | 2     | Compatibility shared-VM check         | ⬜ Not confirmed   |
| `startjob`           | 2     | Start job affecting initial VM        | ⬜ Not confirmed   |
| `defineuserobject`   | 2     | Define indexed user object            | ⬜ Not confirmed   |
| `execuserobject`     | 2     | Execute indexed user object           | ⬜ Not confirmed   |
| `undefineuserobject` | 2     | Remove indexed user object            | ⬜ Not confirmed   |

## Miscellaneous Interpreter Operators

| Operator        | Level | Description                      | Status          |
| --------------- | :---: | -------------------------------- | --------------- |
| `bind`          | 1     | Bind operator names in procedure | 🟩 Implemented   |
| `null`          | 1     | Push null object                 | 🟩 Implemented   |
| `version`       | 1     | Interpreter version string       | ⬜ Not confirmed |
| `realtime`      | 1     | Real time in milliseconds        | 🟩 Implemented   |
| `usertime`      | 1     | Execution time in milliseconds   | 🟩 Implemented   |
| `languagelevel` | 2     | Current LanguageLevel            | 🟩 Implemented   |
| `product`       | 1     | Product name                     | 🟩 Implemented   |
| `revision`      | 1     | Product revision                 | ⬜ Not confirmed |
| `serialnumber`  | 1     | Machine serial number            | ⬜ Not confirmed |
| `executive`     | 1     | Invoke interactive executive     | ⬜ Not confirmed |
| `echo`          | 1     | Enable/disable input echo        | ⬜ Not confirmed |
| `prompt`        | 1     | Interactive prompt procedure     | ⬜ Not confirmed |

## Graphics State — Device Independent

| Operator              | Level | Description                           | Status            |
| --------------------- | :---: | ------------------------------------- | ----------------- |
| `gsave`               | 1     | Save graphics state                   | 🟩 Implemented     |
| `grestore`            | 1     | Restore graphics state                | 🟩 Implemented     |
| `grestoreall`         | 1     | Restore bottommost graphics state     | 🟥 Not implemented |
| `initgraphics`        | 1     | Reset graphics state                  | 🟩 Implemented     |
| `gstate`              | 2     | Create graphics-state object          | ⬜ Not confirmed   |
| `setgstate`           | 2     | Install graphics-state object         | ⬜ Not confirmed   |
| `currentgstate`       | 2     | Copy current state into gstate object | ⬜ Not confirmed   |
| `setlinewidth`        | 1     | Set line width                        | ⬜ Not confirmed   |
| `currentlinewidth`    | 1     | Return line width                     | ⬜ Not confirmed   |
| `setlinecap`          | 1     | Set line cap                          | ⬜ Not confirmed   |
| `currentlinecap`      | 1     | Return line cap                       | ⬜ Not confirmed   |
| `setlinejoin`         | 1     | Set line join                         | ⬜ Not confirmed   |
| `currentlinejoin`     | 1     | Return line join                      | ⬜ Not confirmed   |
| `setmiterlimit`       | 1     | Set miter limit                       | ⬜ Not confirmed   |
| `currentmiterlimit`   | 1     | Return miter limit                    | ⬜ Not confirmed   |
| `setstrokeadjust`     | 2     | Set stroke adjustment                 | ⬜ Not confirmed   |
| `currentstrokeadjust` | 2     | Return stroke adjustment              | ⬜ Not confirmed   |
| `setdash`             | 1     | Set dash pattern                      | ⬜ Not confirmed   |
| `currentdash`         | 1     | Return dash pattern                   | ⬜ Not confirmed   |
| `setcolorspace`       | 2     | Set current color space               | ⬜ Not confirmed   |
| `currentcolorspace`   | 2     | Return current color space            | ⬜ Not confirmed   |
| `setcolor`            | 2     | Set current color components          | ⬜ Not confirmed   |
| `currentcolor`        | 2     | Return current color                  | ⬜ Not confirmed   |
| `setgray`             | 1     | Set DeviceGray color                  | ⬜ Not confirmed   |
| `currentgray`         | 1     | Return gray color                     | ⬜ Not confirmed   |
| `sethsbcolor`         | 1     | Set HSB color                         | ⬜ Not confirmed   |
| `currenthsbcolor`     | 1     | Return HSB color                      | ⬜ Not confirmed   |
| `setrgbcolor`         | 1     | Set RGB color                         | ⬜ Not confirmed   |
| `currentrgbcolor`     | 1     | Return RGB color                      | ⬜ Not confirmed   |
| `setcmykcolor`        | 1     | Set CMYK color                        | ⬜ Not confirmed   |
| `currentcmykcolor`    | 1     | Return CMYK color                     | ⬜ Not confirmed   |
| `clipsave`            | 3     | Save clipping path only               | ⬜ Not confirmed   |
| `cliprestore`         | 3     | Restore clipping path only            | ⬜ Not confirmed   |

## Graphics State — Device Dependent / Rendering

| Operator                   | Level | Description                        | Status          |
| -------------------------- | :---: | ---------------------------------- | --------------- |
| `sethalftone`              | 2     | Set halftone dictionary            | ⬜ Not confirmed |
| `currenthalftone`          | 2     | Return current halftone            | ⬜ Not confirmed |
| `setscreen`                | 1     | Set gray halftone screen           | ⬜ Not confirmed |
| `currentscreen`            | 1     | Return gray screen                 | ⬜ Not confirmed |
| `setcolorscreen`           | 1     | Set color halftone screens         | ⬜ Not confirmed |
| `currentcolorscreen`       | 1     | Return color screens               | ⬜ Not confirmed |
| `settransfer`              | 1     | Set gray transfer function         | ⬜ Not confirmed |
| `currenttransfer`          | 1     | Return gray transfer               | ⬜ Not confirmed |
| `setcolortransfer`         | 1     | Set color transfer functions       | ⬜ Not confirmed |
| `currentcolortransfer`     | 1     | Return color transfer functions    | ⬜ Not confirmed |
| `setblackgeneration`       | 1     | Set black-generation function      | ⬜ Not confirmed |
| `currentblackgeneration`   | 1     | Return black-generation function   | ⬜ Not confirmed |
| `setundercolorremoval`     | 1     | Set undercolor-removal function    | ⬜ Not confirmed |
| `currentundercolorremoval` | 1     | Return undercolor-removal          | ⬜ Not confirmed |
| `setcolorrendering`        | 2     | Set CIE color rendering dictionary | ⬜ Not confirmed |
| `currentcolorrendering`    | 2     | Return current CRD                 | ⬜ Not confirmed |
| `findcolorrendering`       | 3     | Find color rendering resource      | ⬜ Not confirmed |
| `setflat`                  | 1     | Set flatness                       | ⬜ Not confirmed |
| `currentflat`              | 1     | Return flatness                    | ⬜ Not confirmed |
| `setoverprint`             | 2     | Set overprint                      | ⬜ Not confirmed |
| `currentoverprint`         | 2     | Return overprint                   | ⬜ Not confirmed |
| `setsmoothness`            | 3     | Set shading smoothness             | ⬜ Not confirmed |
| `currentsmoothness`        | 3     | Return shading smoothness          | ⬜ Not confirmed |

## Coordinate System & Matrix Operators

| Operator        | Level | Description                                         | Status          |
| --------------- | :---: | --------------------------------------------------- | --------------- |
| `matrix`        | 1     | Create identity matrix                              | 🟩 Implemented   |
| `initmatrix`    | 1     | Reset CTM                                           | 🟩 Implemented   |
| `identmatrix`   | 1     | Fill matrix with identity                           | ⬜ Not confirmed |
| `defaultmatrix` | 1     | Get default matrix                                  | 🟩 Implemented   |
| `currentmatrix` | 1     | Get CTM                                             | 🟩 Implemented   |
| `setmatrix`     | 1     | Replace CTM                                         | 🟩 Implemented   |
| `translate`     | 1     | Translate user space / construct translation matrix | ⬜ Not confirmed |
| `scale`         | 1     | Scale user space / construct scale matrix           | ⬜ Not confirmed |
| `rotate`        | 1     | Rotate user space / construct rotation matrix       | ⬜ Not confirmed |
| `concat`        | 1     | Concatenate matrix with CTM                         | 🟩 Implemented   |
| `concatmatrix`  | 1     | Multiply matrices                                   | 🟩 Implemented   |
| `transform`     | 1     | Transform point                                     | 🟩 Implemented   |
| `dtransform`    | 1     | Transform displacement                              | 🟩 Implemented   |
| `itransform`    | 1     | Inverse-transform point                             | 🟩 Implemented   |
| `idtransform`   | 1     | Inverse-transform displacement                      | 🟩 Implemented   |
| `invertmatrix`  | 1     | Invert matrix                                       | ⬜ Not confirmed |

## Path Construction, Analysis & Clipping

| Operator       | Level | Description                                   | Status            |
| -------------- | :---: | --------------------------------------------- | ----------------- |
| `newpath`      | 1     | Clear current path                            | 🟩 Implemented     |
| `currentpoint` | 1     | Return current point                          | 🟩 Implemented     |
| `moveto`       | 1     | Start subpath                                 | 🟩 Implemented     |
| `rmoveto`      | 1     | Relative moveto                               | ⬜ Not confirmed   |
| `lineto`       | 1     | Append line                                   | 🟩 Implemented     |
| `rlineto`      | 1     | Append relative line                          | ⬜ Not confirmed   |
| `arc`          | 1     | Append counterclockwise arc                   | 🟩 Implemented     |
| `arcn`         | 1     | Append clockwise arc                          | 🟩 Implemented     |
| `arcto`        | 1     | Append tangent arc and return tangency points | 🟩 Implemented     |
| `arct`         | 2     | Append tangent arc                            | 🟩 Implemented     |
| `curveto`      | 1     | Append cubic Bézier                           | 🟩 Implemented     |
| `rcurveto`     | 1     | Append relative cubic Bézier                  | ⬜ Not confirmed   |
| `closepath`    | 1     | Close subpath                                 | 🟩 Implemented     |
| `flattenpath`  | 1     | Flatten curves                                | 🟩 Implemented     |
| `reversepath`  | 1     | Reverse path direction                        | 🟥 Not implemented |
| `strokepath`   | 1     | Replace path by stroke outline                | ⬜ Not confirmed   |
| `ustrokepath`  | 2     | Stroke-outline user path                      | ⬜ Not confirmed   |
| `charpath`     | 1     | Append glyph outlines                         | 🟩 Implemented     |
| `uappend`      | 2     | Append user path                              | ⬜ Not confirmed   |
| `clippath`     | 1     | Replace current path with clipping path       | ⬜ Not confirmed   |
| `setbbox`      | 2     | Set user-path bounding box                    | ⬜ Not confirmed   |
| `pathbbox`     | 1     | Return path bounding box                      | 🟩 Implemented     |
| `pathforall`   | 1     | Enumerate current path                        | 🟩 Implemented     |
| `upath`        | 2     | Return current path as user path              | ⬜ Not confirmed   |
| `ucache`       | 2     | Declare user path cacheable                   | ⬜ Not confirmed   |
| `initclip`     | 1     | Reset clip to device default                  | 🟩 Implemented     |
| `clip`         | 1     | Clip with nonzero winding                     | 🟥 Not implemented |
| `eoclip`       | 1     | Clip with even-odd rule                       | 🟥 Not implemented |
| `rectclip`     | 2     | Clip to rectangle(s)                          | ⬜ Not confirmed   |

## Painting, Images & Insideness

| Operator     | Level | Description                          | Status          |
| ------------ | :---: | ------------------------------------ | --------------- |
| `erasepage`  | 1     | Erase current page                   | ⬜ Not confirmed |
| `stroke`     | 1     | Stroke current path                  | 🟩 Implemented   |
| `fill`       | 1     | Fill current path                    | 🟩 Implemented   |
| `eofill`     | 1     | Even-odd fill                        | 🟩 Implemented   |
| `rectstroke` | 2     | Stroke rectangle(s)                  | ⬜ Not confirmed |
| `rectfill`   | 2     | Fill rectangle(s)                    | ⬜ Not confirmed |
| `ustroke`    | 2     | Stroke user path                     | ⬜ Not confirmed |
| `ufill`      | 2     | Fill user path                       | ⬜ Not confirmed |
| `ueofill`    | 2     | Even-odd fill user path              | ⬜ Not confirmed |
| `shfill`     | 3     | Paint shading                        | ⬜ Not confirmed |
| `image`      | 1     | Paint sampled image                  | ⬜ Not confirmed |
| `colorimage` | 1     | Paint color sampled image            | ⬜ Not confirmed |
| `imagemask`  | 1     | Paint through image mask             | ⬜ Not confirmed |
| `infill`     | 2     | Test point/path against fill         | ⬜ Not confirmed |
| `ineofill`   | 2     | Test against even-odd fill           | ⬜ Not confirmed |
| `instroke`   | 2     | Test against stroke                  | ⬜ Not confirmed |
| `inufill`    | 2     | Test against user-path fill          | ⬜ Not confirmed |
| `inueofill`  | 2     | Test against user-path even-odd fill | ⬜ Not confirmed |
| `inustroke`  | 2     | Test against user-path stroke        | ⬜ Not confirmed |

## Forms & Patterns

| Operator      | Level | Description         | Status          |
| ------------- | :---: | ------------------- | --------------- |
| `makepattern` | 2     | Instantiate pattern | ⬜ Not confirmed |
| `setpattern`  | 2     | Set pattern color   | ⬜ Not confirmed |
| `execform`    | 2     | Paint form          | ⬜ Not confirmed |

## Page Device & Output Operators

| Operator            | Level | Description                       | Status          |
| ------------------- | :---: | --------------------------------- | --------------- |
| `showpage`          | 1     | Output and reset current page     | 🟩 Implemented   |
| `copypage`          | 1     | Output current page without reset | ⬜ Not confirmed |
| `setpagedevice`     | 2     | Install page device               | ⬜ Not confirmed |
| `currentpagedevice` | 2     | Return page-device dictionary     | ⬜ Not confirmed |
| `nulldevice`        | 1     | Install no-output device          | ⬜ Not confirmed |

## Font, Glyph & Text Operators

| Operator          | Level | Description                                              | Status          |
| ----------------- | :---: | -------------------------------------------------------- | --------------- |
| `definefont`      | 1     | Register font/CIDFont                                    | ⬜ Not confirmed |
| `composefont`     | 3     | Create/register composite font from CMap and descendants | ⬜ Not confirmed |
| `undefinefont`    | 2     | Remove Font resource                                     | ⬜ Not confirmed |
| `findfont`        | 1     | Find Font resource                                       | 🟩 Implemented   |
| `scalefont`       | 1     | Scale font                                               | 🟩 Implemented   |
| `makefont`        | 1     | Transform font                                           | 🟩 Implemented   |
| `setfont`         | 1     | Set current font                                         | 🟩 Implemented   |
| `rootfont`        | 2     | Return root font                                         | ⬜ Not confirmed |
| `currentfont`     | 1     | Return current font                                      | ⬜ Not confirmed |
| `selectfont`      | 2     | Find/transform/set font                                  | 🟩 Implemented   |
| `show`            | 1     | Paint string                                             | 🟩 Implemented   |
| `ashow`           | 1     | Show with constant displacement                          | 🟩 Implemented   |
| `widthshow`       | 1     | Show with extra displacement for selected character      | 🟩 Implemented   |
| `awidthshow`      | 1     | Combine ashow and widthshow                              | 🟩 Implemented   |
| `xshow`           | 2     | Show with per-glyph x displacements                      | ⬜ Not confirmed |
| `xyshow`          | 2     | Show with per-glyph xy displacements                     | ⬜ Not confirmed |
| `yshow`           | 2     | Show with per-glyph y displacements                      | ⬜ Not confirmed |
| `glyphshow`       | 2     | Show glyph by name/CID                                   | ⬜ Not confirmed |
| `stringwidth`     | 1     | Measure string advance                                   | 🟩 Implemented   |
| `cshow`           | 2     | Call procedure for mapped characters                     | ⬜ Not confirmed |
| `kshow`           | 1     | Call procedure between shown characters                  | 🟩 Implemented   |
| `findencoding`    | 1     | Find encoding vector                                     | ⬜ Not confirmed |
| `setcachedevice`  | 1     | Declare cached glyph metrics                             | ⬜ Not confirmed |
| `setcachedevice2` | 2     | Declare cached glyph metrics for composite/CID behavior  | ⬜ Not confirmed |
| `setcharwidth`    | 1     | Declare uncached glyph width                             | ⬜ Not confirmed |
| `addglyph`        | 2     | Add glyph to bitmap font                                 | ⬜ Not confirmed |

## Interpreter Parameters & Cache Operators

| Operator              | Level | Description                            | Status          |
| --------------------- | :---: | -------------------------------------- | --------------- |
| `setsystemparams`     | 2     | Set system-wide interpreter parameters | ⬜ Not confirmed |
| `currentsystemparams` | 2     | Return system parameters               | ⬜ Not confirmed |
| `setuserparams`       | 2     | Set per-context parameters             | ⬜ Not confirmed |
| `currentuserparams`   | 2     | Return user parameters                 | ⬜ Not confirmed |
| `setdevparams`        | 2     | Set I/O device parameters              | ⬜ Not confirmed |
| `currentdevparams`    | 2     | Return I/O device parameters           | ⬜ Not confirmed |
| `vmreclaim`           | 2     | Control garbage collector              | ⬜ Not confirmed |
| `setvmthreshold`      | 2     | Set VM GC threshold                    | ⬜ Not confirmed |
| `vmstatus`            | 1     | Report VM status                       | ⬜ Not confirmed |
| `cachestatus`         | 1     | Report font cache                      | ⬜ Not confirmed |
| `setcachelimit`       | 1     | Set max cached glyph size              | ⬜ Not confirmed |
| `setcacheparams`      | 2     | Set font cache parameters              | ⬜ Not confirmed |
| `currentcacheparams`  | 2     | Return font cache parameters           | ⬜ Not confirmed |
| `setucacheparams`     | 2     | Set user-path cache params             | ⬜ Not confirmed |
| `ucachestatus`        | 2     | Report user-path cache                 | ⬜ Not confirmed |

## LanguageLevel 3 Procedure-Set Operators / Procedures

| Operator              | Level | Description                                  | Status          |
| --------------------- | :---: | -------------------------------------------- | --------------- |
| `beginbfchar`         | 3     | CIDInit: begin bfchar mappings               | ⬜ Not confirmed |
| `endbfchar`           | 3     | CIDInit: end bfchar mappings                 | ⬜ Not confirmed |
| `beginbfrange`        | 3     | CIDInit: begin bfrange mappings              | ⬜ Not confirmed |
| `endbfrange`          | 3     | CIDInit: end bfrange mappings                | ⬜ Not confirmed |
| `begincidchar`        | 3     | CIDInit: begin CID character mappings        | ⬜ Not confirmed |
| `endcidchar`          | 3     | CIDInit: end CID character mappings          | ⬜ Not confirmed |
| `begincidrange`       | 3     | CIDInit: begin CID range mappings            | ⬜ Not confirmed |
| `endcidrange`         | 3     | CIDInit: end CID range mappings              | ⬜ Not confirmed |
| `begincmap`           | 3     | CIDInit: begin CMap                          | ⬜ Not confirmed |
| `endcmap`             | 3     | CIDInit: end CMap                            | ⬜ Not confirmed |
| `begincodespacerange` | 3     | CIDInit: begin code-space ranges             | ⬜ Not confirmed |
| `endcodespacerange`   | 3     | CIDInit: end code-space ranges               | ⬜ Not confirmed |
| `beginnotdefchar`     | 3     | CIDInit: begin notdef character mappings     | ⬜ Not confirmed |
| `endnotdefchar`       | 3     | CIDInit: end notdef character mappings       | ⬜ Not confirmed |
| `beginnotdefrange`    | 3     | CIDInit: begin notdef ranges                 | ⬜ Not confirmed |
| `endnotdefrange`      | 3     | CIDInit: end notdef ranges                   | ⬜ Not confirmed |
| `beginrearrangedfont` | 3     | CIDInit: begin rearranged font               | ⬜ Not confirmed |
| `endrearrangedfont`   | 3     | CIDInit: end rearranged font                 | ⬜ Not confirmed |
| `beginusematrix`      | 3     | CIDInit: begin use-matrix section            | ⬜ Not confirmed |
| `endusematrix`        | 3     | CIDInit: end use-matrix section              | ⬜ Not confirmed |
| `StartData`           | 3     | CIDInit: begin font/CMap data                | ⬜ Not confirmed |
| `usefont`             | 3     | CIDInit: select component font               | ⬜ Not confirmed |
| `usecmap`             | 3     | CIDInit: inherit/use another CMap            | ⬜ Not confirmed |
| `removeall`           | 3     | BitmapFontInit: remove all Type 32 glyphs    | ⬜ Not confirmed |
| `removeglyphs`        | 3     | BitmapFontInit: remove Type 32 glyph range   | ⬜ Not confirmed |
| `settrapparams`       | 3     | Trapping ProcSet: set trapping parameters    | ⬜ Not confirmed |
| `currenttrapparams`   | 3     | Trapping ProcSet: return trapping parameters | ⬜ Not confirmed |
| `settrapzone`         | 3     | Trapping ProcSet: define trapping zone       | ⬜ Not confirmed |

## WaavScript / Non-PLRM Extensions Present in the Supplied Registry

| Operator     | Level | Description                                    | Status        |
| ------------ | :---: | ---------------------------------------------- | ------------- |
| `max`        | ext   | Maximum of two values                          | 🟩 Implemented |
| `min`        | ext   | Minimum of two values                          | 🟩 Implemented |
| `writable`   | ext   | Make writable (not a standard PLRM operator)   | 🟩 Implemented |
| `executable` | ext   | Make executable (not a standard PLRM operator) | 🟩 Implemented |


## Additional Standard LanguageLevel 3 Procedure-Set Procedures

| Operator / Procedure | Level | Description                                                   | Status          |
| -------------------- | :---: | ------------------------------------------------------------- | --------------- |
| `GetHalftoneName`    | 3     | Color-rendering ProcSet helper for halftone selection         | ⬜ Not confirmed |
| `GetPageDeviceName`  | 3     | Color-rendering ProcSet helper for page-device identification | ⬜ Not confirmed |
| `GetSubstituteCRD`   | 3     | Color-rendering ProcSet helper for substitute CRD selection   | ⬜ Not confirmed |

## Device / `statusdict` Compatibility Operators

These are standardized compatibility names documented by Adobe but are device- or product-dependent and are not normally the first operators an interpreter needs for general graphics. They are included here so the registry can serve as a broader Level 1–3 checklist.

| Operator             | Level | Description                       | Status          |
| -------------------- | :---: | --------------------------------- | --------------- |
| `letter`             | 2     | Select letter-size media          | ⬜ Not confirmed |
| `lettersmall`        | 2     | Select small-letter media         | ⬜ Not confirmed |
| `legal`              | 2     | Select legal-size media           | ⬜ Not confirmed |
| `ledger`             | 2     | Select ledger-size media          | ⬜ Not confirmed |
| `11x17`              | 2     | Select 11×17 media                | ⬜ Not confirmed |
| `a4`                 | 2     | Select A4 media                   | ⬜ Not confirmed |
| `a3`                 | 2     | Select A3 media                   | ⬜ Not confirmed |
| `a4small`            | 2     | Select small-A4 media             | ⬜ Not confirmed |
| `b5`                 | 2     | Select B5 media                   | ⬜ Not confirmed |
| `note`               | 2     | Select note-size media            | ⬜ Not confirmed |
| `lettertray`         | 2     | Select letter tray                | ⬜ Not confirmed |
| `legaltray`          | 2     | Select legal tray                 | ⬜ Not confirmed |
| `ledgertray`         | 2     | Select ledger tray                | ⬜ Not confirmed |
| `a3tray`             | 2     | Select A3 tray                    | ⬜ Not confirmed |
| `a4tray`             | 2     | Select A4 tray                    | ⬜ Not confirmed |
| `b5tray`             | 2     | Select B5 tray                    | ⬜ Not confirmed |
| `11x17tray`          | 2     | Select 11×17 tray                 | ⬜ Not confirmed |
| `sccbatch`           | 2     | Query SCC batch mode              | ⬜ Not confirmed |
| `sccinteractive`     | 2     | Query SCC interactive mode        | ⬜ Not confirmed |
| `setsccbatch`        | 2     | Set SCC batch mode                | ⬜ Not confirmed |
| `setsccinteractive`  | 2     | Set SCC interactive mode          | ⬜ Not confirmed |
| `duplexmode`         | 2     | Query duplex mode                 | ⬜ Not confirmed |
| `firstside`          | 2     | Query first-side state            | ⬜ Not confirmed |
| `newsheet`           | 2     | Begin a new sheet                 | ⬜ Not confirmed |
| `setduplexmode`      | 2     | Set duplex mode                   | ⬜ Not confirmed |
| `settumble`          | 2     | Set tumble mode                   | ⬜ Not confirmed |
| `tumble`             | 2     | Query tumble mode                 | ⬜ Not confirmed |
| `devdismount`        | 2     | Dismount device                   | ⬜ Not confirmed |
| `devformat`          | 2     | Format device                     | ⬜ Not confirmed |
| `devmount`           | 2     | Mount device                      | ⬜ Not confirmed |
| `devstatus`          | 2     | Query device status               | ⬜ Not confirmed |
| `devforall`          | 2     | Enumerate devices                 | ⬜ Not confirmed |
| `accuratescreens`    | 2     | Query accurate-screen mode        | ⬜ Not confirmed |
| `checkscreen`        | 2     | Check screen parameters           | ⬜ Not confirmed |
| `pagemargin`         | 2     | Query page margin                 | ⬜ Not confirmed |
| `pageparams`         | 2     | Query page parameters             | ⬜ Not confirmed |
| `setaccuratescreens` | 2     | Set accurate-screen mode          | ⬜ Not confirmed |
| `setpage`            | 3     | Compatibility page setup          | ⬜ Not confirmed |
| `setpagemargin`      | 2     | Set page margin                   | ⬜ Not confirmed |
| `setpageparams`      | 3     | Set compatibility page parameters | ⬜ Not confirmed |
| `appletalktype`      | 2     | Query AppleTalk type              | ⬜ Not confirmed |
| `buildtime`          | 2     | Return interpreter build time     | ⬜ Not confirmed |
| `byteorder`          | 2     | Return native byte order          | ⬜ Not confirmed |
| `checkpassword`      | 2     | Check system password             | ⬜ Not confirmed |
| `defaulttimeouts`    | 2     | Return default timeout values     | ⬜ Not confirmed |
| `diskonline`         | 2     | Set/query disk online state       | ⬜ Not confirmed |
| `diskstatus`         | 2     | Return disk status                | ⬜ Not confirmed |
| `manualfeed`         | 2     | Query manual-feed mode            | ⬜ Not confirmed |
| `manualfeedtimeout`  | 2     | Query manual-feed timeout         | ⬜ Not confirmed |
| `margins`            | 2     | Query device margins              | ⬜ Not confirmed |
| `mirrorprint`        | 2     | Query mirror-print mode           | ⬜ Not confirmed |
| `pagecount`          | 2     | Return page count                 | ⬜ Not confirmed |
| `pagestackorder`     | 2     | Query page stack order            | ⬜ Not confirmed |
| `printername`        | 2     | Return printer name               | ⬜ Not confirmed |
| `processcolors`      | 2     | Return process color information  | ⬜ Not confirmed |
| `sethardwareiomode`  | 2     | Set hardware I/O mode             | ⬜ Not confirmed |
| `setjobtimeout`      | 2     | Set job timeout                   | ⬜ Not confirmed |
| `setpagestockorder`  | 2     | Set page stock order              | ⬜ Not confirmed |
| `setprintername`     | 2     | Set printer name                  | ⬜ Not confirmed |
| `setresolution`      | 2     | Set output resolution             | ⬜ Not confirmed |
| `doprinterrors`      | 2     | Query printer-error reporting     | ⬜ Not confirmed |
| `dostartpage`        | 2     | Query start-page behavior         | ⬜ Not confirmed |
| `hardwareiomode`     | 2     | Query hardware I/O mode           | ⬜ Not confirmed |
| `initializedisk`     | 2     | Initialize disk                   | ⬜ Not confirmed |
| `jobname`            | 2     | Return current job name           | ⬜ Not confirmed |
| `jobtimeout`         | 2     | Query job timeout                 | ⬜ Not confirmed |
| `ramsize`            | 2     | Return RAM size                   | ⬜ Not confirmed |
| `realformat`         | 2     | Return real-number representation | ⬜ Not confirmed |
| `resolution`         | 2     | Return output resolution          | ⬜ Not confirmed |
| `setdefaulttimeouts` | 2     | Set default timeout values        | ⬜ Not confirmed |
| `setdoprinterrors`   | 2     | Set printer-error reporting       | ⬜ Not confirmed |
| `setdostartpage`     | 2     | Set start-page behavior           | ⬜ Not confirmed |
| `setdosysstart`      | 2     | Set system-start behavior         | ⬜ Not confirmed |
| `setuserdiskpercent` | 2     | Set user disk percentage          | ⬜ Not confirmed |
| `softwareiomode`     | 2     | Query software I/O mode           | ⬜ Not confirmed |
| `userdiskpercent`    | 2     | Query user disk percentage        | ⬜ Not confirmed |
| `waittimeout`        | 2     | Query wait timeout                | ⬜ Not confirmed |
| `setsoftwareiomode`  | 2     | Set software I/O mode             | ⬜ Not confirmed |
| `dosysstart`         | 2     | Query system-start behavior       | ⬜ Not confirmed |
| `emulate`            | 2     | Select/query emulation            | ⬜ Not confirmed |
| `setmargins`         | 2     | Set device margins                | ⬜ Not confirmed |
| `setmirrorprint`     | 2     | Set mirror-print mode             | ⬜ Not confirmed |

## Standard Names / Objects Often Mistaken for Operators

These names are important to an interpreter but are not all operators in the same sense as the tables above.

| Name                  | Kind                     | Status            |
| --------------------- | ------------------------ | ----------------- |
| `systemdict`          | dictionary               | 🟩 Implemented     |
| `userdict`            | dictionary               | 🟩 Implemented     |
| `globaldict`          | dictionary               | ⬜ Not confirmed   |
| `statusdict`          | dictionary               | 🟩 Implemented     |
| `errordict`           | dictionary               | 🟩 Implemented     |
| `$error`              | dictionary               | 🟩 Implemented     |
| `FontDirectory`       | dictionary               | ⬜ Not confirmed   |
| `GlobalFontDirectory` | dictionary               | ⬜ Not confirmed   |
| `StandardEncoding`    | array                    | 🟩 Implemented     |
| `ISOLatin1Encoding`   | array                    | ⬜ Not confirmed   |
| `UserObjects`         | array                    | ⬜ Not confirmed   |
| `handleerror`         | error-handling procedure | 🟩 Implemented     |

## Coverage Summary (core + standard ProcSet registry)

- Standard operator/procedure names in this registry: **364**
- Confirmed implemented: **138**
- Explicitly marked not implemented in your supplied registry: **16**
- Added but not yet confirmed either way: **210**

The coverage count above covers the core/system operator tables plus the main standard ProcSet table generated before the compatibility appendix; the device/statusdict compatibility appendix is intentionally not folded into that count. The count is conservative: an unconfirmed operator may already exist in WaavScript but was not listed in the supplied registry and has not been explicitly confirmed in our recent work.
