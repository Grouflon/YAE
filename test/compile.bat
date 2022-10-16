call emcc lib.cpp -std=c++14 -fPIC -c -o lib.o
call emcc lib.o -sSIDE_MODULE=1 -sLLD_REPORT_UNDEFINED -o  lib.wasm

call emcc lib2.cpp -std=c++14 -fPIC -c -o lib2.o
call emcc lib2.o -sSIDE_MODULE=1 -sLLD_REPORT_UNDEFINED -o  lib2.wasm

call emcc main.cpp -std=c++14 -fPIC -c -o main.o
call emcc main.o lib.wasm -sMAIN_MODULE=1 -sLLD_REPORT_UNDEFINED --preload-file ./lib2.wasm -o main.html

@echo Fini!