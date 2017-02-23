/*
Copyright (c) 2016 Jorge Giner Cordero

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

#include "kassert.h"
#include "cbase.h"

#ifndef STDLIB_H
#	define STDLIB_H
#	include <stdlib.h>
#endif

#ifndef va_copy
#	define va_copy(dst, src) memcpy(&dst, &src, sizeof(va_list))
#endif

static FILE *s_fp;
static kassert_log_fun_t s_pf;

void kassert_set_log_file(FILE *fp)
{
	s_fp = fp;
}

void kassert_set_log_fun(kassert_log_fun_t pf)
{
	s_pf = pf;
}

void ktracev(const char *fmt, va_list ap)
{
	va_list ap2;
	char buffer[4096];

	if (fmt == NULL)
		return;

	va_copy(ap2, ap);
	if (s_fp != NULL) {
		vfprintf(s_fp, fmt, ap);
		fputc('\n', s_fp);
		fflush(s_fp);
	}
	if (s_pf != NULL) {
		vsnprintf(buffer, sizeof(buffer), fmt, ap2);
		s_pf(buffer);
	}
	va_end(ap2);
}

void ktrace(const char *fmt, ...)
{
	va_list ap;

	if (fmt == NULL)
		return;

	va_start(ap, fmt);
	ktracev(fmt, ap);
	va_end(ap);
}

int kassert_imp(int cond, const char *condstr, const char *fname,
		int lineno, const char *funcname)
{
	if (!cond) {
		ktrace("%s:%d: %s: failed (%s)\n", fname, lineno,
		       funcname, condstr);
		return 0;
	}
	return 1;
}

void kasserta_imp(int cond, const char *condstr, const char *fname,
		  int lineno, const char *funcname)
{
	if (!cond) {
		ktrace("%s:%d: %s: failed (%s)\n", fname, lineno,
		       funcname, condstr);
		abort();
	}
}

void kassert_init(void)
{
	s_fp = NULL;
	s_pf = NULL;
}

