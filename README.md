# The Duality Programming Language

[![](https://github.com/tpuschel/duality/workflows/CI/badge.svg)](https://github.com/tpuschel/duality/actions?workflow=CI)

(This is pre-alpha software; It might not even build at any time.)

Homepage: https://duality-lang.org.

## How to build

In theory, duality should compile on any Windows or Posix platform with a C99<sup>*</sup> compiler.

In practice however, the build system only supports a limited set of compilers and platforms.

<sup>*</sup>Also required is support for unnamed unions/structs, technically a C11 feature,
but available in pretty much all C99 compilers.

### General requirements
- [CMake](https://cmake.org) is the meta build system. Only the *latest release* of CMake is ever supported.

##### Windows-specific requirements
An installation of Visual Studio with these components is required:

- Static Analysis Tools
- Windows 10 SDK
- Visual Studio C++ core features
- VC++ 2017 Version 15.7 v14.14 latest v141 tools
- Windows Universal CRT SDK
- Windows Universal C Runtime

These are the components required to build on Windows 10, 64-bit using Visual Studio 2017.
Adjust accordingly for your version of Windows and Visual Studio.

#### Configuring
To configure a build, simply run CMake:
```
cmake -S <path to source dir> -B <path to build dir; will be created if it doesn't exist>
```
This will pick a default generator for your platform. See the [list of supported generators](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html).

To choose a compiler, set the environment variable CC to whatever compiler you want to use before invoking CMake.

CMake by default builds static libraries. If you want to build a shared one, pass the flag ```-D BUILD_SHARED_LIBS=ON``` to CMake.

To generate a ```compile_commands.json``` file for use with language servers, use the ```-D CMAKE_EXPORT_COMPILE_COMMANDS=ON``` flag.

#### Building
To actually build, either invoke the underlying generator or use CMake:
```
cmake --build <path to build dir>
```

#### Installing
CMake generates install targets that the underlying generator can build.

Alternatively, use CMake:
```
cmake --install <path to build dir>
```

Installation may require administrative privileges.

##### Example
As an example, here's how to build and install a shared library, using gcc, in debug configuration, with ninja as the generator, while being in the same directory as the CMakeLists.txt:
```
CC=gcc cmake -S . -B build -G Ninja -D BUILD_SHARED_LIBS=ON -D CMAKE_BUILD_TYPE=Debug

cmake --build build

sudo cmake --install build
```