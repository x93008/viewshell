# ViewShell

This library is used to load front-end web pages by calling an existing browser engine, similar to tauri.

# build

```
# install dependencies
conan install . -b missing

# requires CMake version 3.26 or higher.
cmake --preset conan-debug --fresh

# build
cmake --build --preset conan-debug

```
