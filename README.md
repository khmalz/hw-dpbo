# Homework for OOP course

## Prerequisites

* C++ Compiler (GCC, Clang, or MSVC)
* **Meson** (Version 1.9.x or newer is recommended)
* **Ninja**

---

## How to Build

Once all prerequisites are installed, building the project is straightforward:

### 1. Set Up The Build Directory
This command only needs to be run once. It creates the `build` directory and detects your compiler and dependencies.

```bash
meson setup build
```

### 2\. Compile The Project

To compile (or re-compile after making changes), run:

```bash
ninja -C build
```

The final executable will be located inside the `build` directory.

3. Run executable file
```bash
./build/app 
```

-----

##  Installing & Upgrading Meson Version

For detailed instructions on how to install & upgrade Meson on Windows and Linux, 
please see the **[Installation Guide (INSTALL.md)](INSTALL.md)**.