# The Duality Programming Language

[![](https://github.com/tpuschel/duality/workflows/CI/badge.svg)](https://github.com/tpuschel/duality/actions?workflow=CI)

## Building

Duality has no dependencies, no configuration step and is intentionally created with just one
translation unit (duality.c).

Thus, all that is needed to build Duality is a C99<sup>*</sup> compiler; no build system necessary!

Your system's equivalent of ```cc duality.c -o duality``` will do the trick. Of course, add any
flags you want for optimizations etc.

In the root of the project is a compile_flags.txt and .clang-format for use with [clangd](https://clangd.llvm.org/installation.html).

<sup>*</sup>Also required is support for anonymous unions, technically a C11 feature, but
available in pretty much all C99 compilers.

## Documentation

Each of the subfolders in this repository has it own README detailing what's implemented by that folder.

/core/ - The core calculus of Duality. Basically the heart of the language.

/dap/ - Support for the [Debug Adapter Protocol](https://microsoft.github.io/debug-adapter-protocol/).

/lsp/ - Support for the [Language Server Protocol](https://microsoft.github.io/language-server-protocol/).

/support/ - Auxiliary data structures and functions.

/syntax/ - Provides the AST, parser and transformation from AST to Core.

/vscode-ext/ - Source of [the Visual Studio Code extension](https://marketplace.visualstudio.com/items?itemName=puschel.duality).
