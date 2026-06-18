ucrc
====

Lean CRC-8/16/32 library for embedded, RTOS, and Linux targets. The CRC model
is a runtime descriptor, while the computation strategy is fixed at compile
time, so you pay only for the size and speed trade-off you choose. Written in
C11, allocation-free, and MISRA C:2023 aware.

Features
--------

- **Three widths**: CRC-8, CRC-16, CRC-32, each independently enabled.
- **Runtime model descriptor**: choose the polynomial, init, reflection, and
  xor-out at the call site via ``ucrc_model_t``.
- **Predefined standards**: CRC-16/CCITT-FALSE, CRC-32/ISO-HDLC, CRC-8/SMBUS.
- **Compile-time strategy**: bitwise, nibble table, or byte-wise table.
- **One-shot and streaming** APIs; reentrant, caller-owned running value.
- **16-bit-MAU aware**: runs on 16-bit-MAU targets with bit-identical results.

Quick Start
-----------

.. code-block:: c

   #include "ucrc.h"

   /* One-shot CRC-32 (matches zlib's crc32). */
   uint32_t crc = ucrc_compute(&ucrc_crc32_iso_hdlc, data, len);

   /* Streaming CRC-16 over chunks. */
   uint32_t running = ucrc_begin(&ucrc_crc16_ccitt_false);
   running = ucrc_update(&ucrc_crc16_ccitt_false, running, chunk, n);
   uint16_t result = (uint16_t)ucrc_finish(&ucrc_crc16_ccitt_false, running);

Building
--------

.. code-block:: sh

   # Library only
   meson setup build --buildtype=release -Dbuild_tests=false
   meson compile -C build

   # With tests (all strategies plus the simulated 16-bit MAU)
   meson setup build --buildtype=debug -Dbuild_tests=true
   meson compile -C build
   meson test -C build --verbose

Contents
--------

.. toctree::
   :maxdepth: 2

   api/modules
