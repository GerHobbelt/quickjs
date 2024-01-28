if not exist b mkdir b
cd b
rem cmake -G "Visual Studio 16 2019" ..
cmake ..
cmake --build . --target qjsc --config Release

cd ..
set qjsc=".\build\Release\qjsc.exe"
%qjsc% -c -o repl.c -m repl.js
rem %qjsc% -c -o qjscalc.c qjscalc.js

cd b
cmake ..
cmake --build . --target qjs --config Release

cd ..
pause
