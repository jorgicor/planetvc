/*
Copyright (c) 2016-2017 Jorge Giner Cordero

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

#include "crypt.h"
#include "cbase/kassert.h"
#include <string.h>
#include <limits.h>

enum {
	N = 32
};

static const unsigned char s_table[N] = {
	30, 3, 6, 25, 14, 7, 21, 0, 9, 31,
	1, 13, 19, 2, 29, 26, 23, 16, 4, 8,
	22, 15, 5, 17, 10, 27, 18, 11, 24, 20, 
	28, 12
};

static unsigned char s_rtable[N];
const unsigned int XOR = 0xC69BA35Eu;

int encrypt(int n)
{
	int i;
	unsigned int k, r, un;

	un = n;
	un ^= XOR;
	k = 1;
	r = 0;
	for (i = 0; i < N; i++, k <<= 1) {
		if (un & k) {
			r |= 1 << s_table[i];
		}
	}
	return r;
}

int decrypt(int n)
{
	int i;
	unsigned int k, r, un;

	un = n;
	k = 1;
	r = 0;
	for (i = 0; i < N; i++, k <<= 1) {
		if (un & k) {
			r |= 1 << s_rtable[i];
		}
	}
	return r ^ XOR;
}

static void check_no_duplicates(void)
{
	int i;
	int chk[N];

	memset(chk, 0, sizeof(chk));
	for (i = 0; i < N; i++) {
		kasserta(chk[s_table[i]] == 0);
		chk[s_table[i]] = 1;
	}
}

void crypt_init(void)
{
	int i;

	check_no_duplicates();
	
	for (i = 0; i < N; i++) {
		s_rtable[s_table[i]] = (unsigned char) i;
	}
#if 0
	for (i = 0; i < INT_MAX; i++) {
		printf("%d %d %d\n", i, encrypt(i), decrypt(encrypt(i)));
		kasserta(i == decrypt(encrypt(i)));
	}
#endif
}
