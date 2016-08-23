#include "lang.h"
#include "cbase/cbase.h"
#include "cbase/kassert.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

struct langstr_info {
	const char *langstr;
	const char *isocode;
};

/* this table must be sorted to do binary search. */
static const struct langstr_info s_langstr_map[] = {
	{ "abkhazian", "ab" },
	{ "afar", "aa" },
	{ "afrikaans", "af" },
	{ "akan", "ak" },
	{ "albanian", "sq" },
	{ "amharic", "am" },
	{ "arabic", "ar" },
	{ "aragonese", "an" },
	{ "armenian", "hy" },
	{ "assamese", "as" },
	{ "avaric", "av" },
	{ "avestan", "ae" },
	{ "aymara", "ay" },
	{ "azerbaijany", "az" },
	{ "azery", "az" }, /**/
	{ "bambara", "bm" },
	{ "bashkir", "ba" },
	{ "basque", "eu" },
	{ "belarusian", "be" },
	{ "bengali", "bn" },
	{ "bislama", "bi" },
	{ "bosnian", "bs" },
	{ "breton", "br" },
	{ "bulgarian", "bg" },
	{ "burmese", "my" },
	{ "castilian", "es" },
	{ "catalan", "ca" },
	{ "chamorro", "ch" },
	{ "chechen", "ce" },
	{ "chewa", "ny" },
	{ "chichewa", "ny" },
	{ "chinese", "zh" },
	{ "chuang", "za" },
	{ "chuvash", "cv" },
	{ "cornish", "kw" },
	{ "corsican", "co" },
	{ "cree", "cr" },
	{ "croatian", "hr" },
	{ "czech", "cs" },
	{ "danish", "da" },
	{ "divehi", "dv" },
	{ "dhivehi", "dv" },
	{ "dutch", "nl" },
	{ "dzongkha", "dz" },
	{ "esperanto", "eo" },
	{ "estonian", "et" },
	{ "ewe", "ee" },
	{ "faroese", "fo" },
	{ "fijian", "fj" },
	{ "finnish", "fi" },
	{ "flemish", "nl" },
	{ "french", "fr" },
	{ "fulah", "ff" },
	{ "gaelic", "gd" },
	{ "galician", "gl" },
	{ "ganda", "lg" },
	{ "georgian", "ka" },
	{ "german", "de" },
	{ "gikuyu", "ki" },
	{ "greek", "el" },
	{ "greenlandic", "kl" },
	{ "guarani", "gn" },
	{ "gujarati", "gu" },
	{ "haitian", "ht" },
	{ "hausa", "ha" },
	{ "hebrew", "he" },
	{ "herero", "hz" },
	{ "hindi", "hi" },
	{ "hungarian", "hu" },
	{ "icelandic", "is" },
	{ "ido", "io" },
	{ "igbo", "ig" },
	{ "indonesian", "id" },
	{ "interlingua", "ia" },
	{ "interlingue", "ia" },
	{ "inuktitut", "iu" },
	{ "inupiak", "ik" },
	{ "irish", "ga" },
	{ "italian", "it" },
	{ "japanese", "ja" },
	{ "javanese", "jv" },
	{ "kalaallisut", "kl" },
	{ "kannada", "kn" },
	{ "kanuri", "kr" },
	{ "kashmiri", "ks" },
	{ "kazakh", "kk" },
	{ "kikuyu", "ki" },
	{ "kinyarwanda", "rw" },
	{ "kirghiz", "ky" },
	{ "komi", "kv" },
	{ "kongo", "kg" },
	{ "korean", "ko" },
	{ "kuanyama", "kj" },
	{ "kurdish", "ku" },
	{ "kwanyama", "kj" },
	{ "kyrgyz", "ky" },
	{ "lao", "lo" },
	{ "latin", "la" },
	{ "latvian", "lv" },
	{ "letzeburgesch", "lb" },
	{ "limburgan", "li" },
	{ "limburger", "li" },
	{ "limburgish", "li" },
	{ "linagala", "ln" },
	{ "lithuanian", "lt" },
	{ "luxembourgish", "lb" },
	{ "madedonian", "mk" },
	{ "makasar", "mg" },
	{ "malay", "ms" },
	{ "malayalam", "ml" },
	{ "maldivian", "mv" },
	{ "maltese", "mt" },
	{ "manx", "gv" },
	{ "maori", "mi" },
	{ "marathi", "mr" },
	{ "marshallese", "mh" },
	{ "moldavian", "ro" },
	{ "moldovan", "ro" },
	{ "mongolian", "mn" },
	{ "nauru", "na" },
	{ "navaho", "nv" },
	{ "navajo", "nv" },
	{ "ndebele", "nd" },
	{ "ndonga", "ng" },
	{ "nepali", "ne" },
	{ "norwegian", "no" },
	{ "nuosu", "ii" },
	{ "nyanja", "ny" },
	{ "nynorsk", "nn" },
	{ "occidental", "ie" },
	{ "occitan", "oc" },
	{ "ojibwa", "oj" },
	{ "oriya", "or" },
	{ "oromo", "om" },
	{ "ossetian", "os" },
	{ "ossetic", "os" },
	{ "pali", "pi" },
	{ "panjabi", "pa" },
	{ "pashto", "ps" },
	{ "persian", "fa" },
	{ "polish", "pl" },
	{ "portuguese", "pt" },
	{ "punjabi", "pa" },
	{ "pushto", "ps" },
	{ "quechua", "qu" },
	{ "romanian", "ro" },
	{ "romansh", "rm" },
	{ "rundi", "rn" },
	{ "russian", "ru" },
	{ "samoan", "sm" },
	{ "sango", "sg" },
	{ "sanskrit", "sa" },
	{ "sardinian", "sc" },
	{ "serbian", "sr" },
	{ "shona", "sn" },
	{ "sindhi", "sd" },
	{ "sinhala", "si" },
	{ "sinhlese", "si" },
	{ "slovak", "sk" },
	{ "slovenian", "sl" },
	{ "somali", "so" },
	{ "sotho", "st" },
	{ "spanish", "es" },
	{ "sudanese", "su" },
	{ "swahili", "sw" },
	{ "swati", "ss" },
	{ "swedish", "sv" },
	{ "swiss", "de" }, /**/
	{ "tagalog", "tl" },
	{ "tahitian", "ty" },
	{ "tajik", "tg" },
	{ "tamil", "ta" },
	{ "tatar", "tt" },
	{ "telugu", "te" },
	{ "thai", "th" },
	{ "tibetan", "bo" },
	{ "tigrinya", "ti" },
	{ "tonga", "to" },
	{ "tsonga", "ts" },
	{ "tswana", "tn" },
	{ "turkish", "tr" },
	{ "turkmen", "tk" },
	{ "twi", "tw" },
	{ "uighur", "ug" },
	{ "ukrainian", "uk" },
	{ "urdu", "ur" },
	{ "uyghur", "ug" },
	{ "uzbek", "uz" },
	{ "valencian", "ca" },
	{ "venda", "ve" },
	{ "vietnamese", "vi" },
	{ "volapuk", "vo" },
	{ "walloon", "wa" },
	{ "welsh", "cy" },
	{ "wolof", "wo" },
	{ "xhosa", "xh" },
	{ "yiddish", "yi" },
	{ "yoruba", "yo" },
	{ "zhuang", "za" },
	{ "zulu", "zu" },
};

