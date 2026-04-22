#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"
#include "common.h"
#include "image.h"
#include "transform.h"

static void print_usage(void) {
    fprintf(stderr,
            "Usage: %s <source-image> <transformed-image> <angle>\n",
            EXECUTABLE_NAME);
}

static int angle_is_valid(int angle) {
    return angle == 0 ||
           angle == 90 ||
           angle == -90 ||
           angle == 180 ||
           angle == -180 ||
           angle == 270 ||
           angle == -270;
}

static int parse_angle(const char *text, int *angle) {
    char *endptr = NULL;
    const long value = strtol(text, &endptr, 10);

    if (text == endptr || *endptr != '\0') {
        return 0;
    }

    *angle = (int) value;
    return 1;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        print_usage();
        return 1;
    }

    const char *source_path = argv[1];
    const char *target_path = argv[2];
    int angle = 0;

    if (!parse_angle(argv[3], &angle) || !angle_is_valid(angle)) {
        fprintf(stderr, "Invalid angle: %s\n", argv[3]);
        print_usage();
        return 1;
    }

    FILE *source_file = fopen(source_path, "rb");
    if (source_file == NULL) {
        fprintf(stderr, "Cannot open source file: %s\n", source_path);
        return 1;
    }

    struct image source = {0};
    const enum read_status read_status = from_bmp(source_file, &source);
    fclose(source_file);

    if (read_status != READ_OK) {
        fprintf(stderr, "Failed to read BMP file: %s\n", source_path);
        return 1;
    }

    struct image rotated = image_rotate(source, angle);
    if (rotated.data == NULL && (source.size.x != 0 && source.size.y != 0)) {
        fprintf(stderr, "Failed to transform image\n");
        image_destroy(&source);
        return 1;
    }

    FILE *target_file = fopen(target_path, "wb");
    if (target_file == NULL) {
        fprintf(stderr, "Cannot open target file: %s\n", target_path);
        image_destroy(&rotated);
        image_destroy(&source);
        return 1;
    }

    const enum write_status write_status = to_bmp(target_file, &rotated);
    fclose(target_file);

    if (write_status != WRITE_OK) {
        fprintf(stderr, "Failed to write BMP file: %s\n", target_path);
        image_destroy(&rotated);
        image_destroy(&source);
        return 1;
    }

    image_destroy(&rotated);
    image_destroy(&source);
    return 0;
}
