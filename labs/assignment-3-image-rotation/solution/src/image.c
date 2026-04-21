#include <stdlib.h>

#include "image.h"

struct image image_create(struct dimensions size) {
    struct image image = {0};
    image.size = size;

    if (size.x == 0 || size.y == 0) {
        image.size.x = 0;
        image.size.y = 0;
        image.data = NULL;
        return image;
    }

    image.data = malloc(size.x * size.y * sizeof(struct pixel));
    if (image.data == NULL) {
        image.size.x = 0;
        image.size.y = 0;
    }

    return image;
}

void image_destroy(struct image *image) {
    if (image == NULL) {
        return;
    }

    free(image->data);
    image->data = NULL;
    image->size.x = 0;
    image->size.y = 0;
}