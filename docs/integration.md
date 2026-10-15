# Integration guide

This guide shows how to add `ucrc` to a project, configure it, build it, and run its tests. The API is described in [api.md](api.md), and the models and strategies in [design.md](design.md).

## Requirements

- A C11 compiler with `uint16_t` and `uint32_t`. The library uses `_Static_assert` and, by default, `<assert.h>`.
- For the tests: Meson, Ninja, and Python 3. zlib is optional; when it is found, an extra test compares CRC-32 with zlib's `crc32()`.

## Copy-in

Copy-in is the recommended method for embedded targets. Copy five files into your source tree:

```
include/ucrc.h
include/ucrc_conf.h
include/ucrc_platform.h
src/ucrc.c
src/ucrc_tables.c
```

Compile both source files with the same configuration macros as every file that includes the header, then include it:

```c
#include "ucrc.h"
```

`src/ucrc_tables.c` is committed, so no code generation runs at build time. `ucrc_version.h` exists only in Meson builds.

## Meson subproject

Put the repository in `subprojects/ucrc`, as a checkout or through a wrap file, then declare the dependency:

```meson
ucrc_dep = dependency('ucrc', fallback: ['ucrc', 'ucrc_dep'])
```

The build calls `meson.override_dependency('ucrc', ...)`, so other subprojects in the same build find `ucrc` by name. Tests are off by default, so a subproject build compiles only the library. Include the header as `"ucrc.h"`.

## Installed package

```sh
meson setup build --buildtype=release --prefix=/usr/local
meson compile -C build
meson install -C build
cc app.c $(pkg-config --cflags --libs ucrc) -o app
```

The headers install to a `ucrc/` subdirectory, and the `pkg-config` flags add that directory to the include path:

<!-- snippet: skip -->
```c
#include "ucrc.h"
#include "ucrc_version.h"
```

## Configuration

Every option is a macro with a default in `ucrc_conf.h`. Define it with `-D` for the library sources and every file that includes `ucrc.h`, so the model declarations and the compiled tables agree.

| Macro | Default | Effect |
| --- | --- | --- |
| `UCRC_ENABLE_CRC8` | `0` | Compiles `ucrc_crc8_smbus` and its tables. |
| `UCRC_ENABLE_CRC16` | `1` | Compiles `ucrc_crc16_ccitt_false` and its tables. |
| `UCRC_ENABLE_CRC32` | `1` | Compiles `ucrc_crc32_iso_hdlc` and its tables. |
| `UCRC_STRATEGY` | `UCRC_STRATEGY_BYTE` | `UCRC_STRATEGY_BITWISE`, `UCRC_STRATEGY_NIBBLE` or `UCRC_STRATEGY_BYTE`. |
| `UCRC_ASSERT(expr)` | `assert(expr)` | Precondition trap. Redefine it to call a supervisor, or to `((void)0)` to rely on the defensive returns alone. |

The width enables control only the predefined models. An application-defined model of any width works in every configuration. Strategy sizes and speeds are compared in [design.md](design.md#strategies).

A Meson build takes the macros through `c_args`, for example:

```sh
meson setup build "-Dc_args=-DUCRC_STRATEGY=UCRC_STRATEGY_NIBBLE -DUCRC_ENABLE_CRC8=1"
```

## Build and test

```sh
# Library only
meson setup build --buildtype=release
meson compile -C build

# Every strategy, the 16-bit-MAU simulation, the assertions-disabled build,
# the zlib comparison, the table freshness check, and the documentation test
meson setup build --buildtype=debug -Dbuild_tests=true
meson compile -C build
meson test -C build --verbose
```

Regenerate `src/ucrc_tables.c` with `meson compile -C build regen-tables` after changing a predefined model. Sanitiser, coverage, formatting, and MISRA commands are in [CONTRIBUTING.md](../CONTRIBUTING.md).

## Cross builds

The consuming firmware owns its cross file, linker command file, and memory placement. No cross file ships with this project.

CI compiles the library for an Arm Cortex-M0 with freestanding flags, and with warnings as errors using two releases of a vendor C11 compiler for a 16-bit-MAU DSP target, where `CHAR_BIT` is 16, under both of its object formats. This covers the source dialect and type assumptions only. Linking, execution, and the final map-file footprint remain checks for the consumer.

To exercise the 16-bit-MAU path on a host, define `UCRC_SIMULATE_16BIT_MAU`; the test suite builds one variant this way. Production builds leave it undefined and rely on detection.
