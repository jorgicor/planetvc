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

#ifndef KASSERT_H
#define KASSERT_H

#ifndef STDARG_H
#define STDARG_H
#include <stdarg.h>
#endif

#ifndef STDIO_H
#define STDIO_H
#include <stdio.h>
#endif

typedef void (*kassert_log_fun_t)(const char *msg);

void kassert_set_log_fun(kassert_log_fun_t pf);
void kassert_set_log_file(FILE *fp);
int kassert_imp(int cond, const char *condstr, const char *fname,
		int lineno, const char *funcname);
void kasserta_imp(int cond, const char *condstr, const char *fname,
		  int lineno, const char *funcname);
void ktrace(const char *fmt, ...);
void ktracev(const char *fmt, va_list alist);

#define kassert(cond) \
	kassert_imp(cond, #cond, __FILE__, __LINE__, __func__)
#define kassert_fails(cond) \
       	!kassert_imp(cond, #cond, __FILE__, __LINE__, __func__)
#define kasserta(cond) \
	kasserta_imp(cond, #cond, __FILE__, __LINE__, __func__)

void kassert_init(void);

#endif
