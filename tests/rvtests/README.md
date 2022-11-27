# RISC-V Tests

This directory contains the tests used for RISC-V.
The tests are split into three categories:

- Unit tests
- System tests
- Running C code

To run all automated RISC-V tests (not running C code), run `make rvtest` in the root directory of MSIM (but beware the dependencies stated later).

## Unit tests

The unit tests are located in the `unit-tests` directory.
They require the [PCUT](https://github.com/vhotspur/pcut) framework to work.

## System

The system tests are located in every subdirectory of this directory, except for `unit-tests` and `execute_c`.
Each test directory consists of the source code in RISC-V assembler (`main.S`), the build script (`build.sh`), the assembled machine code(`main.bin`), the dissasembled output (`main.dis`), the MSIM configuration file (`msim.conf`) and the expected output of the test (`expected-output.txt`).
Sometimes, there can be extra files (like trap handlers or data files).
The machine code is present, to allow the execution of these tests without having the riscv gcc toolchain installed.

To run all system tests, run `./run_tests.py` in this directory.

## Running C code

The compilation and link scripts together with bootloaders for compiling C code into RISC-V machine code that can be run on MSIM can be found in the `execute_c` directory.
There is a `Makefile` script written for compilation of the code, but for the same reason as with system tests, the compiled code is also present in this directory.

## Compilation of tests from sources

To compile the machine code from the RISC-V assembly or C, you will need the [RISC-V GNU Compiler Toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain).

MSIM is built for `RV32IMA`.
The `gcc` toolchain needs to be configured with the right architecture when building.

```shell
./configure --prefix=... --with-arch=rv32ima
make
```
