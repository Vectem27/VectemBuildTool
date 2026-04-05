# TODO

## Compiler

- [ ] Adding compiler abstraction writen in json or lua (has to be defined) to allow customisation from config file. Is can be different for some platforms. If this will be implemented in lua, the lua can write the command themself and give it to the program.
- [ ] Add platform management for cross compile even from a different target platform.
- [ ] Add ability to copy directory inside the output build.
- [ ] Add external libraries link
- [ ] Add flags into the build command
- [ ] Inject the platform var into scripts
- [ ] Add compilation flag into the target
- [x] Add precompiled modules (With lib or dll and include dir)
- [ ] Add file ignore (Especialy for platform management)
- [ ] Let compile a module as a system module

## Sub unit compilation

- [ ] Add sub units compilation to allow plugins systems.
- [ ] Allow dynamic library sub units to be dynamically loaded.

## Optimisation

- [ ] Adding file changes check to avoid recompilation.
