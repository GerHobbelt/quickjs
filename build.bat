rem @echo off
set clang=F:\workspace\LLVM16\bin\clang-cl.exe /MD /O2 /Ob2 /Ot -D_CRT_NONSTDC_NO_WARNINGS -DCONFIG_VERSION=\"2021-03-27\" -D_CRT_SECURE_NO_WARNINGS
%clang% -c libbf.c -o libbf.o
%clang% -c libregexp.c -o libregexp.o
%clang% -c libunicode.c -o libunicode.o
%clang% -c cutils.c -o cutils.o
%clang% -c quickjs.c -o quickjs.o -DEMSCRIPTEN=1
%clang% -c quickjs-libc.c -o quickjs-libc.o
%clang% -c getopt.c -o getopt.o
%clang% -c gettimeofday.c -o gettimeofday.o
set objects=libregexp.o libunicode.o quickjs.o quickjs-libc.o cutils.o getopt.o gettimeofday.o
%clang% qjsc.c %objects% -o qjsc.exe
rem .\qjsc.exe -c -o repl.c -m repl.js
rem %clang% -c repl.c -o repl.o
rem %clang% qjs.c %objects% repl.o -o qjs.exe
rem %clang% /LD -o examples/point.so %objects% examples/point.c
rem %clang% /LD -o examples/fib.so %objects% examples/fib.c -DJS_SHARED_LIBRARY
rem del *.o
pause