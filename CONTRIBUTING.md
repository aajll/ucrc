# Contributing to ucrc

ucrc is a CRC-8/16/32 library for embedded firmware, RTOS, and Linux targets. It has no dependencies. The design supports audited codebases and safety-relevant projects. Contributions must meet that bar.

## Getting started

Run the same commands locally that CI runs:

```sh
# Configure with tests + sanitisers (CI default). The suite builds every
# strategy (bitwise / nibble / byte), the 16-bit-MAU simulation, and an
# assertions-disabled variant.
meson setup build --buildtype=debug -Dbuild_tests=true \
                  -Db_sanitize=address,undefined
meson compile -C build
meson test -C build --verbose

# Coverage (CI gate is 80% line + 70% branch)
meson setup build_cov --buildtype=debug -Dbuild_tests=true -Db_coverage=true
meson compile -C build_cov && meson test -C build_cov
gcovr --root . --filter 'src/' --filter 'include/' --print-summary
```

## Source style

- Run `clang-format -i` on every modified `.c` and `.h` file before submitting. CI enforces the format. The project uses `.clang-format`.
- Use 8-space indent, Linux brace style, and an 80-column limit. Match existing conventions. Do not reformat unrelated code.
- Do **not** run `clang-format` on `src/ucrc_tables.c`. The generator creates this file, and it carries a `clang-format off` guard. The freshness test compares exact bytes.
- Meson is the single source of truth for the build. Update `meson.build` and `tests/meson.build` when adding or removing source files. Do not introduce CMake, Make, or other build systems.

## C language rules

- Use only C11 features. The library uses `_Static_assert`.
- Use fixed-width types from `<stdint.h>` and `<stdbool.h>`. Never use plain `int` for values that must be a specific width.
- No heap allocation (`malloc`, `free`, VLAs), no recursion, and no global mutable state.
- Guard pointers in public functions. Treat `len == 0` as a no-op. Report contract violations through `UCRC_ASSERT`. An integrator may redirect this macro to a safety handler.
- A build with `UCRC_ASSERT` disabled must still behave correctly for every input.

## Generated tables

- The generator creates `src/ucrc_tables.c` and the file is committed. Do not hand-edit it.
- Edit `tools/gen_tables.c` to change the predefined model list or table maths. Then configure with `-Dbuild_tests=true` and run `meson compile regen-tables`.
- Generator output must stay deterministic. Remove timestamps and keep stable ordering. The `tables-up-to-date` test fails if the committed file is stale.
- The file is committed on purpose. Copy-in consumers and non-Meson build systems do not run Meson. A cross build cannot run a native generator. Do not convert it to a build-time `custom_target`.

## Strategy and width matrix

- `UCRC_STRATEGY` (bitwise / nibble / byte) selects one engine at compile time. The bitwise engine is **always** compiled as the fallback for models with `table == NULL`. Keep it that way.
- Any change to the engine must pass every strategy, the 16-bit-MAU simulation, and every width-enable combination.
- On 16-bit-MAU targets only the low 8 bits of each `ucrc_octet_t` are used. Always mask octets and table indices with `& 0xFFu` (or `& 0xFu` for nibbles).

## Tests and coverage

- Add a test for every bug fix and new feature.
- Check any new CRC behaviour against a published `check` constant or the independent reference engine (`ucrc_test_ref`). Never check it against ucrc alone.
- In test code, never use `sizeof(array)` as an element count. The type `ucrc_octet_t` is `uint16_t` under the 16-bit-MAU simulation, so `sizeof` returns twice the count. Use an explicit length instead.
- Fuzz seeds are fixed so failures reproduce from the test name alone. Do not add non-deterministic tests.
- CI enforces an 80% line and 70% branch coverage gate. New code without tests fails this check.

## MISRA C:2023

The library uses fixed-width unsigned types, explicit `u` suffixes, no heap, no recursion, no `errno`, `static const` tables, and no undefined-behaviour shifts. The deviation record lives in the file header of `src/ucrc.c`. If your change introduces a new deviation, add an entry there (rule, sites, justification). Flag the deviation in the pull-request description. A certified static analyser is required for full tool-driven compliance; this repository does not vendor one.

## Commits

Use Conventional Commits:

- `feat: ...` new feature
- `fix: ...` bug fix
- `doc: ...` documentation only
- `test: ...` test-only changes
- `chore: ...` build, CI, release work
- `refactor: ...` code change that neither fixes a bug nor adds a feature

Keep the subject under 70 characters. Use the body to explain why the change is needed, not what the diff shows.

## Pull requests

- Open an issue first for non-trivial changes. Agree on the design before writing code.
- Keep PRs focused. One feature or one fix per PR.
- All CI checks must pass: tests on Linux and macOS under ASan+UBSan, the release build, the coverage gate, the format check, the docs build, and the cross-compile smoke.
- Flag any new MISRA deviation in the PR description.

## When in doubt

Open an issue and discuss before writing code. The library is small enough that even modest design changes affect consumers that depend on wire-compatible CRC values.
