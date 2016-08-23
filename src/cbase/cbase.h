/*
Copyright (c) 2014, 2015, 2016 Jorge Giner Cordero

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef CBASE_H
#define CBASE_H

/*
 * Number of elements in an array.
 */
#define NELEMS(v) (sizeof(v) / sizeof(v[0]))

/*
 * Arithmetic shift right.
 * This is undefined by C. If the >> operation in the current platform
 * does not perform arithmetic shift (conserving sign bit) we will
 * have to implement it. For now we assume it is.
 */
#define asr(n, bits)	((n) >> (bits))

/*
 * 1 if the multiplication of two non-negative [0..INT_MAX] 'signed int' will
 * overflow INT_MAX.
 */
int imul_overflows_int(int a, int b);

/*
 * 1 if the multiplication of two non-negative [0..INT_MAX] 'signed int' as
 * unsigned will overflow UINT_MAX.
 */
int imul_overflows_uint(int a, int b);

/*
 * 1 if the multiplication of two 'unsigned int' will overflow INT_MAX.
 */
int uimul_overflows_int(unsigned int a, unsigned int b);

/*
 * 1 if the sum of two non-negative [0..INT_MAX] 'signed int' will overflow
 * INT_MAX.
 */
int iadd_overflows_int(int a, int b);

/*
 * Returns the abs of n.
 * In case n is INT_MIN, returns INT_MAX (which is not correct).
 */
int iabs(int n);

/* Returns the sign of n: -1, 0, 1. */
int isign(int n);

/* Duplicates the string s, or returns NULL if not enough memory or
 * s is NULL.
 */
char *dupstr(const char *s);

/*
 * Given an integer n, returns the first power of two equal or greater
 * than n.
 * If n <= 1 returns 1.
 * If n > 'the first power of 2 less than
 * INT_MAX' (for 32 bit signed integers, this is 2^30), returns that
 * maximum integer.
 */
int ipow2ge(int n);

#endif
