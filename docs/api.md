# API reference

Every declaration lives in [`include/ucrc.h`](../include/ucrc.h), which carries the full Doxygen contract. Configuration macros are listed in the [integration guide](integration.md#configuration), and the reasons behind these rules are in [design.md](design.md).

## Conventions

- Every function takes the model by pointer and returns a `uint32_t`. Final results are masked to the model width; cast them to `uint8_t`-sized or `uint16_t` storage as needed.
- Input is a stream of octets, one per `ucrc_octet_t`, and `len` always counts octets. Only the low 8 bits of each element are read.
- Preconditions are checked with `UCRC_ASSERT`. When the assertion is compiled out, a `NULL` model or a `NULL` buffer with a non-zero length returns a defined value instead of dereferencing it.
- The library holds no mutable state. Calls are reentrant, and concurrent calls need no locking as long as each stream keeps its own running value.

## Octet type

<!-- snippet: skip -->
```c
typedef uint8_t ucrc_octet_t;   /* where CHAR_BIT is 8 */
typedef uint16_t ucrc_octet_t;  /* where CHAR_BIT is 16 */
```

`ucrc_platform.h` picks the type from `CHAR_BIT` and `UCHAR_MAX`, and `UCRC_ADDR_UNIT_BITS` is `8u` or `16u` to match. A word that holds two packed octets must be split into one octet per element before it is passed in.

## Models

<!-- snippet: skip -->
```c
typedef struct {
        uint32_t poly;
        uint32_t init;
        uint32_t xorout;
        const uint32_t *table;
        uint_fast8_t width;
        bool refin;
        bool refout;
} ucrc_model_t;
```

A model holds the standard CRC parameters. `poly` is the generator polynomial in normal (non-reflected) form without its top bit, `init` the register's starting value, and `xorout` the value XORed into the final result. `refin` reflects each input octet and `refout` reflects the final register. `width` is the CRC width in bits and must be 8, 16 or 32; it is not checked at run time.

`table` selects the engine. `NULL` selects the bitwise engine, which handles any model. A non-`NULL` table must be one built for the selected strategy and for this model's `poly`, `width` and `refin`; `refout`, `init` and `xorout` do not affect it. The predefined models carry the right table for the build, so application-defined models set `table` to `NULL`.

### Predefined models

<!-- snippet: skip -->
```c
extern const ucrc_model_t ucrc_crc8_smbus;         /* UCRC_ENABLE_CRC8 */
extern const ucrc_model_t ucrc_crc16_ccitt_false;  /* UCRC_ENABLE_CRC16 */
extern const ucrc_model_t ucrc_crc32_iso_hdlc;     /* UCRC_ENABLE_CRC32 */
```

| Model | Width | Poly | Init | Reflected | XOR out | Check |
| --- | --- | --- | --- | --- | --- | --- |
| `ucrc_crc8_smbus` | 8 | `0x07` | `0x00` | no | `0x00` | `0xF4` |
| `ucrc_crc16_ccitt_false` | 16 | `0x1021` | `0xFFFF` | no | `0x0000` | `0x29B1` |
| `ucrc_crc32_iso_hdlc` | 32 | `0x04C11DB7` | `0xFFFFFFFF` | yes | `0xFFFFFFFF` | `0xCBF43926` |

The check value is the CRC of the nine ASCII octets `123456789`. CRC-32/ISO-HDLC is the CRC used by zlib, Ethernet, gzip and PNG. Each model is declared only when its width is enabled.

### Application-defined models

For any other CRC, define a `const` model with `table` set to `NULL`. The parameters for a named CRC are listed in the [CRC RevEng catalogue](https://reveng.sourceforge.io/crc-catalogue/).

```c
/* CRC-16/MODBUS: poly 0x8005, init 0xFFFF, reflected, no final XOR. */
static const ucrc_model_t crc16_modbus = {
        .poly = 0x8005u,
        .init = 0xFFFFu,
        .xorout = 0x0000u,
        .table = NULL,
        .width = 16u,
        .refin = true,
        .refout = true,
};
```

## One-shot

<!-- snippet: skip -->
```c
uint32_t ucrc_compute(const ucrc_model_t *model, const ucrc_octet_t *data, size_t len);
```

Returns the CRC of `len` octets, with `init`, reflection and `xorout` applied. A `len` of 0 is valid and returns the CRC of the empty message, and `data` may then be `NULL`. A `NULL` model returns `0`.

<!-- snippet: body -->
```c
static const ucrc_octet_t check[9] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};

uint16_t crc = (uint16_t)ucrc_compute(&ucrc_crc16_ccitt_false, check, 9u); /* 0x29B1 */
(void)crc;
```

## Streaming

<!-- snippet: skip -->
```c
uint32_t ucrc_begin(const ucrc_model_t *model);
uint32_t ucrc_update(const ucrc_model_t *model, uint32_t crc, const ucrc_octet_t *data, size_t len);
uint32_t ucrc_finish(const ucrc_model_t *model, uint32_t crc);
```

`ucrc_begin` returns the starting running value. `ucrc_update` folds `len` octets into a running value and returns the new one; a `len` of 0 returns `crc` unchanged. `ucrc_finish` applies output reflection and `xorout` and returns the result, masked to the width. Pass the same model to all three calls.

The running value is opaque: for a reflected table model it is held reflected, so read the result only through `ucrc_finish`. Splitting the input at any boundaries gives the same result as one `ucrc_compute` call over the whole input.

A `NULL` model makes `ucrc_begin` and `ucrc_finish` return `0` and `ucrc_update` return `crc`. A `NULL` buffer with a non-zero length makes `ucrc_update` return `crc`.

<!-- snippet: body -->
```c
static const ucrc_octet_t part_a[4] = {'1', '2', '3', '4'};
static const ucrc_octet_t part_b[5] = {'5', '6', '7', '8', '9'};

uint32_t running = ucrc_begin(&ucrc_crc32_iso_hdlc);
running = ucrc_update(&ucrc_crc32_iso_hdlc, running, part_a, 4u);
running = ucrc_update(&ucrc_crc32_iso_hdlc, running, part_b, 5u);
uint32_t crc = ucrc_finish(&ucrc_crc32_iso_hdlc, running); /* 0xCBF43926 */
(void)crc;
```

## Version

`ucrc.h` defines the release it belongs to, so a copied-in header still identifies itself:

<!-- snippet: skip -->
```c
#define UCRC_VERSION_MAJOR 0
#define UCRC_VERSION_MINOR 1
#define UCRC_VERSION_PATCH 0
#define UCRC_VERSION "0.1.0"
```

`UCRC_VERSION_NUMBER` is `MAJOR * 10000 + MINOR * 100 + PATCH` as a `long`, so it holds any version where `int` is 16 bits. Compare it in the preprocessor to require a minimum release:

```c
#if UCRC_VERSION_NUMBER < 100L
#error "ucrc 0.1.0 or later is required"
#endif
```

`UCRC_VERSION_STRING` is the same string as `UCRC_VERSION`.
