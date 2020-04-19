# The Duality Programming Language

[![](https://github.com/tpuschel/duality/workflows/CI/badge.svg)](https://github.com/tpuschel/duality/actions?workflow=CI)

Duality has no dependencies, no configuration step and is intentionally created with just one translation unit (duality.c).

Thus, all that is needed to build Duality is a C11<sup>*</sup> compiler; no build system necessary!

Your system's equivalent of ```cc duality.c -o duality``` will do the trick. Of course, add any flags you want for optimizations etc.

In the root of the project is a compile_flags.txt and .clang-format for use with [clangd](https://clangd.llvm.org/installation.html).

<sup>*</sup>The only C11-specific features used are anonymous unions and _Alignof.
