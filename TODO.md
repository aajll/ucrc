# TODO

## Goal

Rewrite the C source-code comments and Doxygen annotations in STE-flavored English.

## Tasks

### 1. Review and rewrite maintained comments

- [x] Rewrite comments and annotations in `include/`, `src/ucrc.c`, `tests/`, and `tools/gen_tables.c`.
- [x] Keep each source-code comment line within the 80-column limit from `.clang-format`.
- [x] Do not change C code, identifiers, command syntax, or Markdown files.

### 2. Regenerate generated source

- [x] Update generator-owned comment text as needed and regenerate `src/ucrc_tables.c`.
- [x] Verify that the generated table file is current.

### 3. Verify the change

- [x] Attempt to run `clang-format` only on modified maintained C and header files. No executable is installed, so comment line lengths were checked instead.
- [x] Build and run the Meson test suite.
- [x] Confirm that this work did not modify Markdown files.

## Discovered Tasks

- [x] Rewrite the Doxygen comments in `config/ucrc_version.h.in`, which generates the installed version header.
- [x] Reconfigure `build_tests` because its Meson build data is incompatible with the installed Meson version. Then rerun table generation.
- [x] Find an installed versioned `clang-format` executable. If none exists, verify comment line lengths without formatting.

## Notes

- `src/ucrc_tables.c` is generated. Do not edit it directly.
- Existing changes in `README.md`, `CONTRIBUTING.md`, and `CHANGELOG.md` are outside this work and must remain untouched.
- Markdown has no manual line-length limit.
