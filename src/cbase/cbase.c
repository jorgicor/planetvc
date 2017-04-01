/*
Copyright (c) 2014-2017 Jorge Giner Cordero

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

#include "cbase.h"
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

int imul_overflows_int(int a, int b)
{
	assert(a >= 0 && b >= 0);
	if (a == 0 || b == 0)
		return 0;
	else
		return (INT_MAX / b) < a;
}

int uimul_overflows_int(unsigned int a, unsigned int b)
{
	if (a == 0 || b == 0)
		return 0;
	else if (a > (unsigned int) INT_MAX || b > (unsigned int) INT_MAX)
		return 1;
	else
	       	return imul_overflows_int((int) a, (int) b);
}

int imul_overflows_uint(int a, int b)
{
	assert(a >= 0 && b >= 0);
	if (a == 0 || b == 0)
		return 0;
	else
		return (UINT_MAX / b) < (unsigned int) a;
}

int iadd_overflows_int(int a, int b)
{
	assert(a >= 0 && b >= 0);
	return (INT_MAX - b) < a;
}

int iabs(int n)
{
	if (n == INT_MIN)
		return INT_MAX;
	else if (n < 0)
		return -n;
	else
		return n;
}

int isign(int n)
{
	if (n == 0)
		return 0;
	else if (n > 0)
		return 1;
	else
		return -1;
}

char *dupstr(const char *s)
{
	size_t n;
	char *q;

	if (s == NULL)
		return NULL;

	n = strlen(s);
	q = (char *) malloc(n + 1);
	if (q == NULL)
		return NULL;
	memcpy(q, s, n + 1);
	return q;
}

int ipow2ge(int n)
{
	unsigned int ui, um, un;
	
	if (n <= 1)
		return 1;

	un = n;
	um = (~((unsigned) INT_MAX)) >> 1;
	if (un >= um)
		return um;

	ui = 2;
	while (ui < un)
		ui <<= 1;

	return ui;
}

