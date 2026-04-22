#pragma once

#include <inttypes.h>

struct dimensions {
    uint64_t x;
    uint64_t y;
};

struct dimensions dimensions_reverse(const struct dimensions *dim);
