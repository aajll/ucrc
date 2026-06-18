/**
 * SPDX-License-Identifier: MIT
 *
 * @file test_zlib.c
 *
 * @brief
 *    Cross-checks ucrc's CRC-32/ISO-HDLC model against zlib's reference
 *    crc32() over random data, one-shot and incremental. Built only when
 *    zlib is available (see tests/meson.build). Standalone driver; uses the
 *    shared harness and PRNG.
 */

#include "test_harness.h"
#include "test_support.h"
#include "ucrc.h"

#include <stdio.h>
#include <zlib.h>

#define BUF 512u

int
main(void)
{
        unsigned char zbuf[BUF];
        ucrc_octet_t ubuf[BUF];
        uint32_t rng = 0x00C0FFEEu;
        unsigned trial;

        fprintf(stdout, "\n=== ucrc vs zlib CRC-32 ===\n\n");

        for (trial = 0u; trial < 3000u; trial++) {
                size_t len = (size_t)(ucrc_test_rng(&rng) % (BUF + 1u));
                size_t i;
                uint32_t z;
                uint32_t u;

                for (i = 0u; i < len; i++) {
                        unsigned char byte =
                            (unsigned char)(ucrc_test_rng(&rng) & 0xFFu);

                        zbuf[i] = byte;
                        ubuf[i] = (ucrc_octet_t)byte;
                }

                z = (uint32_t)crc32(0uL, zbuf, (uInt)len);
                u = ucrc_compute(&ucrc_crc32_iso_hdlc, ubuf, len);
                TEST_ASSERT(u == z);

                /* Incremental: zlib's running value and ucrc's streaming API
                 * agree at an arbitrary split. */
                if (len > 0u) {
                        size_t cut = (size_t)(ucrc_test_rng(&rng) % (len + 1u));
                        uLong zr = crc32(0uL, zbuf, (uInt)cut);
                        uint32_t cr;

                        zr = crc32(zr, &zbuf[cut], (uInt)(len - cut));
                        cr = ucrc_begin(&ucrc_crc32_iso_hdlc);
                        cr = ucrc_update(&ucrc_crc32_iso_hdlc, cr, ubuf, cut);
                        cr = ucrc_update(&ucrc_crc32_iso_hdlc, cr, &ubuf[cut],
                                         len - cut);
                        cr = ucrc_finish(&ucrc_crc32_iso_hdlc, cr);
                        TEST_ASSERT(cr == (uint32_t)zr);
                }
        }

        fprintf(stdout, "PASS  ucrc matches zlib over 3000 random buffers\n");
        fprintf(stdout, "\n=== All tests passed ===\n\n");
        return EXIT_SUCCESS;
}
