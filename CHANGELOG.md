# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Added

- Initial CRC engine: runtime `ucrc_model_t` descriptor with one-shot (`ucrc_compute`) and streaming (`ucrc_begin` / `ucrc_update` / `ucrc_finish`) APIs for CRC-8, CRC-16, and CRC-32.
- Compile-time computation strategy (`UCRC_STRATEGY`): bitwise, half-byte (nibble) table, and byte-wise table; the bitwise engine is always present as the fallback for untabulated (custom) models.
- Predefined models: `ucrc_crc16_ccitt_false`, `ucrc_crc32_iso_hdlc`, and `ucrc_crc8_smbus` (enabled with `-DUCRC_ENABLE_CRC8=1`).
- `ucrc_platform.h`: 8-/16-bit MAU detection (`ucrc_octet_t`), with `UCRC_SIMULATE_16BIT_MAU` for host testing of the 16-bit-MAU path.
- `ucrc_conf.h`: width enables, strategy selection, and the overridable `UCRC_ASSERT` precondition trap.
- Committed predefined-table source (`src/ucrc_tables.c`) with a native generator (`tools/gen_tables.c`), a `regen-tables` Meson target, and a `tables-up-to-date` freshness test.
- Test matrix: every strategy, the 16-bit-MAU simulation, and an assertions-disabled defensive-path build, all verified against the published CRC `check` constants.
