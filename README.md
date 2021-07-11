Requirements
============

Tested with Clang 12.0.0.

On Windows, requires pthreads-win32 to be installed as a CMake package. See
[here](https://github.com/rleathart/pthreads-win32-CMake) for details.

Cloning and Building
====================

```bash
git clone https://github.com/rleathart/ChessEngine --recursive

cmake -B build
cmake --build build
```

Note: You may wish to set `git config submodule.recurse true` to make checkouts with submodules easier.
