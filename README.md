# 16-bit 8086 Assembler
## Introducing
This assembler was a project aimed at deeply understanding the compilation process.
And also a rather entertaining process of developing an approach for compiling processor commands into machine instructions.
You can use it to compiler AT&T 8086 assembler sources in executable formats for DOS systems.

# Features
The compiler receives .asm files as input (AT&T syntax) and generates executable files in .com or .exe format as output,
depending on the specified parameters. It was also intended to create its own object format, but at the moment this remains unimplemented.
Errors are output during compilation. When using parameters,
```
show-ast
show-link
show-seg
show-all
```
you can get AST output, a list of symbols to link, or segment data.
There are also simple optimizations for evaluating expressions at compile time.

### Details
To compile assembly sources it's creates AST (see src/syntax), the code generator then traverses the tree and generates machine code.
Machine code generation is based on selection the most optimal existing processor instruction followed by parameter coding.
Not all possible instructions have been added (see src/arch/8086/arch-8086.cpp), if necessary, you can add the missing instructions there yourself. To do this, you need to know the opcode, parameter types, and encoding type for the instruction (use Intel® 64 and IA-32 Architectures Software Developer Manual or AMD64 Architecture Programmer's Manual).

# Build
You can build it using GNU C++ compiler, just install it and run Makefile target 'linux' or
use MinGW to build it for windows (target 'windows'). I hope it also will works with clang compiler.
```
make linux
make windows
```

# Testing
With the target 'dos' in Makefile you can run DosBox (sure if you are alredy has it on your machine) that automaticly mount current directory to 'D' drive, so you can place some compiled
files there and run them. But before sure if all environment paths variables are seted correctly in Makefile.
