/*
 * xcr.h
 *
 *  Created on: 1 апр. 2020 г.
 *      Author: sadko
 */

#ifndef PRIVATE_DSP_ARCH_X86_AVX_XCR_H_
#define PRIVATE_DSP_ARCH_X86_AVX_XCR_H_

#ifndef PRIVATE_DSP_ARCH_X86_AVX_IMPL
    #error "This header should not be included directly"
#endif /* PRIVATE_DSP_ARCH_X86_AVX_IMPL */

namespace lsp
{
    namespace x86
    {
    #ifdef ARCH_X86_AVX
        uint64_t read_xcr(umword_t xcr_id)
        {
            uint64_t xcr;

            ARCH_X86_ASM
            (
                __ASM_EMIT64("xor       %%rax, %%rax")
                __ASM_EMIT("xgetbv")
                __ASM_EMIT64("shl       $32, %%rdx")
                __ASM_EMIT64("or        %%rdx, %%rax")
                : __IF_32("=A" (xcr)) __IF_64("=a" (xcr))
                : "c" (xcr_id)
                : __IF_64("%rdx")
            );
            return xcr;
        }
    #else
        uint64_t read_xcr(umword_t xcr_id)
        {
            return 0;
        }
    #endif /* ARCH_X86_AVX */
    }
}

#endif /* PRIVATE_DSP_ARCH_X86_AVX_XCR_H_ */
