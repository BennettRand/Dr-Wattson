#ifndef INT_SQRT_H
#define INT_SQRT_H

#include <stdint.h>

/**
 * \brief    Fast Square root algorithm
 * Fractional parts of the answer are discarded. That is:
 *      - SquareRoot(3) --> 1
 *      - SquareRoot(4) --> 2
 *      - SquareRoot(5) --> 2
 *      - SquareRoot(8) --> 2
 *      - SquareRoot(9) --> 3
 *
 * \param[in] a_nInput - unsigned integer for which to find the square root
 *
 * \return Integer square root of the input value.
 */
uint32_t int_sqrt(uint32_t a_nInput);

#endif
