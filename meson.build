project(
  'ucrc',
  'c',
  version: '0.1.0',
  default_options: [
    'c_std=c11',
    'warning_level=3',
  ],
  license: 'MIT',
)

project_description = 'Lean CRC-8/16/32 library for embedded, RTOS, and Linux targets'

# === Version header ===========================================================

ver = meson.project_version().split('.')
major = ver[0].to_int()
minor = ver[1].to_int()
patch = ver[2].to_int()

conf = configuration_data()
conf.set_quoted('VERSION', meson.project_version())
conf.set('MAJOR', major)
conf.set('MINOR', minor)
conf.set('PATCH', patch)

generated_version_header = configure_file(
  input: 'config' / 'ucrc_version.h.in',
  output: 'ucrc_version.h',
  configuration: conf,
  install: true,
  install_dir: get_option('includedir') / 'ucrc',
)

# === Include directories ======================================================

public_headers = include_directories('include')
generated_headers = include_directories('.')

# === Library ==================================================================

library_sources = files(
  'src/ucrc.c',
  'src/ucrc_tables.c',
)

ucrc_lib = static_library(
  'ucrc',
  library_sources,
  include_directories: [public_headers, generated_headers],
  install: true,
)

install_headers(
  'include/ucrc.h',
  'include/ucrc_conf.h',
  'include/ucrc_platform.h',
  subdir: 'ucrc',
)

ucrc_dep = declare_dependency(
  include_directories: [public_headers, generated_headers],
  link_with: ucrc_lib,
  sources: generated_version_header,
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
