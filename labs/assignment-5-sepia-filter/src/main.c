#include <stdio.h>
#include <string.h>

#include "bmp.h"
#include "common.h"
#include "image.h"
#include "sepia.h"

static void print_usage(void) {
    fprintf(stderr,
            "Usage: %s <source-image> <transformed-image> <mode>\n"
            "Modes:\n"
            "  c    - sepia filter in C\n"
            "  sse  - sepia filter in SSE/ASM\n",
            EXECUTABLE_NAME);
}

static int mode_is_valid(const char *mode) {
    return strcmp(mode, "c") == 0 || strcmp(mode, "sse") == 0;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        print_usage();
        return 1;
    }

    const char *source_path = argv[1];
    const char *target_path = argv[2];
    const char *mode = argv[3];

    if (!mode_is_valid(mode)) {
        fprintf(stderr, "Invalid mode: %s\n", mode);
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

    struct image result = {0};

    if (strcmp(mode, "c") == 0) {
        result = image_sepia_c(source);
    } else {
        result = image_sepia_sse(source);
    }

    if (result.data == NULL && (source.size.x != 0 && source.size.y != 0)) {
        fprintf(stderr, "Failed to apply sepia filter\n");
        image_destroy(&source);
        return 1;
    }

    FILE *target_file = fopen(target_path, "wb");
    if (target_file == NULL) {
        fprintf(stderr, "Cannot open target file: %s\n", target_path);
        image_destroy(&result);
        image_destroy(&source);
        return 1;
    }

    const enum write_status write_status = to_bmp(target_file, &result);
    fclose(target_file);

    if (write_status != WRITE_OK) {
        fprintf(stderr, "Failed to write BMP file: %s\n", target_path);
        image_destroy(&result);
        image_destroy(&source);
        return 1;
    }

    image_destroy(&result);
    image_destroy(&source);
    return 0;
}
