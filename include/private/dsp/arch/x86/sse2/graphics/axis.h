/*
 * axis.h
 *
 *  Created on: 2 апр. 2020 г.
 *      Author: sadko
 */

#ifndef PRIVATE_DSP_ARCH_X86_SSE2_GRAPHICS_AXIS_H_
#define PRIVATE_DSP_ARCH_X86_SSE2_GRAPHICS_AXIS_H_

#ifndef PRIVATE_DSP_ARCH_X86_SSE2_IMPL
    #error "This header should not be included directly"
#endif /* PRIVATE_DSP_ARCH_X86_SSE2_IMPL */

namespace lsp
{
    namespace sse2
    {
        IF_ARCH_X86(
            static const uint32_t LOG_IARGS[] __lsp_aligned16 =
            {
                LSP_DSP_VEC4(0x7fffffff),           // X_SIGN
                LSP_DSP_VEC4(0x007fffff),           // X_MANT
                LSP_DSP_VEC4(0x0000007f)            // X_MMASK
            };

            static const float LOG_FARGS[] __lsp_aligned16 =
            {
                LSP_DSP_VEC4(1e-8),                  // X_AMP
                LSP_DSP_VEC4(0.5f),                  // X_HALF
                LSP_DSP_VEC4(M_SQRT1_2),             // SQRT1_2
                LSP_DSP_VEC4(1.0f),                  // ONE
                LSP_DSP_VEC4(7.0376836292E-2),       // C0
                LSP_DSP_VEC4(-1.1514610310E-1),      // C1
                LSP_DSP_VEC4(1.1676998740E-1),       // C2
                LSP_DSP_VEC4(-1.2420140846E-1),      // C3
                LSP_DSP_VEC4(+1.4249322787E-1),      // C4
                LSP_DSP_VEC4(-1.6668057665E-1),      // C5
                LSP_DSP_VEC4(+2.0000714765E-1),      // C6
                LSP_DSP_VEC4(-2.4999993993E-1),      // C7
                LSP_DSP_VEC4(+3.3333331174E-1),      // C8
                LSP_DSP_VEC4(-0.5f),                 // C9
                LSP_DSP_VEC4(M_LN2 - 2.12194440e-4)  // LXE + LN2
            };
        )

