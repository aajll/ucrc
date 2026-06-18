/**
 * SPDX-License-Identifier: MIT
 *
 * @file gen_tables.c
 *
 * @brief
 *    Developer tool: generate src/ucrc_tables.c (the predefined CRC models
 *    and their lookup tables). This is a build-host program, not part of the
 *    library; the committed output is what consumers compile.
 *
 *    Usage:
 *      gen_tables <path>            write the generated file to <path>
 *      gen_tables --check <path>    exit 0 if <path> matches the generated
 *                                   output, 1 otherwise (freshness test)
 *
 *    The output is deterministic (no timestamps, stable ordering) so that
 *    regeneration is reproducible across machines, and is emitted as plain
 *    integer literals so host endianness / CHAR_BIT never leak into it.
 */

/* open_memstream is POSIX.1-2008; request it under -std=c11. */
#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
        const char *var;    /* exported ucrc_model_t name      */
        const char *enable; /* width enable macro              */
        uint32_t poly;
        uint32_t init;
        uint32_t xorout;
        unsigned width; /* 8, 16, 32                       */
        int refin;
        int refout;
} model_t;

static const model_t MODELS[] = {
    {"ucrc_crc8_smbus", "UCRC_ENABLE_CRC8", 0x07u, 0x00u, 0x00u, 8u, 0, 0},
    {"ucrc_crc16_ccitt_false", "UCRC_ENABLE_CRC16", 0x1021u, 0xFFFFu, 0x0000u,
     16u, 0, 0},
    {"ucrc_crc32_iso_hdlc", "UCRC_ENABLE_CRC32", 0x04C11DB7u, 0xFFFFFFFFu,
     0xFFFFFFFFu, 32u, 1, 1},
};

#define N_MODELS ((int)(sizeof(MODELS) / sizeof(MODELS[0])))

static uint32_t
mask_of(unsigned width)
{
        return (width >= 32u) ? 0xFFFFFFFFu : (((uint32_t)1u << width) - 1u);
}

static uint32_t
reflect(uint32_t value, unsigned bits)
{
        uint32_t result = 0u;
        unsigned i;

        for (i = 0u; i < bits; i++) {
                if ((value >> i) & 1u) {
                        result |= (uint32_t)1u << (bits - 1u - i);
                }
        }
        return result;
}

/* Fill a table of `entries` (256 for byte, 16 for nibble) for one model. */
static void
build_table(const model_t *m, unsigned entries, uint32_t *out)
{
        uint32_t mask = mask_of(m->width);
        unsigned index_bits = (entries == 256u) ? 8u : 4u;
        unsigned i;

        if (m->refin) {
                uint32_t rpoly = reflect(m->poly, m->width);

                for (i = 0u; i < entries; i++) {
                        uint32_t crc = i;
                        unsigned b;

                        for (b = 0u; b < index_bits; b++) {
                                crc = (crc & 1u) ? ((crc >> 1) ^ rpoly)
                                                 : (crc >> 1);
                        }
                        out[i] = crc & mask;
                }
        } else {
                uint32_t topbit = (uint32_t)1u << (m->width - 1u);

                for (i = 0u; i < entries; i++) {
                        uint32_t crc = (uint32_t)i << (m->width - index_bits);
                        unsigned b;

                        for (b = 0u; b < index_bits; b++) {
                                crc = (crc & topbit) ? ((crc << 1) ^ m->poly)
                                                     : (crc << 1);
                                crc &= mask;
                        }
                        out[i] = crc & mask;
                }
        }
}

static const char *
ctype_of(unsigned width)
{
        if (width <= 8u) {
                return "uint8_t";
        }
        if (width <= 16u) {
                return "uint16_t";
        }
        return "uint32_t";
}

static void
emit_literal(FILE *out, unsigned width, uint32_t value)
{
        if (width <= 8u) {
                fprintf(out, "0x%02Xu", (unsigned)value);
        } else if (width <= 16u) {
                fprintf(out, "0x%04Xu", (unsigned)value);
        } else {
                fprintf(out, "0x%08Xu", (unsigned)value);
        }
}

static void
emit_table_block(FILE *out, unsigned entries)
{
        int k;

        for (k = 0; k < N_MODELS; k++) {
                const model_t *m = &MODELS[k];
                uint32_t table[256];
                unsigned perline = (m->width <= 16u) ? 8u : 4u;
                unsigned i;

                build_table(m, entries, table);
                fprintf(out, "#if %s\n", m->enable);
                fprintf(out, "static const %s ucrc_tbl_%s[%u] = {\n",
                        ctype_of(m->width), m->var + 5, entries);
                for (i = 0u; i < entries; i++) {
                        if ((i % perline) == 0u) {
                                fprintf(out, "        ");
                        }
                        emit_literal(out, m->width, table[i]);
                        fputc(',', out);
                        if ((i % perline) == perline - 1u
                            || i == entries - 1u) {
                                fputc('\n', out);
                        } else {
                                fputc(' ', out);
                        }
                }
                fprintf(out, "};\n#endif\n\n");
        }
}

