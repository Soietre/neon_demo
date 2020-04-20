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


#if 0
#include "halide_blur.h"
Buffer<uint16_t> blur_halide(Buffer<uint16_t> in) {
    Buffer<uint16_t> out(in.width()-8, in.height()-2);

    // Call it once to initialize the halide runtime stuff
    halide_blur(in, out);
    // Copy-out result if it's device buffer and dirty.
    out.copy_to_host();

    t = benchmark(10, 1, [&]() {
        // Compute the same region of the output as blur_fast (i.e., we're
        // still being sloppy with boundary conditions)
        halide_blur(in, out);
        // Sync device execution if any.
        out.device_sync();
    });

    out.copy_to_host();

    return out;
}
#endif

Buffer<uint16_t> extractNEON_2x2block(Buffer<uint16_t> in) {
    Buffer<uint16_t> out(in.width()/DIV4, in.height()/DIV4);
    uint16x8x4_t intlv_rgba;
    uint16x8x2_t intlv_rg;
    t = benchmark(10, 1, [&]() {
        for (int y = 0; y < in.height() / DIV4  / 2; y++) {
            const uint16_t * inPtr = (uint16_t*)&(in(0, y * 8));
            uint16_t *outPtr = (uint16_t*)&(out(0, y *2));
            for (int x = 0; x < in.width() * 2; x += 32) {
                intlv_rgba = vld4q_u16(inPtr);
                intlv_rg = vtrnq_u16(intlv_rgba.val[0], intlv_rgba.val[1]);
                vst1q_u16(outPtr, intlv_rg.val[0]);
                inPtr += 32;
                outPtr += 8;
            } 
        }
    });
    return out;
}


int main(int argc, char **argv) {
    printf("enter the main function\n");
    const int width[4] = {512, 2048,4096};
    for (int i = 0; i < 3; i++) {
#ifdef MALLOC_TEST
    printf("malloc 2 buffer\n");
    uint16_t *tmp = (uint16_t *)malloc(sizeof(uint16_t) * width[i] * width[i]);
    Buffer<uint16_t> input{tmp, width[i], width[i]};
#else
        //Buffer<uint16_t> input(width[i], width[i]);
#endif

        for (int y = 0; y < input.height(); y++) {
            for (int x = 0; x < input.width(); x++) {
                input(x, y) = rand() & 0xfff;
            }
        }
        Buffer<uint16_t> extra_NEONblock =  extractNEON_2x2block(input);
        double fast_block_time = t;
        printf("block 4x4 width * height is %d neon end time is %f\n", width[i] , fast_block_time);
    }

    return 0;
}


