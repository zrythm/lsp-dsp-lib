/*
 * minmax3.cpp
 *
 *  Created on: 6 июл. 2020 г.
 *      Author: sadko
 */

#include <lsp-plug.in/dsp/dsp.h>
#include <lsp-plug.in/test-fw/ptest.h>
#include <lsp-plug.in/test-fw/helpers.h>
#include <lsp-plug.in/common/alloc.h>

#define MIN_RANK 8
#define MAX_RANK 16

namespace lsp
{
    namespace generic
    {
        void pmin3(float *dst, const float *a, const float *b, size_t count);
        void pmax3(float *dst, const float *a, const float *b, size_t count);
        void psmin3(float *dst, const float *a, const float *b, size_t count);
        void psmax3(float *dst, const float *a, const float *b, size_t count);
        void pamin3(float *dst, const float *a, const float *b, size_t count);
        void pamax3(float *dst, const float *a, const float *b, size_t count);
    }

    IF_ARCH_X86(
        namespace sse
        {
            void pmin3(float *dst, const float *a, const float *b, size_t count);
            void pmax3(float *dst, const float *a, const float *b, size_t count);
            void psmin3(float *dst, const float *a, const float *b, size_t count);
            void psmax3(float *dst, const float *a, const float *b, size_t count);
            void pamin3(float *dst, const float *a, const float *b, size_t count);
            void pamax3(float *dst, const float *a, const float *b, size_t count);
        }

        namespace avx
        {
            void pmin3(float *dst, const float *a, const float *b, size_t count);
            void pmax3(float *dst, const float *a, const float *b, size_t count);
            void psmin3(float *dst, const float *a, const float *b, size_t count);
            void psmax3(float *dst, const float *a, const float *b, size_t count);
            void pamin3(float *dst, const float *a, const float *b, size_t count);
            void pamax3(float *dst, const float *a, const float *b, size_t count);
        }
    )

    IF_ARCH_ARM(
        namespace neon_d32
        {
            void pmin3(float *dst, const float *a, const float *b, size_t count);
            void pmax3(float *dst, const float *a, const float *b, size_t count);
            void psmin3(float *dst, const float *a, const float *b, size_t count);
            void psmax3(float *dst, const float *a, const float *b, size_t count);
            void pamin3(float *dst, const float *a, const float *b, size_t count);
            void pamax3(float *dst, const float *a, const float *b, size_t count);
        }
    )

    IF_ARCH_AARCH64(
        namespace asimd
        {
            void pmin3(float *dst, const float *a, const float *b, size_t count);
            void pmax3(float *dst, const float *a, const float *b, size_t count);
            void psmin3(float *dst, const float *a, const float *b, size_t count);
            void psmax3(float *dst, const float *a, const float *b, size_t count);
            void pamin3(float *dst, const float *a, const float *b, size_t count);
            void pamax3(float *dst, const float *a, const float *b, size_t count);
        }
    )
}

typedef void (* min3_t)(float *dst, const float *a, const float *b, size_t count);

//-----------------------------------------------------------------------------
PTEST_BEGIN("dsp.pmath", minmax3, 5, 10000)

    void call(const char *label, float *dst, const float *a, const float *b, size_t count, min3_t func)
    {
        if (!PTEST_SUPPORTED(func))
            return;

        char buf[80];
        sprintf(buf, "%s x %d", label, int(count));
        printf("Testing %s numbers...\n", buf);

        PTEST_LOOP(buf,
            func(dst, a, b, count);
        );
    }

    PTEST_MAIN
    {
        size_t buf_size = 1 << MAX_RANK;
        uint8_t *data   = NULL;
        float *dst      = alloc_aligned<float>(data, buf_size * 6, 64);
        float *a        = &dst[buf_size];
        float *b        = &a[buf_size];
        float *backup   = &b[buf_size];

        randomize_sign(dst, buf_size*3);
        dsp::copy(backup, dst, buf_size*3);

        #define CALL(method) \
            dsp::copy(dst, backup, buf_size*3); \
            call(#method, dst, a, b, count, method);

        for (size_t i=MIN_RANK; i <= MAX_RANK; ++i)
        {
            size_t count = 1 << i;

            CALL(generic::pmin3);
            IF_ARCH_X86(CALL(sse::pmin3));
            IF_ARCH_X86(CALL(avx::pmin3));
            IF_ARCH_ARM(CALL(neon_d32::pmin3));
            IF_ARCH_AARCH64(CALL(asimd::pmin3));
            PTEST_SEPARATOR;

            CALL(generic::pmax3);
            IF_ARCH_X86(CALL(sse::pmax3));
            IF_ARCH_X86(CALL(avx::pmax3));
            IF_ARCH_ARM(CALL(neon_d32::pmax3));
            IF_ARCH_AARCH64(CALL(asimd::pmax3));
            PTEST_SEPARATOR;

            CALL(generic::psmin3);
            IF_ARCH_X86(CALL(sse::psmin3));
            IF_ARCH_X86(CALL(avx::psmin3));
            IF_ARCH_ARM(CALL(neon_d32::psmin3));
            IF_ARCH_AARCH64(CALL(asimd::psmin3));
            PTEST_SEPARATOR;

            CALL(generic::psmax3);
            IF_ARCH_X86(CALL(sse::psmax3));
            IF_ARCH_X86(CALL(avx::psmax3));
            IF_ARCH_ARM(CALL(neon_d32::psmax3));
            IF_ARCH_AARCH64(CALL(asimd::psmax3));
            PTEST_SEPARATOR;

            CALL(generic::pamin3);
            IF_ARCH_X86(CALL(sse::pamin3));
            IF_ARCH_X86(CALL(avx::pamin3));
            IF_ARCH_ARM(CALL(neon_d32::pamin3));
            IF_ARCH_AARCH64(CALL(asimd::pamin3));
            PTEST_SEPARATOR;

            CALL(generic::pamax3);
            IF_ARCH_X86(CALL(sse::pamax3));
            IF_ARCH_X86(CALL(avx::pamax3));
            IF_ARCH_ARM(CALL(neon_d32::pamax3));
            IF_ARCH_AARCH64(CALL(asimd::pamax3));
            PTEST_SEPARATOR2;
        }

        free_aligned(data);
    }
PTEST_END



