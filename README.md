# ucrc

Lean CRC-8/16/32 library for embedded, RTOS, and Linux targets.

`ucrc` (micro cyclic redundancy check) computes CRCs over a stream of 8-bit octets. The CRC _model_ (polynomial, init, reflection, xor-out) is a runtime descriptor, so one image can use different CRC variants for different jobs. The computation _strategy_ (bitwise, half-byte table, or byte-wise table) is chosen once at compile time, so you pay only for the size and speed trade-off you need. It is written in C11, allocates nothing, and holds no global mutable state.

## Features

- **Three widths**: CRC-8, CRC-16, CRC-32, each independently enabled.
- **Runtime model descriptor**: choose the polynomial, init, and reflection at the call site via `ucrc_model_t`; define your own model or use a predefined one.
- **Predefined standards**: CRC-16/CCITT-FALSE, CRC-32/ISO-HDLC (zlib/Ethernet), and CRC-8/SMBUS.
- **Compile-time strategy**: bitwise (no tables), nibble (16-entry tables), or byte-wise (256-entry tables). Predefined-model tables live in flash, not RAM.
- **One-shot and streaming**: `ucrc_compute`, or `ucrc_begin`/`ucrc_update`/`ucrc_finish` for chunked data. Reentrant, with the caller owning the running value.
- **16-bit-MAU aware**: builds and runs on the TI C2000 family (F28379D) where `CHAR_BIT` is 16, with a bit-identical result to 8-bit-MAU peers.
- **Safety-first boundaries**: no undefined behaviour on bad arguments, plus an overridable `UCRC_ASSERT` contract trap.

## Requirements

- A C11 compiler (uses `_Static_assert`).
- Meson and Ninja to build the bundled tests. The library itself is two source files and three headers and can be copied in directly.

## Quick Start

```c
#include <stddef.h>
#include <stdint.h>
#include "ucrc.h"

/* One-shot CRC-32 over a buffer (matches zlib's crc32). */
uint32_t crc = ucrc_compute(&ucrc_crc32_iso_hdlc, data, len);

/* Streaming CRC-16 over chunks that arrive over time. */
uint32_t running = ucrc_begin(&ucrc_crc16_ccitt_false);
running = ucrc_update(&ucrc_crc16_ccitt_false, running, chunk_a, len_a);
running = ucrc_update(&ucrc_crc16_ccitt_false, running, chunk_b, len_b);
uint16_t result = (uint16_t)ucrc_finish(&ucrc_crc16_ccitt_false, running);
```

### Defining a custom model

Any model whose `table` is `NULL` is computed by the bitwise engine, so no table is needed:

```c
/* CRC-16/MODBUS: poly 0x8005, init 0xFFFF, reflected. */
static const ucrc_model_t modbus = {
        .poly = 0x8005u,
        .init = 0xFFFFu,
        .xorout = 0x0000u,
        .table = NULL,
        .width = 16u,
        .refin = true,
        .refout = true,
};

uint16_t crc = (uint16_t)ucrc_compute(&modbus, data, len);
```

To find the exact parameters for an external peer, consult the CRC RevEng catalogue (reveng.sourceforge.io / crccalc.com) and match poly/init/refl/xorout.

## Installation

### Copy-in (recommended for embedded targets)

Copy `include/ucrc.h`, `include/ucrc_conf.h`, `include/ucrc_platform.h`, `src/ucrc.c`, and `src/ucrc_tables.c` into your project. No build-time code generation is required: `src/ucrc_tables.c` is committed and ready to compile.

```c
#include "ucrc.h"
```

### Meson subproject

```meson
ucrc_dep = dependency('ucrc', fallback: ['ucrc', 'ucrc_dep'])
```

The build also calls `meson.override_dependency('ucrc', ...)` so downstream Meson builds resolve the subproject by name.

### Installed dependency

When installed system-wide, include the namespaced header and discover the package with `pkg-config`:

```c
#include <ucrc/ucrc.h>
```

## Building

```sh
# Library only (release)
meson setup build --buildtype=release -Dbuild_tests=false
meson compile -C build

# With unit tests (all strategies plus the simulated 16-bit MAU)
meson setup build --buildtype=debug -Dbuild_tests=true
meson compile -C build
meson test -C build --verbose
```

## Configuration

Set these before including `ucrc.h`, or at the toolchain level with `-D`.

| Macro               | Default              | Meaning                                                    |
| ------------------- | -------------------- | ---------------------------------------------------------- |
| `UCRC_ENABLE_CRC16` | `1`                  | Compile the CRC-16 model and tables.                       |
| `UCRC_ENABLE_CRC32` | `1`                  | Compile the CRC-32 model and tables.                       |
| `UCRC_ENABLE_CRC8`  | `0`                  | Compile the CRC-8 model and tables.                        |
| `UCRC_STRATEGY`     | `UCRC_STRATEGY_BYTE` | `..._BITWISE`, `..._NIBBLE`, or `..._BYTE`.                |
| `UCRC_ASSERT(expr)` | `assert(expr)`       | Precondition trap; redirect to a safety handler if wanted. |

