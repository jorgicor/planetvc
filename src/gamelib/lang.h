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
