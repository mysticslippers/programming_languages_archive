#include "sepia.h"

#include <stddef.h>
#include <stdint.h>

#include "image.h"

extern void sepia_sse_apply(const struct pixel *src, struct pixel *dst, uint64_t pixel_count);

static uint8_t clamp_to_byte(const float value) {
    if (value < 0.0f) {
        return 0;
    }
    if (value > 255.0f) {
        return 255;
    }
    return (uint8_t)(value + 0.5f);
}

static struct pixel sepia_pixel_c(const struct pixel source) {
    const float blue = (float)source.b;
    const float green = (float)source.g;
    const float red = (float)source.r;

    const float new_red =
        0.393f * red + 0.769f * green + 0.189f * blue;
    const float new_green =
        0.349f * red + 0.686f * green + 0.168f * blue;
    const float new_blue =
        0.272f * red + 0.534f * green + 0.131f * blue;

    return (struct pixel){
        .b = clamp_to_byte(new_blue),
        .g = clamp_to_byte(new_green),
        .r = clamp_to_byte(new_red)
    };
}

struct image image_sepia_c(const struct image source) {
    struct image result = image_create(source.size);

    if (result.data == NULL && source.size.x != 0 && source.size.y != 0) {
        return result;
    }

    const uint64_t pixel_count = source.size.x * source.size.y;

    for (uint64_t i = 0; i < pixel_count; i++) {
        result.data[i] = sepia_pixel_c(source.data[i]);
    }

    return result;
}

struct image image_sepia_sse(const struct image source) {
    struct image result = image_create(source.size);

    if (result.data == NULL && source.size.x != 0 && source.size.y != 0) {
        return result;
    }

    const uint64_t pixel_count = source.size.x * source.size.y;
    const uint64_t simd_count = pixel_count & ~3ULL;

    if (simd_count > 0) {
        sepia_sse_apply(source.data, result.data, simd_count);
    }

    for (uint64_t i = simd_count; i < pixel_count; i++) {
        result.data[i] = sepia_pixel_c(source.data[i]);
    }

    return result;
}
