waavsps

This is an implementation of a Postscript interpreter written in C++.  The backend for the graphics rendering uses the blend2d library.

This project was inspired by a previoius Postscript interpreter written in Lua: https://github.com/Wiladams/lj2ps

There was also a subset (without graphics) implemented in asim: https://github.com/Wiladams/asim

For the current implementation, 'inspired by' just means taking a lot of the learnings from the previous implementations, as the code itself is different enough that it's not an easy translation.  The goal here is to have an even smaller, faster implementation that is as portable as the underlying blend2d library.  Trying to be as feature complete as possible, as the world of Postscript files is fairly finite, and not really expanding.

The list of operators involved in the Postscript language is long.  The current implementation state can be found here: https://github.com/Wiladams/waavscript/blob/main/docs/registryofops.md

