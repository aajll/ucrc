project(
  'ucrc',
  'c',
  version: '0.1.0',
  meson_version: '>=0.57.0',
  default_options: [
    'c_std=c11',
    'warning_level=3',
  ],
  license: 'MIT',
)

project_description = 'Lean CRC-8/16/32 library for embedded, RTOS, and Linux targets'

# === Version ==================================================================
# include/ucrc.h defines the version macros, so a copy-in build has them.
# Configuration fails when they differ from the project version.

fs = import('fs')
version_parts = meson.project_version().split('.')
version_macros = {
  'UCRC_VERSION_MAJOR': version_parts[0],
  'UCRC_VERSION_MINOR': version_parts[1],
  'UCRC_VERSION_PATCH': version_parts[2],
  'UCRC_VERSION': '"' + meson.project_version() + '"',
}
header_lines = []
foreach line : fs.read('include' / 'ucrc.h').split('\n')
  header_lines += ' '.join(line.split())
endforeach
foreach name, value : version_macros
  if '#define @0@ @1@'.format(name, value) not in header_lines
    error('include/ucrc.h must define @0@ as @1@'.format(name, value))
  endif
endforeach

# === Include directories ======================================================

public_headers = include_directories('include')

# === Library ==================================================================

library_sources = files(
  'src/ucrc.c',
  'src/ucrc_tables.c',
)

ucrc_lib = static_library(
  'ucrc',
  library_sources,
  include_directories: [public_headers],
  install: true,
)

install_headers(
  'include/ucrc.h',
  'include/ucrc_conf.h',
  'include/ucrc_platform.h',
  subdir: 'ucrc',
)

ucrc_dep = declare_dependency(
  include_directories: [public_headers],
  link_with: ucrc_lib,
)

meson.override_dependency('ucrc', ucrc_dep)

pkg = import('pkgconfig')
pkg.generate(
  ucrc_lib,
  description: project_description,
  filebase: 'ucrc',
  name: 'ucrc',
  subdirs: 'ucrc',
)

# === Table generator (developer tool) =========================================
# Built with the native (build-host) compiler so it runs even during a cross
# build. `meson compile regen-tables` rewrites the committed src/ucrc_tables.c;
# the freshness test (see tests/) guards against forgetting to do so.

gen_tables = executable(
  'ucrc-gen-tables',
  'tools/gen_tables.c',
  native: true,
  install: false,
)

committed_tables = meson.current_source_dir() / 'src' / 'ucrc_tables.c'

run_target(
  'regen-tables',
  command: [gen_tables, committed_tables],
)

# === Tests ====================================================================

if get_option('build_tests')
  subdir('tests')
endif
