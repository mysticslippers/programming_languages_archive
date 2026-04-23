#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "image.h"
#include "sepia.h"

#define TEST_WIDTH 2000
#define TEST_HEIGHT 2000
#define TEST_RUNS 10

static struct image create_test_image(const uint64_t width, const uint64_t height) {
    struct image img = image_create((struct dimensions){width, height});

    if (img.data == NULL && width != 0 && height != 0) {
        return img;
    }

    const uint64_t pixel_count = width * height;

    for (uint64_t i = 0; i < pixel_count; i++) {
        img.data[i] = (struct pixel){
            .b = (uint8_t)(i % 251),
            .g = (uint8_t)((i * 3) % 253),
            .r = (uint8_t)((i * 7) % 255)
        };
    }

    return img;
}

static double benchmark_c(const struct image source, const int runs) {
    clock_t total = 0;

    for (int i = 0; i < runs; i++) {
        const clock_t start = clock();
        struct image result = image_sepia_c(source);
        const clock_t end = clock();

        total += end - start;
        image_destroy(&result);
    }

    return (double)total / (double)CLOCKS_PER_SEC / (double)runs;
}

static double benchmark_sse(const struct image source, const int runs) {
    clock_t total = 0;

    for (int i = 0; i < runs; i++) {
        const clock_t start = clock();
        struct image result = image_sepia_sse(source);
        const clock_t end = clock();

        total += end - start;
        image_destroy(&result);
    }

    return (double)total / (double)CLOCKS_PER_SEC / (double)runs;
}

int main(void) {
    struct image source = create_test_image(TEST_WIDTH, TEST_HEIGHT);

    if (source.data == NULL) {
        fprintf(stderr, "Failed to create test image\n");
        return 1;
    }

    printf("Sepia benchmark\n");
    printf("Image size: %dx%d\n", TEST_WIDTH, TEST_HEIGHT);
    printf("Runs: %d\n\n", TEST_RUNS);

    const double c_time = benchmark_c(source, TEST_RUNS);
    const double sse_time = benchmark_sse(source, TEST_RUNS);

    printf("C implementation average time   : %.6f s\n", c_time);
    printf("SSE implementation average time : %.6f s\n", sse_time);

    if (sse_time > 0.0) {
        printf("Speedup: %.3fx\n", c_time / sse_time);
    }

    image_destroy(&source);
    return 0;
}
