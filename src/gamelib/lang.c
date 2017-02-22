#include "lang.h"
#include "cbase/cbase.h"
#include "cbase/kassert.h"
#include "cfg/cfg.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#if PP_ANDROID
#include <jni.h>
#include <SDL.h>
#endif

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

/* this table must be sorted to do binary search. */
static const struct langstr_info s_regionstr_map[] = {
	{ "brazil", "br" },
};

struct langregion {
	const char *lang;
	const char *region;	
};

/* If a language and region matches any of these elements, they will be
 * joined to form a locale with the two, for example: "pt" and "br" -> "ptbr".
 */
static const struct langregion s_langregions[] = {
	{ "pt", "br" },
};

/* Let space for 2 chars language and 2 chars region. */
static char s_lang[5] = "en";

static int langstr_cmp(const void *k, const void *elem)
{
	const char *pk;
	const struct langstr_info *pinfo;

	pk = (const char *) k;
	pinfo = (const struct langstr_info *) elem;
	return strcmp(pk, pinfo->langstr);
}

#if PP_ANDROID
static char *Android_GetLocale(void)
{
	static char *locstr = NULL;

	JNIEnv *env;
	jobject theActivity;
	jclass theClass;
	jmethodID theMethod;
	jstring retval;
	const char *str;

	if (locstr != NULL) {
		free(locstr);
		locstr = NULL;
	}

	env = (JNIEnv *) SDL_AndroidGetJNIEnv();
	theActivity = (jobject) SDL_AndroidGetActivity();

	theClass = (*env)->GetObjectClass(env, theActivity);
	theMethod = (*env)->GetStaticMethodID(env, theClass, "getLocale",
			"()Ljava/lang/String;");
	retval = (*env)->CallStaticObjectMethod(env, theClass, theMethod);
	str = (*env)->GetStringUTFChars(env, retval, NULL);
	if (str != NULL) {
		locstr = dupstr(str);
		(*env)->ReleaseStringUTFChars(env, retval, str);
	}

	(*env)->DeleteLocalRef(env, theActivity);
	(*env)->DeleteLocalRef(env, theClass);

	return locstr;
}
#endif

/* Tries to match region and if needed appends it to the current s_lang. */
static void append_region(const char *region)
{
	int i;
	struct langstr_info *pinfo;
	const char *isocode;

	pinfo = bsearch(region, s_regionstr_map, NELEMS(s_regionstr_map),
		        sizeof(*pinfo), langstr_cmp);

	if (pinfo == NULL) {
		if (strlen(region) == 2) {
			isocode = region;
		} else {
			return;
		}
	} else {
		isocode = pinfo->isocode;
	}

	/* Try to find s_lang + region code in s_langregions.
	 * If found, form s_lang with lang + region.
	 */
	for (i = 0; i < NELEMS(s_langregions); i++) { 
		if (strcmp(s_langregions[i].lang, s_lang) == 0 &&
		    strcmp(s_langregions[i].region, isocode) == 0)
		{
			if (kassert_fails(strlen(s_lang) + strlen(isocode) <
					  NELEMS(s_lang)))
			{
				return;
			}
			strcat(s_lang, isocode);
			break;
		}
	}
}

const char *guess_lang(void)
{
	char *loc, *p, *region;
	struct langstr_info *pinfo;

#if PP_ANDROID
	loc = dupstr(Android_GetLocale());
#else
	loc = dupstr(setlocale(LC_CTYPE, ""));
	setlocale(LC_CTYPE, "C");
#endif

	if (loc == NULL) {
		ktrace("setlocale() NULL");
		return s_lang;
	}

#if 0
	free(loc);
	loc = dupstr("English_United States.1234");
#endif
#if 0
	free(loc);
	loc = dupstr("Portuguese_Brazil.1234");
#endif
#if 0
	free(loc);
	loc = dupstr("pt_BR");
#endif

	ktrace("setlocale() is %s", loc);
	for (p = loc; *p != '\0'; p++) { 
		*p = tolower(*p);
	}

	/* Find region first, if any */
	region = p;	/* Point to '\0' by default */
	for (p = loc; *p != '\0'; p++) { 
		if (*p == '_') {
			region = p + 1;
			break;
		}
	}

	/* Put \0 at the end of the region */
	for (p = region; *p != '\0'; p++) {
		if (*p != ' ' && !isalpha(*p)) {
			*p = '\0';
			break;
		}
	}

	/* Now put '\0' at the end of the language name.
	 * This discards any -xxx part.
	 */
	for (p = loc; *p != '\0'; p++) { 
		if (!isalpha(*p)) {
			*p = '\0';
			break;
		}
	}

	/* try yo find it in our list */
	ktrace("try to find lang %s, region %s", loc, region);
	pinfo = bsearch(loc, s_langstr_map, NELEMS(s_langstr_map),
			sizeof(*pinfo), langstr_cmp);


	if (pinfo != NULL) {
		strcpy(s_lang, pinfo->isocode);
		append_region(region);
	} else if (strlen(loc) == 2) {
		strcpy(s_lang, loc);
		append_region(region);
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

