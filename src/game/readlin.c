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

#include "readlin.h"
#include <ctype.h>
#include <stdio.h>

static int isascii_space(int c)
{
	return c > 0 && c < 128 && isspace(c);
}

/* Reads a line of max READLIN_LINESZ - 1 characters.
 * The new line character is not added.
 * Whitespace trimmed from both ends, and the string is \0 terminated.
 * The two characters \n are changed by one new line character.
 * Returns -1 if EOF, or ~ is found alone in a line.
 * Returns the length of the string (like calling strlen(line)).
 */
int readlin(FILE *fp, char line[])
{
	int c, i, n, lastc;

	i = 0;
	lastc = ' ';
	while ((c = getc(fp)) != EOF) {
		if (c == '\n')
			break;
		if (c == '\r')
			continue;
		if (i == 0 && isascii_space(c))
			continue;
		if (i < READLIN_LINESZ - 1) {
			if (lastc == '\\' && c == 'n') {
				line[i - 1] = '\n';
			} else {
				line[i++] = c;
			}
			lastc = c;
		}
	}

	/* trim at end */
	n = i - 1;
	while (n >= 0 && isascii_space(line[n])) {
		n--;
	}
	n++;
	line[n] = '\0';

	if (n == 1 && line[0] == '~')
		return -1;
	else
		return n;
}
