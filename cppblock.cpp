
#include <cmath>
#include <cstdint>
#include <cstdio>
#ifdef __SSE2__
#include <emmintrin.h>
#elif __ARM_NEON
#include <arm_neon.h>
#endif

#include "halide_benchmark.h"
#include "HalideBuffer.h"

#include "HalideRuntimeHexagonHost.h"

using namespace Halide::Runtime;
using namespace Halide::Tools;
#define    DIV4   4
double t;

Buffer<uint16_t> extract4x4block(Buffer<uint16_t> in) {
    const int downScaleRatio = 4;
    const int rawDataDiv = 2;
    int width = in.width()/downScaleRatio;
    int height = in.height()/downScaleRatio;
    Buffer<uint16_t> out(width, height);

    t = benchmark(10, 1, [&](){
        for (int i = 0; i < height / rawDataDiv; i++) {
            for (int j = 0; j < width / rawDataDiv; j++) {
#if 1
                out(i * 2, j * 2 ) = in(i * 8 , j * 8);
                out(i * 2, j * 2 + 1) = in(i * 8, j * 8 + 1);
                out(i * 2 + 1, j * 2) = in (i * 8 + 1, j * 8);
                out(i * 2 + 1, j * 2 + 1) = in((8 * i + 1), (8 * j + 1));
#endif
            }
        }
    });

    return out;
}

int main(int argc, char **argv) {
    printf("enter the main function\n");
    const int width[4] = {512, 2048,4096};
    for (int i = 0; i < 3; i++) {
        Buffer<uint16_t> input(width[i], width[i]);

        for (int y = 0; y < input.height(); y++) {
            for (int x = 0; x < input.width(); x++) {
                input(x, y) = rand() & 0xfff;
            }
        }
        Buffer<uint16_t> extra_NEONblock =  extract4x4block(input);
        double fast_block_time = t;
        printf("block 4x4 cpp width is %d end time is %f\n", width[i], fast_block_time);
    }

    return 0;
}

