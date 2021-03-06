/*
 * bt.cpp
 *
 *  Created on: 23 авг. 2018 г.
 *      Author: sadko
 */

#include <lsp-plug.in/dsp/dsp.h>
#include <lsp-plug.in/test-fw/ptest.h>
#include <lsp-plug.in/common/alloc.h>

#define PERF_BUF_SIZE   0x200

namespace lsp
{
    namespace generic
    {
        void bilinear_transform_x1(dsp::biquad_x1_t *bf, const dsp::f_cascade_t *bc, float kf, size_t count);
        void bilinear_transform_x2(dsp::biquad_x2_t *bf, const dsp::f_cascade_t *bc, float kf, size_t count);
        void bilinear_transform_x4(dsp::biquad_x4_t *bf, const dsp::f_cascade_t *bc, float kf, size_t count);
        void bilinear_transform_x8(dsp::biquad_x8_t *bf, const dsp::f_cascade_t *bc, float kf, size_t count);
    }

    IF_ARCH_X86(
        namespace sse
        {
            void bilinear_transform_x1(dsp::biquad_x1_t *bf, const dsp::f_cascade_t *bc, float kf, size_t count);
            void bilinear_transform_x2(dsp::biquad_x2_t *bf, const dsp::f_cascade_t *bc, float kf, size_t count);
            void bilinear_transform_x4(dsp::biquad_x4_t *bf, const dsp::f_cascade_t *bc, float kf, size_t count);
            void bilinear_transform_x8(dsp::biquad_x8_t *bf, const dsp::f_cascade_t *bc, float kf, size_t count);
        }

        IF_ARCH_X86_64(
            namespace sse3
            {
                void x64_bilinear_transform_x8(dsp::biquad_x8_t *bf, const dsp::f_cascade_t *bc, float kf, size_t count);
            }
        )

        namespace avx
        {
            void bilinear_transform_x1(dsp::biquad_x1_t *bf, const dsp::f_cascade_t *bc, float kf, size_t count);
            void bilinear_transform_x2(dsp::biquad_x2_t *bf, const dsp::f_cascade_t *bc, float kf, size_t count);
            void bilinear_transform_x4(dsp::biquad_x4_t *bf, const dsp::f_cascade_t *bc, float kf, size_t count);
            void x64_bilinear_transform_x8(dsp::biquad_x8_t *bf, const dsp::f_cascade_t *bc, float kf, size_t count);
        }
    )

    IF_ARCH_ARM(
        namespace neon_d32
        {
            void bilinear_transform_x1(dsp::biquad_x1_t *bf, const dsp::f_cascade_t *bc, float kf, size_t count);
            void bilinear_transform_x2(dsp::biquad_x2_t *bf, const dsp::f_cascade_t *bc, float kf, size_t count);
            void bilinear_transform_x4(dsp::biquad_x4_t *bf, const dsp::f_cascade_t *bc, float kf, size_t count);
            void bilinear_transform_x8(dsp::biquad_x8_t *bf, const dsp::f_cascade_t *bc, float kf, size_t count);
        }
    )

    IF_ARCH_AARCH64(
        namespace asimd
        {
            void bilinear_transform_x1(dsp::biquad_x1_t *bf, const dsp::f_cascade_t *bc, float kf, size_t count);
            void bilinear_transform_x2(dsp::biquad_x2_t *bf, const dsp::f_cascade_t *bc, float kf, size_t count);
            void bilinear_transform_x4(dsp::biquad_x4_t *bf, const dsp::f_cascade_t *bc, float kf, size_t count);
            void bilinear_transform_x8(dsp::biquad_x8_t *bf, const dsp::f_cascade_t *bc, float kf, size_t count);
        }
    )

    typedef void (* bilinear_transform_x1_t)(dsp::biquad_x1_t *bf, const dsp::f_cascade_t *bc, float kf, size_t count);
    typedef void (* bilinear_transform_x2_t)(dsp::biquad_x2_t *bf, const dsp::f_cascade_t *bc, float kf, size_t count);
    typedef void (* bilinear_transform_x4_t)(dsp::biquad_x4_t *bf, const dsp::f_cascade_t *bc, float kf, size_t count);
    typedef void (* bilinear_transform_x8_t)(dsp::biquad_x8_t *bf, const dsp::f_cascade_t *bc, float kf, size_t count);

    static const dsp::f_cascade_t test_c =
    {
        1, 2, 1, 0,
        1, -2, 1, 0
    };
}