        void axis_apply_log1(float *x, const float *v, float zero, float norm_x, size_t count)
        {
            //---------------------------------------------------------------
            // Prepare constants
            ARCH_X86_ASM
            (
                __ASM_EMIT("movss       %[zero], %%xmm0")
                __ASM_EMIT("movss       %[norm_x], %%xmm1")

                __ASM_EMIT("sub         $4, %[count]")
                __ASM_EMIT("jb          2f")
                __ASM_EMIT("shufps      $0x00, %%xmm0, %%xmm0")         // xmm0 == zero
                __ASM_EMIT("shufps      $0x00, %%xmm1, %%xmm1")         // xmm1 == norm_x

                // Do x4 blocks
                __ASM_EMIT("1:")
                __ASM_EMIT("movups      (%[v]), %%xmm3")                // xmm3 = v
                __ASM_EMIT("andps       0x00 + %[ILOG], %%xmm3")        // xmm3 = abs(v)
                __ASM_EMIT("add         $0x10, %[v]")                   // v   += 4
                __ASM_EMIT("maxps       0x00 + %[FLOG], %%xmm3")        // xmm3 = max(X_AMP, abs(v)), ignores denormalized values
                __ASM_EMIT("mulps       %%xmm0, %%xmm3")                // xmm3 = max(X_AMP, abs(v)) * zero
                // Step 2: parse float value
                __ASM_EMIT("movdqa      %%xmm3, %%xmm4")                // xmm4 = v
                __ASM_EMIT("psrld       $23, %%xmm4")                   // xmm4 = frac(v)
                __ASM_EMIT("andps       0x10 + %[ILOG], %%xmm3")        // xmm3 = mant(v)
                __ASM_EMIT("psubd       0x20 + %[ILOG], %%xmm4")        // xmm4 = frac(v) - 127
                __ASM_EMIT("orps        0x10 + %[FLOG], %%xmm3")        // xmm3 = V = mant(v)+0.5
                __ASM_EMIT("cvtdq2ps    %%xmm4, %%xmm4")                // xmm4 = E = float(frac(v)-127)
                // Prepare logarithm approximation calculations
                __ASM_EMIT("movaps      %%xmm3, %%xmm5")                // xmm5 = V
                __ASM_EMIT("movaps      %%xmm3, %%xmm6")                // xmm6 = V
                __ASM_EMIT("cmpltps     0x20 + %[FLOG], %%xmm5")        // xmm5 = [ V < sqrt(1/2) ]
                __ASM_EMIT("movaps      0x30 + %[FLOG], %%xmm7")        // xmm7 = 1.0
                __ASM_EMIT("andps       %%xmm5, %%xmm6")                // xmm6 = V * [ V < sqrt(1/2) ]
                __ASM_EMIT("addps       %%xmm6, %%xmm3")                // xmm3 = V + V * [ V < sqrt(1/2) ]
                __ASM_EMIT("andnps      %%xmm7, %%xmm5")                // xmm5 = 1.0 * [ V >= sqrt(1/2) ]
                __ASM_EMIT("addps       %%xmm5, %%xmm4")                // xmm4 = E + 1.0 * [ V >= sqrt(1/2) ]      = B
                __ASM_EMIT("subps       %%xmm7, %%xmm3")                // xmm3 = V + V * [ V < sqrt(1/2) ] - 1.0   = A
                // Calculate logarithmic values
                __ASM_EMIT("movaps      0x40 + %[FLOG], %%xmm5")        // xmm5 = L0
                __ASM_EMIT("mulps       %%xmm3, %%xmm5")                // xmm5 = L0*A
                __ASM_EMIT("addps       0x50 + %[FLOG], %%xmm5")        // xmm5 = L1+L0*A
                __ASM_EMIT("mulps       %%xmm3, %%xmm5")                // xmm5 = A*(L1+L0*A)
                __ASM_EMIT("addps       0x60 + %[FLOG], %%xmm5")        // xmm5 = L2+A*(L1+L0*A)
                __ASM_EMIT("mulps       %%xmm3, %%xmm5")                // xmm5 = A*(L2+A*(L1+L0*A))
                __ASM_EMIT("addps       0x70 + %[FLOG], %%xmm5")        // xmm5 = L3+A*L2+A*(L1+L0*A)
                __ASM_EMIT("mulps       %%xmm3, %%xmm5")                // xmm5 = A*(L3+A*L2+A*(L1+L0*A))
                __ASM_EMIT("addps       0x80 + %[FLOG], %%xmm5")        // xmm5 = L4+A*(L3+A*L2+A*(L1+L0*A))
                __ASM_EMIT("mulps       %%xmm3, %%xmm5")                // xmm5 = A*(L4+A*(L3+A*L2+A*(L1+L0*A)))
                __ASM_EMIT("addps       0x90 + %[FLOG], %%xmm5")        // xmm5 = L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A)))
                __ASM_EMIT("mulps       %%xmm3, %%xmm5")                // xmm5 = A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))
                __ASM_EMIT("addps       0xa0 + %[FLOG], %%xmm5")        // xmm5 = L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))
                __ASM_EMIT("mulps       %%xmm3, %%xmm5")                // xmm5 = A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A)))))
                __ASM_EMIT("addps       0xb0 + %[FLOG], %%xmm5")        // xmm5 = L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A)))))
                __ASM_EMIT("mulps       %%xmm3, %%xmm5")                // xmm5 = A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))))
                __ASM_EMIT("addps       0xc0 + %[FLOG], %%xmm5")        // xmm5 = L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))))
                __ASM_EMIT("mulps       %%xmm3, %%xmm5")                // xmm5 = A*(L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A)))))))
                __ASM_EMIT("addps       0xd0 + %[FLOG], %%xmm5")        // xmm5 = A*(L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))))) + L9
                __ASM_EMIT("mulps       %%xmm3, %%xmm5")                // xmm5 = A*(A*(L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))))) + L9)
                __ASM_EMIT("mulps       %%xmm3, %%xmm5")                // xmm5 = A*A*(A*(L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))))) + L9)
                __ASM_EMIT("mulps       0xe0 + %[FLOG], %%xmm4")        // xmm4 = B*(LXE + LN2)
                __ASM_EMIT("addps       %%xmm4, %%xmm5")                // xmm5 = B*(LXE + LN2) + A*A*A*(L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A)))))))
                __ASM_EMIT("addps       %%xmm3, %%xmm5")                // xmm5 = B*(LXE + LN2) + A*A*A*(L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))))) + A
                // Apply values to axes
                __ASM_EMIT("movaps      %%xmm5, %%xmm3")                // xmm3 = log(abs(v*zero)), xmm5=log(abs(v*zero))
                __ASM_EMIT("movups      (%[x]), %%xmm4")                // xmm4 = x
                __ASM_EMIT("mulps       %%xmm1, %%xmm3")                // xmm3 = log(abs(v*zero)) * norm_x
                __ASM_EMIT("addps       %%xmm3, %%xmm4")                // xmm4 = x + log(abs(v*zero)) * norm_x
                __ASM_EMIT("movups      %%xmm4, (%[x])")
                __ASM_EMIT("add         $0x10, %[x]")
                __ASM_EMIT("sub         $4, %[count]")
                __ASM_EMIT("jae         1b")

                __ASM_EMIT("2:")
                __ASM_EMIT("add         $3, %[count]")
                __ASM_EMIT("jl          4f")

                // Do x1 blocks
                __ASM_EMIT("3:")
                __ASM_EMIT("movss       (%[v]), %%xmm3")                // xmm3 = v
                __ASM_EMIT("andps       0x00 + %[ILOG], %%xmm3")        // xmm3 = abs(v)
                __ASM_EMIT("add         $0x04, %[v]")                   // v   += 4
                __ASM_EMIT("maxps       0x00 + %[FLOG], %%xmm3")        // xmm3 = max(X_AMP, abs(v)), ignores denormalized values
                __ASM_EMIT("mulps       %%xmm0, %%xmm3")                // xmm5 = max(X_AMP, abs(v)) * zero
                // Step 2: parse float value
                __ASM_EMIT("movdqa      %%xmm3, %%xmm4")                // xmm4 = v
                __ASM_EMIT("psrld       $23, %%xmm4")                   // xmm4 = frac(v)
                __ASM_EMIT("andps       0x10 + %[ILOG], %%xmm3")        // xmm3 = mant(v)
                __ASM_EMIT("psubd       0x20 + %[ILOG], %%xmm4")        // xmm4 = frac(v) - 127
                __ASM_EMIT("orps        0x10 + %[FLOG], %%xmm3")        // xmm3 = V = mant(v)+0.5
                __ASM_EMIT("cvtdq2ps    %%xmm4, %%xmm4")                // xmm4 = E = float(frac(v)-127)
                // Prepare logarithm approximation calculations
                __ASM_EMIT("movaps      %%xmm3, %%xmm5")                // xmm5 = V
                __ASM_EMIT("movaps      %%xmm3, %%xmm6")                // xmm6 = V
                __ASM_EMIT("cmpltps     0x20 + %[FLOG], %%xmm5")        // xmm5 = [ V < sqrt(1/2) ]
                __ASM_EMIT("movaps      0x30 + %[FLOG], %%xmm7")        // xmm7 = 1.0
                __ASM_EMIT("andps       %%xmm5, %%xmm6")                // xmm6 = V * [ V < sqrt(1/2) ]
                __ASM_EMIT("addps       %%xmm6, %%xmm3")                // xmm6 = V + V * [ V < sqrt(1/2) ]
                __ASM_EMIT("andnps      %%xmm7, %%xmm5")                // xmm5 = 1.0 * [ V >= sqrt(1/2) ]
                __ASM_EMIT("addps       %%xmm5, %%xmm4")                // xmm4 = E + 1.0 * [ V >= sqrt(1/2) ]      = B
                __ASM_EMIT("subps       %%xmm7, %%xmm3")                // xmm3 = V + V * [ V < sqrt(1/2) ] - 1.0   = A
                // Calculate logarithmic values
                __ASM_EMIT("movaps      0x40 + %[FLOG], %%xmm5")        // xmm5 = L0
                __ASM_EMIT("mulss       %%xmm3, %%xmm5")                // xmm5 = L0*A
                __ASM_EMIT("addss       0x50 + %[FLOG], %%xmm5")        // xmm5 = L1+L0*A
                __ASM_EMIT("mulss       %%xmm3, %%xmm5")                // xmm5 = A*(L1+L0*A)
                __ASM_EMIT("addss       0x60 + %[FLOG], %%xmm5")        // xmm5 = L2+A*(L1+L0*A)
                __ASM_EMIT("mulss       %%xmm3, %%xmm5")                // xmm5 = A*(L2+A*(L1+L0*A))
                __ASM_EMIT("addss       0x70 + %[FLOG], %%xmm5")        // xmm5 = L3+A*L2+A*(L1+L0*A)
                __ASM_EMIT("mulss       %%xmm3, %%xmm5")                // xmm5 = A*(L3+A*L2+A*(L1+L0*A))
                __ASM_EMIT("addss       0x80 + %[FLOG], %%xmm5")        // xmm5 = L4+A*(L3+A*L2+A*(L1+L0*A))
                __ASM_EMIT("mulss       %%xmm3, %%xmm5")                // xmm5 = A*(L4+A*(L3+A*L2+A*(L1+L0*A)))
                __ASM_EMIT("addss       0x90 + %[FLOG], %%xmm5")        // xmm5 = L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A)))
                __ASM_EMIT("mulss       %%xmm3, %%xmm5")                // xmm5 = A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))
                __ASM_EMIT("addss       0xa0 + %[FLOG], %%xmm5")        // xmm5 = L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))
                __ASM_EMIT("mulss       %%xmm3, %%xmm5")                // xmm5 = A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A)))))
                __ASM_EMIT("addss       0xb0 + %[FLOG], %%xmm5")        // xmm5 = L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A)))))
                __ASM_EMIT("mulss       %%xmm3, %%xmm5")                // xmm5 = A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))))
                __ASM_EMIT("addss       0xc0 + %[FLOG], %%xmm5")        // xmm5 = L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))))
                __ASM_EMIT("mulss       %%xmm3, %%xmm5")                // xmm5 = A*(L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A)))))))
                __ASM_EMIT("addss       0xd0 + %[FLOG], %%xmm5")        // xmm5 = A*(L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))))) + L9
                __ASM_EMIT("mulss       %%xmm3, %%xmm5")                // xmm5 = A*(A*(L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))))) + L9)
                __ASM_EMIT("mulss       %%xmm3, %%xmm5")                // xmm5 = A*A*(A*(L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))))) + L9)
                __ASM_EMIT("mulss       0xe0 + %[FLOG], %%xmm4")        // xmm4 = B*(LXE + LN2)
                __ASM_EMIT("addss       %%xmm4, %%xmm5")                // xmm5 = B*(LXE + LN2) + A*A*A*(L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A)))))))
                __ASM_EMIT("addss       %%xmm3, %%xmm5")                // xmm5 = B*(LXE + LN2) + A*A*A*(L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))))) + A
                // Apply values to axes
                __ASM_EMIT("movaps      %%xmm5, %%xmm3")                // xmm3 = log(abs(v*zero)), xmm5=log(abs(v*zero))
                __ASM_EMIT("movss       (%[x]), %%xmm4")                // xmm4 = x
                __ASM_EMIT("mulps       %%xmm1, %%xmm3")                // xmm3 = log(abs(v*zero)) * norm_x
                __ASM_EMIT("addps       %%xmm3, %%xmm4")                // xmm4 = x + log(abs(v*zero)) * norm_x
                __ASM_EMIT("movss       %%xmm4, (%[x])")
                __ASM_EMIT("add         $0x04, %[x]")
                __ASM_EMIT("dec         %[count]")
                __ASM_EMIT("jge         3b")

                __ASM_EMIT("4:")

                : [v] "+r" (v), [x] "+r" (x), [count] "+r" (count)
                : [zero] "m" (zero), [norm_x] "m" (norm_x),
                  [ILOG] "o" (LOG_IARGS), [FLOG] "o" (LOG_FARGS)
                : "%xmm0", "%xmm1", "%xmm2", "%xmm3",
                  "%xmm4", "%xmm5", "%xmm6", "%xmm7"
            );
        }

