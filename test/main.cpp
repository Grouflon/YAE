#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "lib.h"

#include <emscripten.h>

int main(int _argc, char** _argv)
{
    printf("Hello World blabla!\n");
    
    helloLib();

    {
        void* handle = dlopen("lib2.wasm", RTLD_NOW);    
        void (*helloLib2_ptr)() = (void (*)())dlsym(handle, "helloLib2");
        helloLib2_ptr();
        dlclose(handle);
    }
    

    return EXIT_SUCCESS;
}
