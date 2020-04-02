/*
 * sse3.cpp
 *
 *  Created on: 31 мар. 2020 г.
 *      Author: sadko
 */

#include <lsp-plug.in/common/types.h>

#ifdef ARCH_X86
    #include <lsp-plug.in/dsp/dsp.h>
    #include <lsp-plug.in/common/bits.h>
    #include <lsp-plug.in/stdlib/math.h>

    // Test framework
    #ifdef LSP_TESTING
        #include <lsp-plug.in/test-fw/test.h>
    #else
        #define TEST_EXPORT(...)
    #endif /* LSP_TESTING */

    // Feature detection
    #define LSP_PLUG_IN_DSP_ARCH_X86_IMPL
        #include <lsp-plug.in/dsp/arch/x86/features.h>
    #undef LSP_PLUG_IN_DSP_ARCH_X86_IMPL

    #define LSP_PLUG_IN_DSP_ARCH_X86_SSE3_IMPL
        #include <lsp-plug.in/dsp/arch/x86/sse3/copy.h>
        #include <lsp-plug.in/dsp/arch/x86/sse3/graphics.h>
        #include <lsp-plug.in/dsp/arch/x86/sse3/filters/static.h>
        #include <lsp-plug.in/dsp/arch/x86/sse3/filters/dynamic.h>
        #include <lsp-plug.in/dsp/arch/x86/sse3/filters/transform.h>
        #include <lsp-plug.in/dsp/arch/x86/sse3/pcomplex.h>
        #include <lsp-plug.in/dsp/arch/x86/sse3/3dmath.h>
    #undef LSP_PLUG_IN_DSP_ARCH_X86_SSE3_IMPL

    namespace lsp
    {
        namespace sse3
        {
            using namespace x86;

            #define EXPORT2(function, export)               dsp::function = sse3::export; TEST_EXPORT(sse3::export);
            #define EXPORT2_X64(function, export)           IF_ARCH_X86_64(dsp::function = sse3::export; TEST_EXPORT(sse3::export));
            #define EXPORT1(export)                         EXPORT2(export, export)
            #define SUPPORT_X64(function)                   IF_ARCH_X86_64(TEST_EXPORT(sse3::function))

            void dsp_init(const cpu_features_t *f)
            {
                if (!(f->features & CPU_OPTION_SSE3))
                    return;

                // Additional xmm registers are available only in 64-bit mode
                EXPORT1(pcomplex_mul2);
                EXPORT1(pcomplex_mul3);

                if (!feature_check(f, FEAT_FAST_MOVS))
                {
                    EXPORT2(copy, copy);
                }
                else
                {
                    TEST_EXPORT(copy);
                }
                EXPORT2(pcomplex_mod, pcomplex_mod);
                EXPORT2_X64(pcomplex_mod, x64_pcomplex_mod);

                EXPORT2_X64(biquad_process_x8, x64_biquad_process_x8);
                EXPORT2_X64(dyn_biquad_process_x8, x64_dyn_biquad_process_x8);
                EXPORT2_X64(bilinear_transform_x8, x64_bilinear_transform_x8);
                EXPORT2_X64(axis_apply_log1, x64_axis_apply_log1);
                EXPORT2_X64(axis_apply_log2, x64_axis_apply_log2);
                EXPORT2_X64(pcomplex_mul2, x64_pcomplex_mul2);
                EXPORT2_X64(pcomplex_mul3, x64_pcomplex_mul3);

                EXPORT2_X64(rgba32_to_bgra32, x64_rgba32_to_bgra32);

                EXPORT1(calc_split_point_p2v1);
                EXPORT1(calc_split_point_pvv1);

                EXPORT1(colocation_x2_v1p2);
                EXPORT1(colocation_x2_v1pv);
                EXPORT1(colocation_x3_v1p3);
                EXPORT1(colocation_x3_v1pv);
                EXPORT1(colocation_x3_v3p1);
                EXPORT1(colocation_x3_vvp1);

                EXPORT1(calc_min_distance_p3);
                EXPORT1(calc_min_distance_pv);

                EXPORT1(split_triangle_raw);
                EXPORT1(cull_triangle_raw);
            }

            #undef EXPORT2
        }
    }

#endif /* ARCH_X86 */