/* Let space for iso 639 3 digit codes. */
static char s_lang[4] = "en";

static int langstr_cmp(const void *k, const void *elem)
{
	const char *pk;
	const struct langstr_info *pinfo;

	pk = (const char *) k;
	pinfo = (const struct langstr_info *) elem;
	return strcmp(pk, pinfo->langstr);
}

const char *guess_lang(void)
{
	char *loc, *p;
	struct langstr_info *pinfo;

	loc = dupstr(setlocale(LC_CTYPE, ""));
	setlocale(LC_CTYPE, "C");
	if (loc == NULL) {
		ktrace("setlocale() NULL");
		return s_lang;
	}

	ktrace("setlocale() is %s", loc);
	for (p = loc; *p != '\0'; p++) { 
		*p = tolower(*p);
	}

	/* Find first non letter. */
	for (p = loc; *p != '\0'; p++) { 
		if (!isalpha(*p)) {
			*p = '\0';
			break;
		}
	}

	/* try yo find it in our list */
	ktrace("try to find lang %s", loc);
	pinfo = bsearch(loc, s_langstr_map, NELEMS(s_langstr_map),
			sizeof(*pinfo), langstr_cmp);

	if (pinfo != NULL) {
		strcpy(s_lang, pinfo->isocode);
	} else if (strlen(loc) == 2) {
		strcpy(s_lang, loc);
	}

	free(loc);

	ktrace("returning lang %s", s_lang);
	return s_lang;
}

static struct lang_equiv {
	const char *src;
	const char *dst;
} s_lang_equivs[LANG_NEQUIVS];

static int s_lang_equiv_index = 0;

void add_lang_equivalence(const char *src, const char *dst)
{
	if (kassert_fails(s_lang_equiv_index < LANG_NEQUIVS))
		return;
	if (kassert_fails(src != NULL && dst != NULL))
		return;

	s_lang_equivs[s_lang_equiv_index].src = src;
	s_lang_equivs[s_lang_equiv_index].dst = dst;
	s_lang_equiv_index++;
}

const char *get_lang_equivalence(const char *src)
{
	int i;

	for (i = 0; i < s_lang_equiv_index; i++) {
		if (strcmp(s_lang_equivs[i].src, src) == 0) {
			return s_lang_equivs[i].dst;
		}
	}

	return src;
}

void reset_lang_equivalences(void)
{
	s_lang_equiv_index = 0;
}

