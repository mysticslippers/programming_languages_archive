#pragma once

#include <inttypes.h>

#include "dimensions.h"

struct pixel {
    uint8_t b;
    uint8_t g;
    uint8_t r;
};

struct image {
    struct dimensions size;
    struct pixel *data;
};

struct image image_create(struct dimensions size);
void image_destroy(struct image *image);
