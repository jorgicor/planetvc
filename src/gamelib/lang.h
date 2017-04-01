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

#ifndef LANG_H
#define LANG_H

enum {
	LANG_NEQUIVS = 16
};

/* Tries to guess the OS language returning an ISO 639 two digit code.
 * Returns a pointer to a local string. No need to free.
 * Never returns NULL. If the language is not found, returns "en".
 */
const char *guess_lang(void);

/* Adds a language equivalence.
 * For example, if our game does not implement Catalan language but does
 * implement Spanish, and we know that catalans know Spanish, we do
 * add_lang_equivalence("ca", "es").
 * Then we can use get_lang_equivalence("ca") to get "es".
 * src and dst are not copied, so their data must remain.
 * Only a maximum of LANG_NEQUIVS can be added.
 */
void add_lang_equivalence(const char *src, const char *dst);

/* Returns the language equivalence for src, or src if none found. */
const char *get_lang_equivalence(const char *src);

/* Removes all equivalences. */
void reset_lang_equivalences(void);

void lang_init(void);

#endif