//-----------------------------------------------------------------------------
// Performance test for bilinear transform
PTEST_BEGIN("dsp.filters", bt, 10, 10000)

    void call(const char * label, size_t count, bilinear_transform_x1_t func)
    {
        printf("Testing %s bilinear transform on buffer size %d ...\n", label, int(count));

        void *p1 = NULL, *p2 = NULL;
        dsp::biquad_x1_t *dst = alloc_aligned<dsp::biquad_x1_t>(p1, count, 64);
        dsp::f_cascade_t *src = alloc_aligned<dsp::f_cascade_t>(p2, count, 64);

        for (size_t i=0; i<count; ++i)
            src[i]  = test_c;

        PTEST_LOOP(label,
            func(dst, src, 1.0f, count);
            func(dst, src, 1.0f, count);
            func(dst, src, 1.0f, count);
            func(dst, src, 1.0f, count);
            func(dst, src, 1.0f, count);
            func(dst, src, 1.0f, count);
            func(dst, src, 1.0f, count);
            func(dst, src, 1.0f, count);
        );

        free_aligned(p1);
        free_aligned(p2);
    }

    void call(const char * label, size_t count, bilinear_transform_x2_t func)
    {
        printf("Testing %s bilinear transform on buffer size %d ...\n", label, int(count));

        count++;
        void *p1 = NULL, *p2 = NULL;
        dsp::biquad_x2_t *dst = alloc_aligned<dsp::biquad_x2_t>(p1, count, 32);
        dsp::f_cascade_t *src = alloc_aligned<dsp::f_cascade_t>(p2, count*2, 32);

        for (size_t i=0; i<count*2; ++i)
            src[i]  = test_c;

        PTEST_LOOP(label,
            func(dst, src, 1.0f, count);
            func(dst, src, 1.0f, count);
            func(dst, src, 1.0f, count);
            func(dst, src, 1.0f, count);
        );

        free_aligned(p1);
        free_aligned(p2);
    }

    void call(const char * label, size_t count, bilinear_transform_x4_t func)
    {
        printf("Testing %s bilinear transform on buffer size %d ...\n", label, int(count));

        count += 3;
        void *p1 = NULL, *p2 = NULL;
        dsp::biquad_x4_t *dst = alloc_aligned<dsp::biquad_x4_t>(p1, count, 32);
        dsp::f_cascade_t *src = alloc_aligned<dsp::f_cascade_t>(p2, count*4, 32);

        for (size_t i=0; i<count*4; ++i)
            src[i]  = test_c;

        PTEST_LOOP(label,
            func(dst, src, 1.0f, count);
            func(dst, src, 1.0f, count);
        );

        free_aligned(p1);
        free_aligned(p2);
    }

    void call(const char * label, size_t count, bilinear_transform_x8_t func)
    {
        printf("Testing %s bilinear transform on buffer size %d ...\n", label, int(count));

        count += 7;
        void *p1 = NULL, *p2 = NULL;
        dsp::biquad_x8_t *dst = alloc_aligned<dsp::biquad_x8_t>(p1, count, 32);
        dsp::f_cascade_t *src = alloc_aligned<dsp::f_cascade_t>(p2, count*8, 32);

        for (size_t i=0; i<count*8; ++i)
            src[i]  = test_c;

        PTEST_LOOP(label,
            func(dst, src, 1.0f, count);
        );

        free_aligned(p1);
        free_aligned(p2);
    }

    PTEST_MAIN
    {
        #define CALL(func) \
            call(#func, PERF_BUF_SIZE, func)

        CALL(generic::bilinear_transform_x1);
        IF_ARCH_X86(CALL(sse::bilinear_transform_x1));
        IF_ARCH_X86(CALL(avx::bilinear_transform_x1));
        IF_ARCH_ARM(CALL(neon_d32::bilinear_transform_x1));
        IF_ARCH_AARCH64(CALL(asimd::bilinear_transform_x1));
        PTEST_SEPARATOR;

        CALL(generic::bilinear_transform_x2);
        IF_ARCH_X86(CALL(sse::bilinear_transform_x2));
        IF_ARCH_X86(CALL(avx::bilinear_transform_x2));
        IF_ARCH_ARM(CALL(neon_d32::bilinear_transform_x2));
        IF_ARCH_AARCH64(CALL(asimd::bilinear_transform_x2));
        PTEST_SEPARATOR;

        CALL(generic::bilinear_transform_x4);
        IF_ARCH_X86(CALL(sse::bilinear_transform_x4));
        IF_ARCH_X86(CALL(avx::bilinear_transform_x4));
        IF_ARCH_ARM(CALL(neon_d32::bilinear_transform_x4));
        IF_ARCH_AARCH64(CALL(asimd::bilinear_transform_x4));
        PTEST_SEPARATOR;

        CALL(generic::bilinear_transform_x8);
        IF_ARCH_X86(CALL(sse::bilinear_transform_x8));
        IF_ARCH_X86_64(CALL(sse3::x64_bilinear_transform_x8));
        IF_ARCH_X86_64(CALL(avx::x64_bilinear_transform_x8));
        IF_ARCH_ARM(CALL(neon_d32::bilinear_transform_x8));
        IF_ARCH_AARCH64(CALL(asimd::bilinear_transform_x8));
        PTEST_SEPARATOR;
    }

PTEST_END





