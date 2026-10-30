#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "qcompress.h"

static const uint8_t payload[] = { '{', '"', 'v', '"', ':', '1', '}' };

static const uint8_t expected[] = {
    0x00U, 0x00U, 0x00U, 0x07U, 0x78U, 0x01U, 0x01U, 0x07U, 0x00U, 0xF8U, 0xFFU,
    0x7BU, 0x22U, 0x76U, 0x22U, 0x3AU, 0x31U, 0x7DU, 0x08U, 0x93U, 0x02U, 0x1EU
};

int main(void)
{
    uint8_t packed[32];
    size_t size;
    size_t i;
    int failures = 0;

    size = qcompress_pack_size(sizeof(payload));
    printf("packed size: %lu\n", (unsigned long)size);

    if (size != sizeof(expected)) {
        printf("FAIL size %lu, want %lu\n", (unsigned long)size, (unsigned long)sizeof(expected));
        failures++;
    }

    if (qcompress_pack(packed, sizeof(packed), payload, sizeof(payload)) != QCOMPRESS_OK) {
        printf("FAIL qcompress_pack\n");
        return 1;
    }

    printf("packed     :");
    for (i = 0U; i < size; i++) {
        printf(" %02X", (unsigned int)packed[i]);
    }
    printf("\n");

    if (memcmp(packed, expected, sizeof(expected)) != 0) {
        printf("FAIL payload bytes\n");
        failures++;
    }

    if (qcompress_pack(packed, 21U, payload, sizeof(payload)) != QCOMPRESS_ERROR) {
        printf("FAIL undersized destination was accepted\n");
        failures++;
    }

    if (qcompress_pack(NULL, sizeof(packed), payload, sizeof(payload)) != QCOMPRESS_ERROR) {
        printf("FAIL NULL destination was accepted\n");
        failures++;
    }

    if (qcompress_pack(packed, sizeof(packed), NULL, sizeof(payload)) != QCOMPRESS_ERROR) {
        printf("FAIL NULL source was accepted\n");
        failures++;
    }

    if (qcompress_pack(packed, sizeof(packed), payload, 0U) != QCOMPRESS_ERROR) {
        printf("FAIL zero-length source was accepted\n");
        failures++;
    }

    if (failures != 0) {
        printf("%d check(s) failed\n", failures);
        return 1;
    }

    printf("all checks passed\n");
    return 0;
}
