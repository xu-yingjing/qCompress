# qcompress

Packer for the Qt `qCompress` container: a 4-byte big-endian original length followed by a zlib stream. C99, no OS, no HAL, no dynamic allocation, no writable static data.

The zlib stream uses stored (BTYPE 00) blocks, so the packed output is always a few bytes larger than the input; the payload is never compressed. The 4-byte prefix is what makes the container Qt-specific: `qUncompress()` allocates by it and returns an empty `QByteArray` when the inflated size does not match.

## Scope

Supported:

- Any byte payload, including embedded NULs.
- Output that `qUncompress()` and any zlib implementation accept after the 4-byte prefix is consumed.

Not supported, by design:

- Real compression. Block headers cost 5 bytes per 65,535-byte block; a caller that needs smaller output links zlib instead.
- Decoding, and streaming: the packed size is a closed-form function of the input length, so the caller sizes one buffer.

## Quick start

```c
#include "qcompress.h"

static const uint8_t payload[] = { '{', '"', 'v', '"', ':', '1', '}' };
static uint8_t packed[22];   /* qcompress_pack_size(sizeof(payload)) */

if (qcompress_pack(packed, sizeof(packed), payload, sizeof(payload)) != QCOMPRESS_OK) {
    /* zero length, NULL pointer, or a destination that is too small */
}
```

The full contract is in [doc/api.md](doc/api.md). `examples/qcompress_demo.c` packs `{"v":1}` and compares the 22-byte result with a known-good vector.

## Wire format

| offset | size | content |
| --- | --- | --- |
| 0 | 4 | original (uncompressed) length, big-endian |
| 4 | 2 | `78 01`, zlib header |
| 6 | 5 + n | one stored block: BFINAL/BTYPE byte, LEN and NLEN (one's complement), little-endian, then n data bytes, n <= 65,535 |
| end - 4 | 4 | Adler-32 of the original bytes, big-endian |

`{"v":1}` (7 bytes) packs to 22 bytes:

```
00 00 00 07 78 01 01 07 00 F8 FF 7B 22 76 22 3A 31 7D 08 93 02 1E
```

Compatible is not the same as byte-identical: Qt compresses by default, so its own `qCompress()` output for the same payload is shorter and has different bytes. Only the decompressed result is guaranteed to match.

## Verification

`sh build.sh` compiles the library and the example with every warning enabled (`-Wconversion`, `-Wsign-conversion`, `-Wshadow`, `-Wundef`, `-Wwrite-strings`, `-Werror`). The release was checked with arm-none-eabi-gcc 12.3 and produced no warnings.

Build and run the example on a host:

```sh
cc -std=c99 -I src src/qcompress.c examples/qcompress_demo.c -o qcompress_demo && ./qcompress_demo
```

## License

BSD 3-Clause, see [LICENSE](LICENSE).
