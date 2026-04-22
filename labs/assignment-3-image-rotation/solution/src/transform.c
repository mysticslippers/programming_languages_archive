#include <stddef.h>

#include "transform.h"

#include "dimensions.h"
#include "image.h"

static struct pixel *image_pixel_at(const struct image *image, uint64_t x, uint64_t y) {
    return image->data + y * image->size.x + x;
}

static struct image image_copy(const struct image source) {
    struct image result = image_create(source.size);

    if (result.data == NULL && (source.size.x != 0 && source.size.y != 0)) {
        return result;
    }

    for (uint64_t y = 0; y < source.size.y; y++) {
        for (uint64_t x = 0; x < source.size.x; x++) {
            *image_pixel_at(&result, x, y) = *image_pixel_at(&source, x, y);
        }
    }

    return result;
}

static struct image rotate_90(const struct image source) {
    const struct dimensions rotated_size = dimensions_reverse(&source.size);
    struct image result = image_create(rotated_size);

    if (result.data == NULL && (rotated_size.x != 0 && rotated_size.y != 0)) {
        return result;
    }

    for (uint64_t y = 0; y < source.size.y; y++) {
        for (uint64_t x = 0; x < source.size.x; x++) {
            const uint64_t new_x = source.size.y - 1 - y;
            const uint64_t new_y = x;
            *image_pixel_at(&result, new_x, new_y) = *image_pixel_at(&source, x, y);
        }
    }

    return result;
}

static struct image rotate_180(const struct image source) {
    struct image result = image_create(source.size);

    if (result.data == NULL && (source.size.x != 0 && source.size.y != 0)) {
        return result;
    }

    for (uint64_t y = 0; y < source.size.y; y++) {
        for (uint64_t x = 0; x < source.size.x; x++) {
            const uint64_t new_x = source.size.x - 1 - x;
            const uint64_t new_y = source.size.y - 1 - y;
            *image_pixel_at(&result, new_x, new_y) = *image_pixel_at(&source, x, y);
        }
    }

    return result;
}

static struct image rotate_270(const struct image source) {
    const struct dimensions rotated_size = dimensions_reverse(&source.size);
    struct image result = image_create(rotated_size);

    if (result.data == NULL && (rotated_size.x != 0 && rotated_size.y != 0)) {
        return result;
    }

    for (uint64_t y = 0; y < source.size.y; y++) {
        for (uint64_t x = 0; x < source.size.x; x++) {
            const uint64_t new_x = y;
            const uint64_t new_y = source.size.x - 1 - x;
            *image_pixel_at(&result, new_x, new_y) = *image_pixel_at(&source, x, y);
        }
    }

    return result;
}

struct image image_rotate(const struct image source, int angle) {
    if (angle == 0) {
        return image_copy(source);
    }

    if (angle == 90 || angle == -270) {
        return rotate_90(source);
    }

    if (angle == 180 || angle == -180) {
        return rotate_180(source);
    }

    if (angle == 270 || angle == -90) {
        return rotate_270(source);
    }

    return image_copy(source);
}
