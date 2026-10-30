#include "qcompress.h"
#include <stddef.h>
#include <string.h>

#define ZLIB_BLOCK_MAX (65535U)

static const uint8_t zlib_header[2] = {0x78U, 0x01U};

static uint32_t adler32_calc(const uint8_t *data, uint32_t len)
{
    uint32_t a = 1U;
    uint32_t b = 0U;
 
    while (len-- > 0U) {
        a += *data++;
        if (a >= 65521U) {
            a -= 65521U;
        }
        b += a;
        if (b >= 65521U) {
            b -= 65521U;
        }
    }
 
    return (b << 16) | a;
}

size_t qcompress_pack_size(size_t src_len)
{
    return (10U + src_len + (5U * ((src_len + ZLIB_BLOCK_MAX - 1U) / ZLIB_BLOCK_MAX)));
}

qcompress_status_t qcompress_pack(uint8_t *dst, size_t dst_size, const uint8_t *src, size_t src_len)
{
    size_t src_pos = 0U;
    size_t dst_pos = 0U;
    size_t block_len;
    uint32_t adler;

    if ((src_len == 0U) ||
        (src == NULL) ||
        (dst == NULL) ||
        (dst_size < qcompress_pack_size(src_len))) {
        return QCOMPRESS_ERROR;
    }

    dst[dst_pos++] = (uint8_t)(src_len >> 24);
    dst[dst_pos++] = (uint8_t)(src_len >> 16);
    dst[dst_pos++] = (uint8_t)(src_len >> 8);
    dst[dst_pos++] = (uint8_t)src_len;
    dst[dst_pos++] = zlib_header[0];
    dst[dst_pos++] = zlib_header[1];

    while (src_pos < src_len) {
        block_len = src_len - src_pos;
        if (block_len > ZLIB_BLOCK_MAX) {
            block_len = ZLIB_BLOCK_MAX;
        }

        dst[dst_pos++] = ((src_pos + block_len) == src_len) ? 0x01U : 0x00U;
        dst[dst_pos++] = (uint8_t)block_len;
        dst[dst_pos++] = (uint8_t)(block_len >> 8);
        dst[dst_pos++] = (uint8_t)(~block_len);
        dst[dst_pos++] = (uint8_t)(~block_len >> 8);
        (void)memcpy(&dst[dst_pos], &src[src_pos], block_len);

        dst_pos += block_len;
        src_pos += block_len;
    }

    adler = adler32_calc(src, src_len);
    dst[dst_pos++] = (uint8_t)(adler >> 24);
    dst[dst_pos++] = (uint8_t)(adler >> 16);
    dst[dst_pos++] = (uint8_t)(adler >> 8);
    dst[dst_pos++] = (uint8_t)adler;

    return QCOMPRESS_OK;
}
