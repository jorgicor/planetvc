#include "crypt.h"
#include "cbase/kassert.h"
#include <string.h>
#include <limits.h>

enum {
	N = 32
};

static const int s_table[N] = {
	30, 3, 6, 25, 14, 7, 21, 0, 9, 31,
	1, 13, 19, 2, 29, 26, 23, 16, 4, 8,
	22, 15, 5, 17, 10, 27, 18, 11, 24, 20, 
	28, 12
};

static int s_rtable[N];

int encrypt(int n)
{
	int i;
	unsigned int k, r, un;

	un = n;
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
	return r;
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
		s_rtable[s_table[i]] = i;
	}
#if 0
	for (i = 0; i < INT_MAX; i++) {
		printf("%d %d %d\n", i, encrypt(i), decrypt(encrypt(i)));
		kasserta(i == decrypt(encrypt(i)));
	}
#endif
}
