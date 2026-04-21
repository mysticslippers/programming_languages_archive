#include "dimensions.h"

struct dimensions dimensions_reverse(const struct dimensions *dim) {
    struct dimensions result = {0};

    if (dim == NULL) {
        return result;
    }

    result.x = dim->y;
    result.y = dim->x;
    return result;
}