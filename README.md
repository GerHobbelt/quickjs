# quickjs
a fork of QuickJS, ported to Code::Blocks plus some changes:
- Removed default std file IO.
- No Multithreading.
- LibUV added and Timer function and File read/write are implimented.
- No Console interepter.
- JS Compiler and Interepter are mixed into main.c.
- Changes in file structure

###### still is under maintance and more features will be added soon

## Why?

Personally, I love Python and hate JS. 
Python is more readable, makes things easier to solve, on the other hand, 
JS syntax is quite bad especially "JS callback hell".

But my only choice as a C developer is to use a "scripting" language 
with an easy C interface, have a small codebase, easy to compile 
with no dependencies and acceptable performance.

If you have looked at the CPython you'll know that it matches non of my needs.
Developing a library in Python is more difficult, 
although at least I haven't found a good QuickJS documentation.





## TODO:
- Using multiple search path to Import module
- storing compiled files into a zip file
- LibUV Socket implimentation