static void
emit(FILE *out)
{
        int k;

        fputs("/**\n"
              " * SPDX-License-Identifier: MIT\n"
              " *\n"
              " * @file ucrc_tables.c\n"
              " *\n"
              " * @brief Predefined CRC models and their lookup tables.\n"
              " *\n"
              " * GENERATED FILE - do not edit by hand.\n"
              " * Regenerate with: meson compile regen-tables\n"
              " * The 'tables-up-to-date' test enforces that this file is "
              "current.\n"
              " */\n\n"
              "/* clang-format off */\n\n"
              "#include \"ucrc.h\"\n\n",
              out);

        fputs("#if UCRC_STRATEGY == UCRC_STRATEGY_BYTE\n\n", out);
        emit_table_block(out, 256u);
        fputs("#elif UCRC_STRATEGY == UCRC_STRATEGY_NIBBLE\n\n", out);
        emit_table_block(out, 16u);
        fputs("#endif /* UCRC_STRATEGY */\n\n", out);

        for (k = 0; k < N_MODELS; k++) {
                const model_t *m = &MODELS[k];

                fprintf(out, "#if %s\n", m->enable);
                fprintf(out, "const ucrc_model_t %s = {\n", m->var);
                fprintf(out, "        .poly = ");
                emit_literal(out, m->width, m->poly);
                fprintf(out, ",\n        .init = ");
                emit_literal(out, m->width, m->init);
                fprintf(out, ",\n        .xorout = ");
                emit_literal(out, m->width, m->xorout);
                fprintf(out, ",\n");
                fprintf(out, "#if UCRC_STRATEGY == UCRC_STRATEGY_BITWISE\n");
                fprintf(out, "        .table = NULL,\n");
                fprintf(out, "#else\n");
                fprintf(out, "        .table = ucrc_tbl_%s,\n", m->var + 5);
                fprintf(out, "#endif\n");
                fprintf(out, "        .width = %uu,\n", m->width);
                fprintf(out, "        .refin = %s,\n",
                        m->refin ? "true" : "false");
                fprintf(out, "        .refout = %s,\n",
                        m->refout ? "true" : "false");
                fprintf(out, "};\n#endif\n\n");
        }

        fputs("/* clang-format on */\n", out);
}

static int
check_against(const char *path)
{
        char *expected = NULL;
        size_t expected_len = 0u;
        FILE *mem;
        FILE *fp;
        long file_len;
        char *actual;
        int same;

        mem = open_memstream(&expected, &expected_len);
        if (mem == NULL) {
                fprintf(stderr, "gen_tables: open_memstream failed\n");
                return 2;
        }
        emit(mem);
        fclose(mem);

        fp = fopen(path, "rb");
        if (fp == NULL) {
                fprintf(stderr, "gen_tables: cannot open %s\n", path);
                free(expected);
                return 1;
        }
        fseek(fp, 0, SEEK_END);
        file_len = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        actual = malloc((size_t)file_len + 1u);
        if (actual == NULL) {
                fclose(fp);
                free(expected);
                return 2;
        }
        if (fread(actual, 1u, (size_t)file_len, fp) != (size_t)file_len) {
                fclose(fp);
                free(actual);
                free(expected);
                return 2;
        }
        fclose(fp);

        same = ((size_t)file_len == expected_len)
               && (memcmp(actual, expected, expected_len) == 0);
        if (!same) {
                fprintf(stderr,
                        "gen_tables: %s is stale - run "
                        "'meson compile regen-tables'\n",
                        path);
        }
        free(actual);
        free(expected);
        return same ? 0 : 1;
}

int
main(int argc, char **argv)
{
        if (argc == 3 && strcmp(argv[1], "--check") == 0) {
                return check_against(argv[2]);
        }
        if (argc == 2) {
                FILE *fp = fopen(argv[1], "wb");

                if (fp == NULL) {
                        fprintf(stderr, "gen_tables: cannot write %s\n",
                                argv[1]);
                        return 1;
                }
                emit(fp);
                fclose(fp);
                return 0;
        }
        fprintf(stderr, "usage: %s <path> | --check <path>\n", argv[0]);
        return 2;
}
