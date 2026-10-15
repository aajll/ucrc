# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Added

- CI cross-builds the library with TI C2000 CGT 22.6.2 and 25.11.1 under the COFF and EABI object formats, with warnings as errors.
- CI runs MISRA C:2023 analysis with `misch` and fails on any finding not covered by `misra-deviations.txt`.
- `scripts/format.sh`, which pins the clang-format release and runs it with `uvx`. CI checks formatting through it.

### Changed

- `ucrc.h` defines `UCRC_VERSION_MAJOR`, `_MINOR`, `_PATCH`, `UCRC_VERSION`, `UCRC_VERSION_STRING` and a new `UCRC_VERSION_NUMBER` (`long`, `MAJOR * 10000 + MINOR * 100 + PATCH`), so a copied-in header carries its version. Meson fails to configure when the header and `project(version:)` disagree, and the build needs Meson 0.57 or later.
- The README is a short landing page; installation, the API reference and the design record are in `docs/integration.md`, `docs/api.md` and `docs/design.md`, which replace the Sphinx site. A `ucrc docs examples` test compiles every documented C example.
- CI installs Meson, Ninja and gcovr with `uv` instead of `pip`.
- The coverage gate is 100 % line and 87 % branch, the coverage the suite already reaches.

### Removed

- `ucrc_version.h`. `ucrc.h` defines the version macros.

## [0.1.0] - 2026-07-01

### Added

- Initial CRC engine: runtime `ucrc_model_t` descriptor with one-shot (`ucrc_compute`) and streaming (`ucrc_begin` / `ucrc_update` / `ucrc_finish`) APIs for CRC-8, CRC-16, and CRC-32.
- Compile-time computation strategy (`UCRC_STRATEGY`): bitwise, half-byte (nibble) table, and byte-wise table; the bitwise engine is always present as the fallback for untabulated (custom) models.
- Predefined models: `ucrc_crc16_ccitt_false`, `ucrc_crc32_iso_hdlc`, and `ucrc_crc8_smbus` (enabled with `-DUCRC_ENABLE_CRC8=1`).
- `ucrc_platform.h`: 8-/16-bit MAU detection (`ucrc_octet_t`), with `UCRC_SIMULATE_16BIT_MAU` for host testing of the 16-bit-MAU path.
- `ucrc_conf.h`: width enables, strategy selection, and the overridable `UCRC_ASSERT` precondition trap.
- Committed predefined-table source (`src/ucrc_tables.c`) with a native generator (`tools/gen_tables.c`), a `regen-tables` Meson target, and a `tables-up-to-date` freshness test.
- Test matrix: every strategy, the 16-bit-MAU simulation, and an assertions-disabled defensive-path build, all verified against the published CRC `check` constants.
- MISRA C:2012 hygiene: the library source analyses clean under `misch` (cppcheck + the `misra.py` addon), with two advisory deviations recorded in `misra-deviations.txt`: rule 15.5 (single point of exit) deviated project-wide as a deliberate guard-clause house style, and rule 8.7 (external linkage referenced in one translation unit) at the `ucrc_compute` public entry point, which only consumer translation units call.
