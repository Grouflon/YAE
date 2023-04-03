YAE
*yet another engine*

## Dependencies
LLVM/Clang 13.0.0
emscripten

## Installation
- Add {emsdk}\upstream\emscripten to your PATH directory

## Comments
- Static memory that leads to memory allocation does not fit well with the engine's philosophy. In an ideal world, no allocation should ever been made until the main starts.