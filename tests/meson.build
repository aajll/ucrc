# Unit tests. The suite is split into category modules that share a common
# harness (test_harness.h/.c) and support helpers (test_support.h/.c). It is
# built once per computation strategy and once with the 16-bit-MAU simulation,
# so every code path is verified against the published CRC `check` constants
# and an independent reference engine. All variants enable CRC-8 for coverage.

test_common_args = ['-DUCRC_ENABLE_CRC8=1']

# Shared harness/support plus the category modules and the driver.
test_sources = files(
  'test_harness.c',
  'test_support.c',
  'test_known_answers.c',
  'test_streaming.c',
  'test_boundaries.c',
  'test_fuzz.c',
  'test_main.c',
)

strategies = [
  ['bitwise', 'UCRC_STRATEGY_BITWISE'],
  ['nibble', 'UCRC_STRATEGY_NIBBLE'],
  ['byte', 'UCRC_STRATEGY_BYTE'],
]

foreach s : strategies
  variant_args = test_common_args + ['-DUCRC_STRATEGY=' + s[1]]

  variant_lib = static_library(
    'ucrc_test_' + s[0],
    library_sources,
    include_directories: [public_headers, generated_headers],
    c_args: variant_args,
  )

  variant_exe = executable(
    'test_ucrc_' + s[0],
    test_sources,
    link_with: variant_lib,
    include_directories: [public_headers, generated_headers],
    c_args: variant_args,
  )

  test('ucrc ' + s[0], variant_exe)
endforeach

# 16-bit-MAU simulation against the default (byte) strategy.
sim_args = test_common_args + [
  '-DUCRC_STRATEGY=UCRC_STRATEGY_BYTE',
  '-DUCRC_SIMULATE_16BIT_MAU',
]

sim_lib = static_library(
  'ucrc_test_sim',
  library_sources,
  include_directories: [public_headers, generated_headers],
  c_args: sim_args,
)

sim_exe = executable(
  'test_ucrc_sim',
  test_sources,
  link_with: sim_lib,
  include_directories: [public_headers, generated_headers],
  c_args: sim_args,
)

test('ucrc byte 16-bit-MAU', sim_exe)

# Assertions-disabled variant: exercises the production no-UB defensive
# returns (NULL arguments) that would otherwise trip UCRC_ASSERT.
noassert_args = test_common_args + [
  '-DUCRC_STRATEGY=UCRC_STRATEGY_BYTE',
  '-DUCRC_ASSERT(expr)=((void)0)',
  '-DUCRC_TEST_NO_ASSERT=1',
]

noassert_lib = static_library(
  'ucrc_test_noassert',
  library_sources,
  include_directories: [public_headers, generated_headers],
  c_args: noassert_args,
)

noassert_exe = executable(
  'test_ucrc_noassert',
  test_sources,
  link_with: noassert_lib,
  include_directories: [public_headers, generated_headers],
  c_args: noassert_args,
)

test('ucrc byte assertions-disabled', noassert_exe)

# Cross-check the CRC-32 model against zlib's reference crc32(), when zlib is
# available on the build host (skipped gracefully otherwise).
zlib_dep = dependency('zlib', required: false)
if zlib_dep.found()
  zlib_lib = static_library(
    'ucrc_test_zlib',
    library_sources,
    include_directories: [public_headers, generated_headers],
    c_args: test_common_args,
  )

  zlib_exe = executable(
    'test_ucrc_zlib',
    files('test_harness.c', 'test_support.c', 'test_zlib.c'),
    link_with: zlib_lib,
    dependencies: [zlib_dep],
    include_directories: [public_headers, generated_headers],
    c_args: test_common_args,
  )

  test('ucrc vs zlib crc32', zlib_exe)
endif

# The committed table file must match what the generator produces.
test(
  'tables-up-to-date',
  gen_tables,
  args: ['--check', committed_tables],
)
