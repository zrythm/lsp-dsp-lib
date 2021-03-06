/*
 * float.h
 *
 *  Created on: 31 мар. 2020 г.
 *      Author: sadko
 */

#ifndef LSP_PLUG_IN_DSP_COMMON_FLOAT_H_
#define LSP_PLUG_IN_DSP_COMMON_FLOAT_H_

#include <lsp-plug.in/dsp/common/types.h>

namespace lsp
{
    namespace dsp
    {
        /** Copy saturated data from source to destination.
         * Avoid +Inf, -Inf values by replacing with large values
         * Avoid +NaN, -NaN values by replacing with zeros
         *
         * @param dst destination pointer
         * @param src source pointer
         * @param count number of elements
         */
        LSP_DSP_LIB_IMPORT
        void (*copy_saturated)(float *dst, const float *src, size_t count);

        /** Saturate floating-point data in buffer
         * Avoid +Inf, -Inf values by replacing with large values
         * Avoid +NaN, -NaN values by replacing with zeros
         *
         * @param dst destination pointer
         * @param count number of elements
         */
        LSP_DSP_LIB_IMPORT
        void (* saturate)(float *dst, size_t count);

        /** Limit floating-point data in buffer
         * Avoid +Inf, -Inf values by replacing with 1.0 and -1.0
         * Avoid +NaN, -NaN values by replacing with zeros
         * Replace values greater than 1 with 1.0
         * Replace values less than -1 with -1.0
         *
         * @param dst destination pointer
         * @param src source pointer
         * @param count number of elements
         */
        LSP_DSP_LIB_IMPORT
        void (* limit_saturate1)(float *dst, size_t count);

        /** Limit floating-point data in buffer and copy do destination
         * Avoid +Inf, -Inf values by replacing with 1.0 and -1.0
         * Avoid +NaN, -NaN values by replacing with zeros
         * Replace values greater than 1 with 1.0
         * Replace values less than -1 with -1.0
         *
         * @param dst destination pointer
         * @param src source pointer
         * @param count number of elements
         */
        LSP_DSP_LIB_IMPORT
        void (* limit_saturate2)(float *dst, const float *src, size_t count);

        /**
         * Limit floating-point data in buffer
         * Replace +Inf with max, -Inf, +NaN, -NaN with min
         * Replace values greater than max with max,
         * Replace values less than min with min
         *
         * @param dst destination buffer
         * @param min minimum value
         * @param max maximum value
         * @param count number of samples
         */
        LSP_DSP_LIB_IMPORT
        void (* limit1)(float *dst, float min, float max, size_t count);

        /**
         * Limit floating-point data in buffer
         * Replace +Inf with max, -Inf, +NaN, -NaN with min
         * Replace values greater than max with max,
         * Replace values less than min with min
         *
         * @param dst destination buffer
         * @param src source buffer
         * @param min minimum value
         * @param max maximum value
         * @param count number of samples
         */
        LSP_DSP_LIB_IMPORT
        void (* limit2)(float *dst, const float *src, float min, float max, size_t count);

        /**
         * Sanitize floating-point samples from NaN's, Inf's and denormal values
         * Replaces the sanitized values with zeros
         *
         * @param dst destination buffer to sanitize
         * @param count number of samples
         */
        LSP_DSP_LIB_IMPORT
        void (* sanitize1)(float *dst, size_t count);

        /**
         * Sanitize floating-point samples from NaN's, Inf's and denormal values
         * Replaces the sanitized values with zeros
         *
         * @param dst destination buffer to sanitize
         * @param count number of samples
         */
        LSP_DSP_LIB_IMPORT
        void (* sanitize2)(float *dst, const float *src, size_t count);
    }
}

#endif /* LSP_PLUG_IN_DSP_COMMON_FLOAT_H_ */
