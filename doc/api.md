# API contract

Both functions are declared in `src/qcompress.h`. Neither allocates, neither writes outside the caller's buffer, and the packer keeps no state between calls: it is re-entrant.

## qcompress_pack_size

```c
size_t qcompress_pack_size(size_t src_len);
```

Returns `10 + src_len + 5 * ceil(src_len / 65535)`, the exact number of bytes `qcompress_pack()` writes:

- 4 bytes for the big-endian original length,
- 2 bytes for the zlib header,
- 5 bytes of stored-block header per block,
- `src_len` bytes of payload,
- 4 bytes for the big-endian Adler-32.

The result is a `size_t`. Two limits apply, neither checked at run time. The container stores the original length in four bytes, so `src_len` must be at most 4,294,967,295; that input is exactly 65,537 blocks and packs to `10 + 4,294,967,295 + 5 * 65,537 = 4,295,294,990` bytes. On the 32-bit target the packed size must itself fit in `size_t`, which lowers the input bound to 4,294,639,620 bytes (packed size 4,294,967,290). On a 64-bit host only the container limit applies.

## qcompress_pack

```c
qcompress_status_t qcompress_pack(uint8_t *dst, size_t dst_size, const uint8_t *src, size_t src_len);
```

Writes exactly `qcompress_pack_size(src_len)` bytes to `dst` and returns `QCOMPRESS_OK`.

Returns `QCOMPRESS_ERROR` and writes nothing to `dst` when:

- `src_len` is 0,
- `src` or `dst` is `NULL`,
- `dst_size` is smaller than `qcompress_pack_size(src_len)`.

`dst` and `src` must not overlap: the packer does not detect an in-place attempt. The 4-byte prefix is the **uncompressed** length, not the packed length; `qUncompress()` compares it with the inflated size and returns an empty `QByteArray` on a mismatch.

## Wire format

```
00 00 00 <len>         original length, big-endian
78 01                  zlib header: 32 KiB window, no preset dictionary
<blocks>               deflate stored blocks
<adler>                Adler-32 of the original bytes, big-endian
```

Each stored block is 1 byte of BFINAL/BTYPE (0x01 for the last block, 0x00 otherwise), 2 bytes of LEN, 2 bytes of NLEN (the one's complement of LEN), both little-endian, then LEN data bytes. LEN is a 16-bit field in RFC 1951, so 65,535 is a format limit, not a tuning parameter; a longer input is split across blocks.

The Adler-32 starts at 1, has no final XOR, and covers the source bytes, not the packed stream.

## Compatibility with Qt

`qCompress()` in Qt is `zlib::compress()` (level 6 by default) plus the same 4-byte prefix; `qUncompress()` parses the prefix, inflates the rest and requires the inflated size to equal the prefix. This library emits stored blocks, so the bytes differ from Qt's own output and the packed size is larger, but the inflated result is identical. Anything that accepts a standard zlib stream after skipping the 4-byte prefix will read it.

## Adding a decoder

A decoder is not part of this release. If one is added, keep it a separate entry point with its own size function and the same budget rule: validate the length prefix and the destination capacity before writing anything, and never allocate.
