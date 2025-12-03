# MSIM change log

All notable changes to MSIM will be documented in this file
(though changes before v2.0.0 are often incomplete).

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).


## Unreleased

### Fixed

* Use proper breakpoint size (see #96, @ceresek)

### Added

* Missing documentation for `dlcd` (see #92, @rosenbergm)
* Installation notes for Arch (see #92, , @rosenbergm)
* Basic DAP debugging support (@0xVector)

### Changed

### Deprecated

### Removed


## v3.0.1 - 2025-10-09

### Fixed

* Enable instruction tracing with `-t` on RISC-V 32 (see #93)


## v3.0.0 - 2025-09-26

### Fixed

* Wrong toolchain link in mini-kernel tutorial (see #73, @HanyzPAPU)
* Wrong header guard (see #74, @HanyzPAPU, @vhotspur)
* Document `-n` option (see #78, @PeterHero)
* Segfault on MacOS (see #90, @MatusJurcak)

### Added

* Support for RISC-V 64 (see #81, #82, #83, #84, @rosenbergm)
* `dlcd` LCD display device (@rosenbergm)
* Added Mini-kernel multiplatform tutorial (see #70, @HanyzPAPU)
* RISC-V virtual memory commands tutorial (see #70, @HanyzPAPU)
* CI builds on MacOS (see #76, #77, @vhotspur)
* DEB packages CI built updates (@vhotspur)

### Changed

* Tutorial reorganization (see #70, @vhotspur)


## v2.3.1 - 2024-09-25

### Fixed

 - Wrong formatting of virtual address in the result of `tr` and `str` (see #71, @HanyzPAPU)
 - Propagate error when ddisk cannot write to a file (see #63, @PeterHero)


## v2.3.0 - 2024-09-12

### Fixed

* wrong `break` documentation (see #55 and #56, @KronwarsCZ, @vhotspur)
* RISC-V executing from paged memory in M-mode (see #66, @HanyzPAPU)

### Added

* setup C style (via `.clang-format`) (see #51, @vhotspur)
* CI checks C style as well as file encoding and line-endings (see #51, @vhotspur)
* completion for `add` prints available devices (see #31, @vhotspur)
* page table dumping commands `ptd` and `sptd` (see #67, @HanyzPAPU)
* virtual address translation commands `tr` and `str` (see #67, @HanyzPAPU)

### Changed

* renamed RISC-V CPU commands (see #60, @HanyzPAPU)
   * `csrrd` is `csrd`, `tlbrd` is `tlbd`
* `csrd` dumps selected registers only (see #61, @HanyzPAPU)


## v2.2.1 - 2023-10-09

### Fixed

* ebuild recipe (thx MFF sysadmins)
* RISC-V TLB documentation (@vhotspur, @HanyzPAPU)
* ReadTheDocs configuration (@vhotspur)

### Changed

* `EBREAK` instruction on RISC-V halts simulation without TTY (as is done on MIPS) (@vhotspur)
* move changelog to a more structured format (@vhotspur)


## v2.2.0 - 2023-09-25

### Added

* `dnomem` debugging device (breaks simulation on memory access to its area) (@vhotspur)


## v2.1.2 - 2023-09-22

### Changed

* more automated tests (@vhotspur)
* various CI improvements (@vhotspur)


## v2.1.1 - 2023-09-22

### Fixed

* broken device clean-up (see #44, @vhotspur)

### Changed

* more CI automation (@vhotspur)


## v2.1.0 - 2023-09-19

### Added

* simulated TLB for RISC-V (Jan Papesch <janpapuch@seznam.cz>)

### Changed

* switch documentation to read-the-docs (@vhotspur)


## v2.0.0 - 2022-10-07

### Added

* support for RISC-V (Jan Papesch <janpapuch@seznam.cz>)


## v1.4.2 - 2020-11-02

### Changed

* halt simulation when `XINT` invoked without TTY (@vhotspur)


## v1.4.1 - 2020-10-27

### Added

* add DCRV instruction for CP0 register dump (@vhotspur)
* reintroduce DVAL instruction (@vhotspur)

### Changed

* improved CP0 register dump (@vhotspur)


## v1.4.0 - 2012-??-??

### Changed

* improve command line processing code (Martin Decky <decky@d3s.mff.cuni.cz>)
* code consolidation (Martin Decky <decky@d3s.mff.cuni.cz>)
* port fixes from the 1.3 branch (Martin Decky <decky@d3s.mff.cuni.cz>)


## v1.3.8 - 2010-10-05

### Fixed

* `TLBWI` implementation (Jan Zaloha and Vlastimil Babka)

### Added

* support for `DESTDIR` into `Makefile.in` (@vhotspur)
* resurrect GDB support, better device management and various other improvements (Tomas Martinec)


## v1.3.7.1 - 2009-11-02

### Added

* Gentoo ebuild and RPM spec file (Martin Decky <decky@d3s.mff.cuni.cz>)


## v1.3.7 - 2009-02-02

### Added

* official reference manual in doc/reference.html (Martin Decky <decky@d3s.mff.cuni.cz>)


## v1.3.6 - 2009-01-19

### Fixed

* compilation in Cygwin (Martin Decky <decky@d3s.mff.cuni.cz>)
* use EPC of previous branch instruction if TLB exception on code fetch happens inside a branch delay slot (Ondrej Cerny)
* memory access alignment (Martin Decky <decky@d3s.mff.cuni.cz>)


## v1.3.5.1 - 2008-10-31

### Fixed

* fix console input in MinGW (Martin Decky <decky@d3s.mff.cuni.cz>)


## v1.3.5 - 2008-10-26

### Added

* initial support for building in MinGW (Martin Decky <decky@d3s.mff.cuni.cz>)
* code and data breakpoints (Martin Decky <decky@d3s.mff.cuni.cz>)
* support WATCH exception (thx Lubomir Bulej)


## v1.3.4 - 2008-09-24

### Added

* add GPL license (Martin Decky <decky@d3s.mff.cuni.cz>)

### Changed

* source cleanup (Martin Decky <decky@d3s.mff.cuni.cz>)
* simplify build system (Martin Decky <decky@d3s.mff.cuni.cz>)
* split sources into multiple directories (Martin Decky <decky@d3s.mff.cuni.cz>)


## v1.3.4rc2 - 2008-06-05

### Fixed

* LL-SC tracking fix (Jiri Svoboda)


## v1.3.4rc1 - 2007-12-17

### Fixed

* dorder sychdown command (Andrej Krutak)
* badvaddr now set for all exceptions (Viliam Holub <holub@d3s.mff.cuni.cz>)

### Changed

* ddisk and mem clean-up (Milan Burda)
* dtime now reads usec as well (Viliam Holub <holub@d3s.mff.cuni.cz>)
* minor performance improvements (Viliam Holub <holub@d3s.mff.cuni.cz>)


## v1.3.3 - 2007-11-30

### Fixed

* error messages (Viliam Holub <holub@d3s.mff.cuni.cz>)
* CPU number (Viliam Holub <holub@d3s.mff.cuni.cz>)

### Added

* `dorder` has two registers now - enable/disable interrupt pending (Viliam Holub <holub@d3s.mff.cuni.cz>)


## v1.3.2.3 - 2007-11-23

### Fixed

* `dorder` infinite loop (David Matousek)


## v1.3.2.2 - 2007-11-21

### Fixed

* several small fixes (Martin Decky <decky@d3s.mff.cuni.cz>)

## v1.3.2.1 - 2007-03-11

### Fixed

* missing `stdio.h` in `input.c` fixed (Viliam Holub <holub@d3s.mff.cuni.cz>)
* `MULT` implementation (thx Ondrej Palkovsky <ondrap@penguin.cz>)
* `u_int64_t` -> `uint_64` (Viliam Holub <holub@d3s.mff.cuni.cz>)
* `autogen.sh` added (Viliam Holub <holub@d3s.mff.cuni.cz>)

### Changed

* `goto` has been renamed to `continue` (Viliam Holub <holub@d3s.mff.cuni.cz>)


## v1.3.1 - 2006-10-09

### Changed

* `dprintf` renamed to `mprintf` to avoid conflicts with libc (Martin Decky <decky@d3s.mff.cuni.cz>)
* keybord keycode register is cleared after read operation; this allows to read new keycodes even with disabled interrupts (Ondrej Palkovsky <ondrap@penguin.cz>)


## v1.3 - 2005-10-18

### Fixed

* TLB Modification exception (Jakub Jermar <jermar@itbs.cz>)
* LWL, LWR, SWL, SWR byte counting fix by (Ondrej Palkovsky <ondrap@penguin.cz>)
* MTC0 fix (Ondrej Palkovsky <ondrap@penguin.cz>)
* various small issues (Lubomir Bulej)

### Added

* add 4kc support (Ondrej Palkovsky <ondrap@penguin.cz>)

### Changed

* error messages cleanup (Viliam Holub <holub@d3s.mff.cuni.cz>)
* code polishing (Viliam Holub <holub@d3s.mff.cuni.cz>)


## v1.2.18 - 2005-01-08

### Fixed

* various fixes of `ERET`, `LBU` and `EPC` rewrite (Matej Pivoluska, Martin Horvath, Jakub Kotrla)

### Added

* the first version of TAB completion (Viliam Holub <holub@d3s.mff.cuni.cz>)
* safe versions of `malloc` and `strdup` (Viliam Holub <holub@d3s.mff.cuni.cz>)

### Changed

* all system and device command are now defined via structures


## v1.2.12 - 2004-03-23

### Added

* generic context help (Viliam Holub <holub@d3s.mff.cuni.cz>)

### Changed

* device command are now generic (Viliam Holub <holub@d3s.mff.cuni.cz>)


## v1.2.3 - 2003-12-17

### Fixed

* non-POSIX terminal attributes are now checked (Viliam Holub <holub@d3s.mff.cuni.cz>)
* `dcpu md` command fixed (Viliam Holub <holub@d3s.mff.cuni.cz>)
* endians checked via autoconf (Viliam Holub <holub@d3s.mff.cuni.cz>)

### Changed

* cycle counter size increased to long long (Viliam Holub <holub@d3s.mff.cuni.cz>)
* code cleanup (comments, read/write functions) (Viliam Holub <holub@d3s.mff.cuni.cz>)


## v1.2.2 - 2003-12-01

### Fixed

* TLB dirty mask (read-only pages were writable) (Viliam Holub <holub@d3s.mff.cuni.cz>)


## v1.2.1 - 2003-11-28

### Fixed

* MTLO and MTHI fixes (Viliam Holub <holub@d3s.mff.cuni.cz>)


## v1.2 - 2003-11-24

### Fixed

* various debug messages (Viliam Holub <holub@d3s.mff.cuni.cz>)

### Added

* added `dtime` device (Viliam Holub <holub@d3s.mff.cuni.cz>)
