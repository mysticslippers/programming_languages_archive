cmake_minimum_required(VERSION 3.10)
project(assignment-5-sepia-filter C ASM_NASM)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include_directories(include)

add_compile_options(
    $<$<COMPILE_LANGUAGE:C>:-pedantic>
    $<$<COMPILE_LANGUAGE:C>:-Wall>
    $<$<COMPILE_LANGUAGE:C>:-Werror>
    $<$<COMPILE_LANGUAGE:C>:-Wno-attributes>
    $<$<COMPILE_LANGUAGE:C>:-ggdb>
)

set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -g -F dwarf")

set(COMMON_SOURCES
    src/bmp.c
    src/dimensions.c
    src/image.c
    src/sepia.c
    src/sepia_sse.asm
)

add_executable(image-transformer
    src/main.c
    ${COMMON_SOURCES}
)

add_executable(benchmark-sepia
    test/benchmark.c
    src/bmp.c
    src/dimensions.c
    src/image.c
    src/sepia.c
    src/sepia_sse.asm
)

enable_testing()
add_test(NAME benchmark-sepia COMMAND benchmark-sepia)