#include "bmp.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "dimensions.h"
#include "image.h"

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

struct bmp_header {
    uint16_t bfType;
    uint32_t bfileSize;
    uint32_t bfReserved;
    uint32_t bOffBits;
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter;
    uint32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

static size_t get_padding(const uint64_t width) {
    return (4 - (width * sizeof(struct pixel)) % 4) % 4;
}

static bool header_is_valid(const struct bmp_header *header) {
    if (header == NULL) {
        return false;
    }

    return header->bfType == 0x4D42 &&
           header->biSize == 40 &&
           header->biPlanes == 1 &&
           header->biBitCount == 24 &&
           header->biCompression == 0 &&
           header->biWidth > 0 &&
           header->biHeight != 0 &&
           header->bOffBits >= sizeof(struct bmp_header);
}

static struct bmp_header bmp_header_create(const struct image *img) {
    const uint64_t padding = get_padding(img->size.x);
    const uint64_t row_size = img->size.x * sizeof(struct pixel) + padding;
    const uint64_t image_size = row_size * img->size.y;

    struct bmp_header header = {0};
    header.bfType = 0x4D42;
    header.bfileSize = (uint32_t)(sizeof(struct bmp_header) + image_size);
    header.bfReserved = 0;
    header.bOffBits = sizeof(struct bmp_header);

    header.biSize = 40;
    header.biWidth = (int32_t)img->size.x;
    header.biHeight = (int32_t)img->size.y;
    header.biPlanes = 1;
    header.biBitCount = 24;
    header.biCompression = 0;
    header.biSizeImage = (uint32_t)image_size;
    header.biXPelsPerMeter = 0;
    header.biYPelsPerMeter = 0;
    header.biClrUsed = 0;
    header.biClrImportant = 0;

    return header;
}

static enum read_status read_header(FILE *in, struct bmp_header *header) {
    if (fread(header, sizeof(struct bmp_header), 1, in) != 1) {
        return READ_IO_ERROR;
    }

    if (header->bfType != 0x4D42) {
        return READ_INVALID_SIGNATURE;
    }

    if (header->biBitCount != 24) {
        return READ_INVALID_BITS;
    }

    if (!header_is_valid(header)) {
        return READ_INVALID_HEADER;
    }

    return READ_OK;
}

static enum read_status skip_to_pixel_array(FILE *in, const struct bmp_header *header) {
    const long extra_bytes = (long)(header->bOffBits - sizeof(struct bmp_header));

    if (extra_bytes == 0) {
        return READ_OK;
    }

    if (fseek(in, extra_bytes, SEEK_CUR) != 0) {
        return READ_IO_ERROR;
    }

    return READ_OK;
}

enum read_status from_bmp(FILE *in, struct image *img) {
    if (in == NULL || img == NULL) {
        return READ_INVALID_HEADER;
    }

    struct bmp_header header = {0};
    enum read_status status = read_header(in, &header);
    if (status != READ_OK) {
        return status;
    }

    status = skip_to_pixel_array(in, &header);
    if (status != READ_OK) {
        return status;
    }

    const uint64_t width = (uint64_t)header.biWidth;
    const uint64_t height = (header.biHeight > 0)
                                ? (uint64_t)header.biHeight
                                : (uint64_t)(-header.biHeight);

    *img = image_create((struct dimensions){width, height});
    if (img->data == NULL && width != 0 && height != 0) {
        return READ_MEMORY_ERROR;
    }

    const size_t padding = get_padding(width);
    uint8_t padding_buffer[3] = {0};

    for (uint64_t row = 0; row < height; row++) {
        const uint64_t target_row = (header.biHeight > 0) ? (height - 1 - row) : row;
        struct pixel *row_ptr = img->data + target_row * width;

        if (fread(row_ptr, sizeof(struct pixel), width, in) != width) {
            image_destroy(img);
            return READ_INVALID_BODY;
        }

        if (padding > 0 && fread(padding_buffer, 1, padding, in) != padding) {
            image_destroy(img);
            return READ_INVALID_BODY;
        }
    }

    return READ_OK;
}

enum write_status to_bmp(FILE *out, const struct image *img) {
    if (out == NULL || img == NULL) {
        return WRITE_ERROR;
    }

    const struct bmp_header header = bmp_header_create(img);
    if (fwrite(&header, sizeof(struct bmp_header), 1, out) != 1) {
        return WRITE_ERROR;
    }

    const size_t padding = get_padding(img->size.x);
    static const uint8_t padding_bytes[3] = {0};

    for (uint64_t row = 0; row < img->size.y; row++) {
        const uint64_t source_row = img->size.y - 1 - row;
        const struct pixel *row_ptr = img->data + source_row * img->size.x;

        if (fwrite(row_ptr, sizeof(struct pixel), img->size.x, out) != img->size.x) {
            return WRITE_ERROR;
        }

        if (padding > 0 && fwrite(padding_bytes, 1, padding, out) != padding) {
            return WRITE_ERROR;
        }
    }

    return WRITE_OK;
}
