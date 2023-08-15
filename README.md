YAE
*yet another engine*

## Dependencies
Ruby/Rake
LLVM/Clang 13.0.0
emscripten

## Installation
### Build system
- Install ruby from https://rubyinstaller.org/downloads/
- Install rake by running `gem install rake` from the command line

## Comments
- Static memory that leads to memory allocation does not fit well with the engine's philosophy. In an ideal world, no allocation should ever been made until the main starts.

## Notes
- If we want to do dynamic patching of function pointers, for instance when we will be using delegates, we'll probably need DbgHelp. see the doc here: https://learn.microsoft.com/en-us/windows/win32/debug/using-dbghelp.
- ResourceManager exists at Application/Window level and not at Program level because most graphical resources are coupled to one specific instance of the renderer.