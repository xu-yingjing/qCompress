#ifndef QCOMPRESS_H
#define QCOMPRESS_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    QCOMPRESS_OK = 0U,
    QCOMPRESS_ERROR,
} qcompress_status_t;

size_t qcompress_pack_size(size_t src_len);

qcompress_status_t qcompress_pack(uint8_t *dst, size_t dst_size, const uint8_t *src, size_t src_len);

#endif
