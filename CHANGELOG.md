# Changelog

Notable changes follow the [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) format.

## [0.1.0] - 2026-07-01

### Added

- CRC engine with runtime `ucrc_model_t` descriptor. One-shot API (`ucrc_compute`) and streaming API (`ucrc_begin` / `ucrc_update` / `ucrc_finish`). Supports CRC-8, CRC-16, and CRC-32.
- Compile-time strategy selection via `UCRC_STRATEGY`: bitwise, nibble table, and byte table. The bitwise engine is always present as the fallback for models without tables.
- Predefined models: `ucrc_crc16_ccitt_false`, `ucrc_crc32_iso_hdlc`, and `ucrc_crc8_smbus` (enabled with `-DUCRC_ENABLE_CRC8=1`).
- `ucrc_platform.h` detects 8-bit and 16-bit MAU targets (`ucrc_octet_t`). Use `UCRC_SIMULATE_16BIT_MAU` to test the 16-bit-MAU path on a host.
- `ucrc_conf.h` defines width enables, strategy selection, and the overridable `UCRC_ASSERT` precondition trap.
- Committed predefined-table source (`src/ucrc_tables.c`) with a native generator (`tools/gen_tables.c`). A `regen-tables` Meson target and a `tables-up-to-date` freshness test guard the file.
- Test matrix covers every strategy, the 16-bit-MAU simulation, and an assertions-disabled build. All tests verify against published CRC `check` constants.
- MISRA C:2012 hygiene: `misch` (cppcheck + `misra.py` addon) analyses clean. Two advisory deviations are recorded in `misra-deviations.txt`: rule 15.5 (single point of exit) is deviated project-wide for guard-clause style. Rule 8.7 (external linkage referenced in one translation unit) applies at `ucrc_compute`, which only consumer units call.
