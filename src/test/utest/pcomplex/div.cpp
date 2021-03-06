/*
 * div.cpp
 *
 *  Created on: 6 сент. 2018 г.
 *      Author: sadko
 */

#include <lsp-plug.in/dsp/dsp.h>
#include <lsp-plug.in/test-fw/utest.h>
#include <lsp-plug.in/test-fw/FloatBuffer.h>

namespace lsp
{
    namespace generic
    {
        void pcomplex_div2(float *dst, const float *src, size_t count);
        void pcomplex_rdiv2(float *dst, const float *src, size_t count);
        void pcomplex_div3(float *dst, const float *t, const float *b, size_t count);
    }

    IF_ARCH_X86(
        namespace sse
        {
            void pcomplex_div2(float *dst, const float *src, size_t count);
            void pcomplex_rdiv2(float *dst, const float *src, size_t count);
            void pcomplex_div3(float *dst, const float *t, const float *b, size_t count);
        }

        namespace avx
        {
            void pcomplex_div2(float *dst, const float *src, size_t count);
            void pcomplex_rdiv2(float *dst, const float *src, size_t count);
            void pcomplex_div3(float *dst, const float *t, const float *b, size_t count);

            void pcomplex_div2_fma3(float *dst, const float *src, size_t count);
            void pcomplex_rdiv2_fma3(float *dst, const float *src, size_t count);
            void pcomplex_div3_fma3(float *dst, const float *t, const float *b, size_t count);
        }
    )

    IF_ARCH_ARM(
        namespace neon_d32
        {
            void pcomplex_div2(float *dst, const float *src, size_t count);
            void pcomplex_rdiv2(float *dst, const float *src, size_t count);
            void pcomplex_div3(float *dst, const float *t, const float *b, size_t count);
        }
    )

    IF_ARCH_AARCH64(
        namespace asimd
        {
            void pcomplex_div2(float *dst, const float *src, size_t count);
            void pcomplex_rdiv2(float *dst, const float *src, size_t count);
            void pcomplex_div3(float *dst, const float *t, const float *b, size_t count);
        }
    )

    typedef void (* pcomplex_div2_t) (float *dst, const float *src, size_t count);
    typedef void (* pcomplex_div3_t) (float *dst, const float *t, const float *b, size_t count);
}

UTEST_BEGIN("dsp.pcomplex", div)

    void call(const char *text,  size_t align, pcomplex_div2_t func1, pcomplex_div2_t func2)
    {
        if (!UTEST_SUPPORTED(func1))
            return;
        if (!UTEST_SUPPORTED(func2))
            return;

        UTEST_FOREACH(count, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                32, 33, 37, 48, 49, 64, 65, 0x3f, 100, 999, 0x1fff)
        {
            for (size_t mask=0; mask <= 0x03; ++mask)
            {
                printf("Testing %s on input buffer of %d numbers, mask=0x%x...\n", text, int(count), int(mask));

                FloatBuffer src(count*2, align, mask & 0x01);
                FloatBuffer dst1(count*2, align, mask & 0x02);
                FloatBuffer dst2(dst1);

                // Call functions
                func1(dst1, src, count);
                func2(dst2, src, count);

                UTEST_ASSERT_MSG(src.valid(), "Source buffer corrupted");
                UTEST_ASSERT_MSG(dst1.valid(), "Destination buffer 1 corrupted");
                UTEST_ASSERT_MSG(dst2.valid(), "Destination buffer 2 corrupted");

                // Compare buffers
                if (!dst2.equals_absolute(dst1, 1e-4))
                {
                    src.dump("src ");
                    dst1.dump("dst1");
                    dst2.dump("dst2");
                    UTEST_FAIL_MSG("Output of functions for test '%s' differs", text);
                }
            }
        }
    }

    void call(const char *text,  size_t align, pcomplex_div3_t func1, pcomplex_div3_t func2)
    {
        if (!UTEST_SUPPORTED(func1))
            return;
        if (!UTEST_SUPPORTED(func2))
            return;

        UTEST_FOREACH(count, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                32, 33, 37, 48, 49, 64, 65, 0x3f, 100, 999, 0x1fff)
        {
            for (size_t mask=0; mask <= 0x07; ++mask)
            {
                printf("Testing %s on input buffer of %d numbers, mask=0x%x...\n", text, int(count), int(mask));

                FloatBuffer src1(count*2, align, mask & 0x01);
                FloatBuffer src2(count*2, align, mask & 0x02);
                FloatBuffer dst1(count*2, align, mask & 0x04);
                FloatBuffer dst2(count*2, align, mask & 0x04);

                // Call functions
                func1(dst1, src1, src2, count);
                func2(dst2, src1, src2, count);

                UTEST_ASSERT_MSG(src1.valid(), "Source buffer 1corrupted");
                UTEST_ASSERT_MSG(src2.valid(), "Source buffer 2 corrupted");
                UTEST_ASSERT_MSG(dst1.valid(), "Destination buffer 1 corrupted");
                UTEST_ASSERT_MSG(dst2.valid(), "Destination buffer 2 corrupted");

                // Compare buffers
                if (!dst2.equals_absolute(dst1, 1e-4))
                {
                    src1.dump("src1");
                    src2.dump("src2");
                    dst1.dump("dst1");
                    dst2.dump("dst2");
                    UTEST_FAIL_MSG("Output of functions for test '%s' differs", text);
                }
            }
        }
    }

    UTEST_MAIN
    {
        #define CALL(generic, func, align) \
            call(#func, align, generic, func)

        IF_ARCH_X86(CALL(generic::pcomplex_div2, sse::pcomplex_div2, 16));
        IF_ARCH_X86(CALL(generic::pcomplex_rdiv2, sse::pcomplex_rdiv2, 16));
        IF_ARCH_X86(CALL(generic::pcomplex_div3, sse::pcomplex_div3, 16));

        IF_ARCH_X86(CALL(generic::pcomplex_div2, avx::pcomplex_div2, 32));
        IF_ARCH_X86(CALL(generic::pcomplex_div2, avx::pcomplex_div2_fma3, 32));
        IF_ARCH_X86(CALL(generic::pcomplex_rdiv2, avx::pcomplex_rdiv2, 32));
        IF_ARCH_X86(CALL(generic::pcomplex_rdiv2, avx::pcomplex_rdiv2_fma3, 32));
        IF_ARCH_X86(CALL(generic::pcomplex_div3, avx::pcomplex_div3, 32));
        IF_ARCH_X86(CALL(generic::pcomplex_div3, avx::pcomplex_div3_fma3, 32));

        IF_ARCH_ARM(CALL(generic::pcomplex_div2, neon_d32::pcomplex_div2, 16));
        IF_ARCH_ARM(CALL(generic::pcomplex_rdiv2, neon_d32::pcomplex_rdiv2, 16));
        IF_ARCH_ARM(CALL(generic::pcomplex_div3, neon_d32::pcomplex_div3, 16));

        IF_ARCH_AARCH64(CALL(generic::pcomplex_div2, asimd::pcomplex_div2, 16));
        IF_ARCH_AARCH64(CALL(generic::pcomplex_rdiv2, asimd::pcomplex_rdiv2, 16));
        IF_ARCH_AARCH64(CALL(generic::pcomplex_div3, asimd::pcomplex_div3, 16));
    }

UTEST_END;



