# ucrc

[![CI](https://github.com/aajll/ucrc/actions/workflows/ci.yml/badge.svg)](https://github.com/aajll/ucrc/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

**CRC-8, CRC-16 and CRC-32 over octet streams, with a runtime model and a compile-time strategy, for embedded C.**

## Quick start

Copy `include/ucrc.h`, `include/ucrc_conf.h`, `include/ucrc_platform.h`, `src/ucrc.c` and `src/ucrc_tables.c` into your project. For a Meson subproject or an installed `pkg-config` package, see the [integration guide](docs/integration.md).

```c
#include "ucrc.h"

/* One-shot CRC-32, equal to zlib's crc32(). */
uint32_t
image_crc(const ucrc_octet_t *image, size_t len)
{
        return ucrc_compute(&ucrc_crc32_iso_hdlc, image, len);
}

/* Streaming CRC-16 over a frame that arrives in two chunks. */
uint16_t
frame_crc(const ucrc_octet_t *head, size_t head_len, const ucrc_octet_t *body,
          size_t body_len)
{
        uint32_t running = ucrc_begin(&ucrc_crc16_ccitt_false);

        running = ucrc_update(&ucrc_crc16_ccitt_false, running, head, head_len);
        running = ucrc_update(&ucrc_crc16_ccitt_false, running, body, body_len);
        return (uint16_t)ucrc_finish(&ucrc_crc16_ccitt_false, running);
}
```

## Description

`ucrc` computes a CRC over a stream of 8-bit octets. The model (polynomial, initial value, reflection and final XOR) is a runtime descriptor, so one image can use several CRC variants. The computation strategy (bitwise, 16-entry table or 256-entry table) is chosen once at compile time.

- CRC-8, CRC-16 and CRC-32, each enabled independently
- Predefined CRC-16/CCITT-FALSE, CRC-32/ISO-HDLC and CRC-8/SMBUS models, and application-defined models for any polynomial
- One-shot and streaming APIs; the caller holds the running value, so independent streams need no locking
- Tables are `const` and cost no RAM; the bitwise engine needs no table at all
- Identical results where `CHAR_BIT` is 16, with one octet per `ucrc_octet_t`
- 100% line coverage, sanitiser-clean tests against the published check values and zlib, and MISRA C:2023 analysis with [`misch`](https://pypi.org/project/misch/)

A CRC detects accidental corruption, not tampering. It is not a cryptographic check.

## Validated toolchains

| Toolchain               | Target                        | Validation                                       |
| ----------------------- | ----------------------------- | ------------------------------------------------ |
| GCC, Clang              | Linux hosts                   | Unit, fuzz and sanitiser tests for every strategy |
| GCC                     | Arm Cortex-M0, freestanding   | Library compile                                  |
| DSP vendor C11 compiler | 16-bit MAU (`CHAR_BIT == 16`) | Library compile, two object formats              |

Target linking and execution belong to the consuming firmware. See [cross builds](docs/integration.md#cross-builds).

## Documentation

- [Integration guide](docs/integration.md) — copy-in, Meson subproject, installed package, configuration, and cross builds
- [API reference](docs/api.md) — every function and model, with its rules
- [Design](docs/design.md) — models, strategies, tables, octets on wide-byte targets, and verification
- [Changelog](CHANGELOG.md) — release history

## Contributing

Read [CONTRIBUTING.md](CONTRIBUTING.md) before you open a pull request. Report security issues as described in [SECURITY.md](.github/SECURITY.md).

## License

MIT. See [LICENSE](LICENSE).