## Strategy trade-offs

| Strategy | Table ROM (CRC-16 / CRC-32) | Relative speed |
| -------- | --------------------------- | -------------- |
| Bitwise  | 0 B / 0 B                   | baseline       |
| Nibble   | 32 B / 64 B                 | ~2 to 4x       |
| Byte     | 512 B / 1 KiB               | ~8x            |

Predefined-model tables are `static const` (flash/`.rodata`), so they cost no RAM. The bitwise engine is always present as the fallback for untabulated (custom) models, regardless of the selected strategy.

## API Reference

Every model is passed by pointer, and every call returns a `uint32_t` masked to the model width (cast to `uint16_t` or `uint8_t` as appropriate). Full per-function contracts (`@pre`/`@post`) live in `ucrc.h` and render into the Doxygen/Sphinx API reference under `docs/`.

### One-shot

```c
uint32_t ucrc_compute(const ucrc_model_t *model, const ucrc_octet_t *data, size_t len);
```

Computes the complete CRC over `len` octets, applying the model's `init` and `xorout`. A `len` of 0 is valid and returns the model's empty-message CRC. On a 16-bit-MAU target only the low 8 bits of each `data` element are used.

### Streaming

```c
uint32_t ucrc_begin(const ucrc_model_t *model);
uint32_t ucrc_update(const ucrc_model_t *model, uint32_t crc, const ucrc_octet_t *data, size_t len);
uint32_t ucrc_finish(const ucrc_model_t *model, uint32_t crc);
```

`ucrc_begin` returns the initial running register. `ucrc_update` folds a chunk of octets into the running register and returns it; a `len` of 0 is a no-op. `ucrc_finish` applies `refout` and `xorout` and returns the result. The running value is carried by the caller and the functions hold no internal state, so several independent streams can run concurrently, each with its own model. A one-shot result equals a streamed result for any chunk boundaries.

### The model descriptor

```c
typedef struct {
        uint32_t poly;     /* Generator polynomial, normal form        */
        uint32_t init;     /* Initial register value (catalogue form)  */
        uint32_t xorout;   /* Final XOR mask                           */
        const void *table; /* Strategy lookup table, or NULL = bitwise */
        uint8_t width;     /* CRC width in bits: 8, 16, or 32          */
        bool refin;        /* Reflect input octets when true           */
        bool refout;       /* Reflect output register when true        */
} ucrc_model_t;
```

`ucrc_model_t` is a plain aggregate. The predefined models (`ucrc_crc16_ccitt_false`, `ucrc_crc32_iso_hdlc`, and `ucrc_crc8_smbus` when CRC-8 is enabled) are `const` instances carrying a table that matches the build strategy. For a custom model, set `table` to `NULL` and the bitwise engine handles any polynomial. Table strategies require `refin == refout`; the bitwise engine supports them independently.

## TI C2000 / 16-bit-MAU notes

On the C2000 family the minimum addressable unit is 16 bits and `uint8_t` aliases `uint16_t`. ucrc consumes one logical octet per addressable unit and uses only the low 8 bits of each, so:

- The CRC is bit-identical to an 8-bit-MAU peer for the same octet stream.
- `len` always counts octets.
- Where two octets are genuinely packed into one 16-bit word, decompose them into one-octet-per-word first. The sibling `ppack` primitive is the intended tool for that; ucrc does not guess.

Set `-DUCRC_SIMULATE_16BIT_MAU` on a normal host to exercise this path under the unit tests.

## Use Cases

- **Inter-processor comms (IPC)**: frame integrity between firmware components.
- **Flash / EEPROM integrity**: detect corruption of stored images or records, verifiable from a host tool (CRC-32/ISO-HDLC matches zlib).
- **Serial / fieldbus framing**: CRC-16 variants for SCI, SPI, CAN payloads.

## Limitations

- CRC detects accidental corruption, not tampering. It is not cryptographic.
- Table strategies require `refin == refout`; the bitwise engine supports them independently. All predefined models satisfy this.
- No hardware-CRC-peripheral backend yet (pure software).

## Notes

| Topic                | Note                                                                                                                                                                                   |
| -------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Table provenance** | `src/ucrc_tables.c` is generated by `tools/gen_tables.c` and committed. Regenerate with `meson compile regen-tables`; the `tables-up-to-date` test guards freshness.                   |
| **Verification**     | Tests run every strategy and the 16-bit-MAU simulation against the published CRC `check` constants, cross-check CRC-32 against zlib, and fuzz against an independent reference engine. |
| **Design**           | See `docs/design/PROJECT_DESIGN.md` for the full rationale.                                                                                                                            |