        void axis_apply_log2(float *x, float *y, const float *v, float zero, float norm_x, float norm_y, size_t count)
        {
            //---------------------------------------------------------------
            // Prepare constants
            ARCH_X86_ASM
            (
                __ASM_EMIT("movss       %[zero], %%xmm0")
                __ASM_EMIT("movss       %[norm_x], %%xmm1")
                __ASM_EMIT("movss       %[norm_y], %%xmm2")

                __ASM_EMIT("sub         $4, %[count]")
                __ASM_EMIT("jb          2f")
                __ASM_EMIT("shufps      $0x00, %%xmm0, %%xmm0")         // xmm0 == zero
                __ASM_EMIT("shufps      $0x00, %%xmm1, %%xmm1")         // xmm1 == norm_x
                __ASM_EMIT("shufps      $0x00, %%xmm2, %%xmm2")         // xmm2 == norm_y

                // Do x4 blocks
                __ASM_EMIT("1:")
                __ASM_EMIT("movups      (%[v]), %%xmm3")                // xmm3 = v
                __ASM_EMIT("andps       0x00 + %[ILOG], %%xmm3")        // xmm3 = abs(v)
                __ASM_EMIT("add         $0x10, %[v]")                   // v   += 4
                __ASM_EMIT("maxps       0x00 + %[FLOG], %%xmm3")        // xmm3 = max(X_AMP, abs(v)), ignores denormalized values
                __ASM_EMIT("mulps       %%xmm0, %%xmm3")                // xmm3 = max(X_AMP, abs(v)) * zero
                // Step 2: parse float value
                __ASM_EMIT("movdqa      %%xmm3, %%xmm4")                // xmm4 = v
                __ASM_EMIT("psrld       $23, %%xmm4")                   // xmm4 = frac(v)
                __ASM_EMIT("andps       0x10 + %[ILOG], %%xmm3")        // xmm3 = mant(v)
                __ASM_EMIT("psubd       0x20 + %[ILOG], %%xmm4")        // xmm4 = frac(v) - 127
                __ASM_EMIT("orps        0x10 + %[FLOG], %%xmm3")        // xmm3 = V = mant(v)+0.5
                __ASM_EMIT("cvtdq2ps    %%xmm4, %%xmm4")                // xmm4 = E = float(frac(v)-127)
                // Prepare logarithm approximation calculations
                __ASM_EMIT("movaps      %%xmm3, %%xmm5")                // xmm5 = V
                __ASM_EMIT("movaps      %%xmm3, %%xmm6")                // xmm6 = V
                __ASM_EMIT("cmpltps     0x20 + %[FLOG], %%xmm5")        // xmm5 = [ V < sqrt(1/2) ]
                __ASM_EMIT("movaps      0x30 + %[FLOG], %%xmm7")        // xmm7 = 1.0
                __ASM_EMIT("andps       %%xmm5, %%xmm6")                // xmm6 = V * [ V < sqrt(1/2) ]
                __ASM_EMIT("addps       %%xmm6, %%xmm3")                // xmm3 = V + V * [ V < sqrt(1/2) ]
                __ASM_EMIT("andnps      %%xmm7, %%xmm5")                // xmm5 = 1.0 * [ V >= sqrt(1/2) ]
                __ASM_EMIT("addps       %%xmm5, %%xmm4")                // xmm4 = E + 1.0 * [ V >= sqrt(1/2) ]      = B
                __ASM_EMIT("subps       %%xmm7, %%xmm3")                // xmm3 = V + V * [ V < sqrt(1/2) ] - 1.0   = A
                // Calculate logarithmic values
                __ASM_EMIT("movaps      0x40 + %[FLOG], %%xmm5")        // xmm5 = L0
                __ASM_EMIT("mulps       %%xmm3, %%xmm5")                // xmm5 = L0*A
                __ASM_EMIT("addps       0x50 + %[FLOG], %%xmm5")        // xmm5 = L1+L0*A
                __ASM_EMIT("mulps       %%xmm3, %%xmm5")                // xmm5 = A*(L1+L0*A)
                __ASM_EMIT("addps       0x60 + %[FLOG], %%xmm5")        // xmm5 = L2+A*(L1+L0*A)
                __ASM_EMIT("mulps       %%xmm3, %%xmm5")                // xmm5 = A*(L2+A*(L1+L0*A))
                __ASM_EMIT("addps       0x70 + %[FLOG], %%xmm5")        // xmm5 = L3+A*L2+A*(L1+L0*A)
                __ASM_EMIT("mulps       %%xmm3, %%xmm5")                // xmm5 = A*(L3+A*L2+A*(L1+L0*A))
                __ASM_EMIT("addps       0x80 + %[FLOG], %%xmm5")        // xmm5 = L4+A*(L3+A*L2+A*(L1+L0*A))
                __ASM_EMIT("mulps       %%xmm3, %%xmm5")                // xmm5 = A*(L4+A*(L3+A*L2+A*(L1+L0*A)))
                __ASM_EMIT("addps       0x90 + %[FLOG], %%xmm5")        // xmm5 = L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A)))
                __ASM_EMIT("mulps       %%xmm3, %%xmm5")                // xmm5 = A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))
                __ASM_EMIT("addps       0xa0 + %[FLOG], %%xmm5")        // xmm5 = L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))
                __ASM_EMIT("mulps       %%xmm3, %%xmm5")                // xmm5 = A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A)))))
                __ASM_EMIT("addps       0xb0 + %[FLOG], %%xmm5")        // xmm5 = L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A)))))
                __ASM_EMIT("mulps       %%xmm3, %%xmm5")                // xmm5 = A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))))
                __ASM_EMIT("addps       0xc0 + %[FLOG], %%xmm5")        // xmm5 = L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))))
                __ASM_EMIT("mulps       %%xmm3, %%xmm5")                // xmm5 = A*(L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A)))))))
                __ASM_EMIT("addps       0xd0 + %[FLOG], %%xmm5")        // xmm5 = A*(L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))))) + L9
                __ASM_EMIT("mulps       %%xmm3, %%xmm5")                // xmm5 = A*(A*(L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))))) + L9)
                __ASM_EMIT("mulps       %%xmm3, %%xmm5")                // xmm5 = A*A*(A*(L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))))) + L9)
                __ASM_EMIT("mulps       0xe0 + %[FLOG], %%xmm4")        // xmm4 = B*(LXE + LN2)
                __ASM_EMIT("addps       %%xmm4, %%xmm5")                // xmm5 = B*(LXE + LN2) + A*A*A*(L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A)))))))
                __ASM_EMIT("addps       %%xmm3, %%xmm5")                // xmm5 = B*(LXE + LN2) + A*A*A*(L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))))) + A
                // Apply values to axes
                __ASM_EMIT("movaps      %%xmm5, %%xmm3")                // xmm3 = log(abs(v*zero)), xmm5=log(abs(v*zero))
                __ASM_EMIT("movups      (%[x]), %%xmm4")                // xmm4 = x
                __ASM_EMIT("movups      (%[y]), %%xmm6")                // xmm6 = y
                __ASM_EMIT("mulps       %%xmm1, %%xmm3")                // xmm3 = log(abs(v*zero)) * norm_x
                __ASM_EMIT("mulps       %%xmm2, %%xmm5")                // xmm5 = log(abs(v*zero)) * norm_y
                __ASM_EMIT("addps       %%xmm3, %%xmm4")                // xmm4 = x + log(abs(v*zero)) * norm_x
                __ASM_EMIT("addps       %%xmm5, %%xmm6")                // xmm6 = y + log(abs(v*zero)) * norm_y
                __ASM_EMIT("movups      %%xmm4, (%[x])")
                __ASM_EMIT("movups      %%xmm6, (%[y])")
                __ASM_EMIT("add         $0x10, %[x]")
                __ASM_EMIT("add         $0x10, %[y]")
                __ASM_EMIT("sub         $4, %[count]")
                __ASM_EMIT("jae         1b")

                __ASM_EMIT("2:")
                __ASM_EMIT("add         $3, %[count]")
                __ASM_EMIT("jl          4f")

                // Do x1 blocks
                __ASM_EMIT("3:")
                __ASM_EMIT("movss       (%[v]), %%xmm3")                // xmm3 = v
                __ASM_EMIT("andps       0x00 + %[ILOG], %%xmm3")        // xmm3 = abs(v)
                __ASM_EMIT("add         $0x04, %[v]")                   // v   += 4
                __ASM_EMIT("maxps       0x00 + %[FLOG], %%xmm3")        // xmm3 = max(X_AMP, abs(v)), ignores denormalized values
                __ASM_EMIT("mulps       %%xmm0, %%xmm3")                // xmm5 = max(X_AMP, abs(v)) * zero
                // Step 2: parse float value
                __ASM_EMIT("movdqa      %%xmm3, %%xmm4")                // xmm4 = v
                __ASM_EMIT("psrld       $23, %%xmm4")                   // xmm4 = frac(v)
                __ASM_EMIT("andps       0x10 + %[ILOG], %%xmm3")        // xmm3 = mant(v)
                __ASM_EMIT("psubd       0x20 + %[ILOG], %%xmm4")        // xmm4 = frac(v) - 127
                __ASM_EMIT("orps        0x10 + %[FLOG], %%xmm3")        // xmm3 = V = mant(v)+0.5
                __ASM_EMIT("cvtdq2ps    %%xmm4, %%xmm4")                // xmm4 = E = float(frac(v)-127)
                // Prepare logarithm approximation calculations
                __ASM_EMIT("movaps      %%xmm3, %%xmm5")                // xmm5 = V
                __ASM_EMIT("movaps      %%xmm3, %%xmm6")                // xmm6 = V
                __ASM_EMIT("cmpltps     0x20 + %[FLOG], %%xmm5")        // xmm5 = [ V < sqrt(1/2) ]
                __ASM_EMIT("movaps      0x30 + %[FLOG], %%xmm7")        // xmm7 = 1.0
                __ASM_EMIT("andps       %%xmm5, %%xmm6")                // xmm6 = V * [ V < sqrt(1/2) ]
                __ASM_EMIT("addps       %%xmm6, %%xmm3")                // xmm6 = V + V * [ V < sqrt(1/2) ]
                __ASM_EMIT("andnps      %%xmm7, %%xmm5")                // xmm5 = 1.0 * [ V >= sqrt(1/2) ]
                __ASM_EMIT("addps       %%xmm5, %%xmm4")                // xmm4 = E + 1.0 * [ V >= sqrt(1/2) ]      = B
                __ASM_EMIT("subps       %%xmm7, %%xmm3")                // xmm3 = V + V * [ V < sqrt(1/2) ] - 1.0   = A
                // Calculate logarithmic values
                __ASM_EMIT("movaps      0x40 + %[FLOG], %%xmm5")        // xmm5 = L0
                __ASM_EMIT("mulss       %%xmm3, %%xmm5")                // xmm5 = L0*A
                __ASM_EMIT("addss       0x50 + %[FLOG], %%xmm5")        // xmm5 = L1+L0*A
                __ASM_EMIT("mulss       %%xmm3, %%xmm5")                // xmm5 = A*(L1+L0*A)
                __ASM_EMIT("addss       0x60 + %[FLOG], %%xmm5")        // xmm5 = L2+A*(L1+L0*A)
                __ASM_EMIT("mulss       %%xmm3, %%xmm5")                // xmm5 = A*(L2+A*(L1+L0*A))
                __ASM_EMIT("addss       0x70 + %[FLOG], %%xmm5")        // xmm5 = L3+A*L2+A*(L1+L0*A)
                __ASM_EMIT("mulss       %%xmm3, %%xmm5")                // xmm5 = A*(L3+A*L2+A*(L1+L0*A))
                __ASM_EMIT("addss       0x80 + %[FLOG], %%xmm5")        // xmm5 = L4+A*(L3+A*L2+A*(L1+L0*A))
                __ASM_EMIT("mulss       %%xmm3, %%xmm5")                // xmm5 = A*(L4+A*(L3+A*L2+A*(L1+L0*A)))
                __ASM_EMIT("addss       0x90 + %[FLOG], %%xmm5")        // xmm5 = L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A)))
                __ASM_EMIT("mulss       %%xmm3, %%xmm5")                // xmm5 = A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))
                __ASM_EMIT("addss       0xa0 + %[FLOG], %%xmm5")        // xmm5 = L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))
                __ASM_EMIT("mulss       %%xmm3, %%xmm5")                // xmm5 = A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A)))))
                __ASM_EMIT("addss       0xb0 + %[FLOG], %%xmm5")        // xmm5 = L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A)))))
                __ASM_EMIT("mulss       %%xmm3, %%xmm5")                // xmm5 = A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))))
                __ASM_EMIT("addss       0xc0 + %[FLOG], %%xmm5")        // xmm5 = L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))))
                __ASM_EMIT("mulss       %%xmm3, %%xmm5")                // xmm5 = A*(L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A)))))))
                __ASM_EMIT("addss       0xd0 + %[FLOG], %%xmm5")        // xmm5 = A*(L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))))) + L9
                __ASM_EMIT("mulss       %%xmm3, %%xmm5")                // xmm5 = A*(A*(L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))))) + L9)
                __ASM_EMIT("mulss       %%xmm3, %%xmm5")                // xmm5 = A*A*(A*(L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))))) + L9)
                __ASM_EMIT("mulss       0xe0 + %[FLOG], %%xmm4")        // xmm4 = B*(LXE + LN2)
                __ASM_EMIT("addss       %%xmm4, %%xmm5")                // xmm5 = B*(LXE + LN2) + A*A*A*(L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A)))))))
                __ASM_EMIT("addss       %%xmm3, %%xmm5")                // xmm5 = B*(LXE + LN2) + A*A*A*(L8+A*(L7+A*(L6+A*(L5+A*(L4+A*(L3+A*L2+A*(L1+L0*A))))))) + A
                // Apply values to axes
                __ASM_EMIT("movaps      %%xmm5, %%xmm3")                // xmm3 = log(abs(v*zero)), xmm5=log(abs(v*zero))
                __ASM_EMIT("movss       (%[x]), %%xmm4")                // xmm4 = x
                __ASM_EMIT("movss       (%[y]), %%xmm6")                // xmm6 = y
                __ASM_EMIT("mulps       %%xmm1, %%xmm3")                // xmm3 = log(abs(v*zero)) * norm_x
                __ASM_EMIT("mulps       %%xmm2, %%xmm5")                // xmm5 = log(abs(v*zero)) * norm_y
                __ASM_EMIT("addps       %%xmm3, %%xmm4")                // xmm4 = x + log(abs(v*zero)) * norm_x
                __ASM_EMIT("addps       %%xmm5, %%xmm6")                // xmm6 = y + log(abs(v*zero)) * norm_y
                __ASM_EMIT("movss       %%xmm4, (%[x])")
                __ASM_EMIT("movss       %%xmm6, (%[y])")
                __ASM_EMIT("add         $0x04, %[x]")
                __ASM_EMIT("add         $0x04, %[y]")
                __ASM_EMIT("dec         %[count]")
                __ASM_EMIT("jge         3b")

                __ASM_EMIT("4:")

                : [v] "+r" (v), [x] "+r" (x) , [y] "+r" (y),
                  [count] "+r" (count)
                : [zero] "m" (zero), [norm_x] "m" (norm_x), [norm_y] "m" (norm_y),
                  [ILOG] "o" (LOG_IARGS), [FLOG] "o" (LOG_FARGS)
                : "%xmm0", "%xmm1", "%xmm2", "%xmm3",
                  "%xmm4", "%xmm5", "%xmm6", "%xmm7"
            );
        }
    }
}

#endif /* PRIVATE_DSP_ARCH_X86_SSE2_GRAPHICS_AXIS_H_ */
