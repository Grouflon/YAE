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
- Hot-Reloading a module that has been linked by another module: I do believe that this is possible if we have knowledge of all the module dependencies. We'll then have to unload the whole chain of dependent modules back and forth which is probably not a big deal. By doing that we could benefit havin a core module that never reloads and a yae module that could be reloaded, but this module will only contain Resources and Renderers children and I don't know if it is worth the trouble right now. Let's see in the future if systems get decoupled from application, or if different submodules have to talk between each other.

## Useful links
- Android native build: https://hereket.github.io/posts/android_from_command_line/