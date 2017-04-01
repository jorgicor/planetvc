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

#include "vfs.h"
#include "cbase/kassert.h"
#include "cfg/cfg.h"
#include <ctype.h>
#include <string.h>
#include <limits.h>

#if PP_ANDROID
#	include <errno.h>
#	include <jni.h>
#	include <android/asset_manager.h>
#	include <android/asset_manager_jni.h>
#	include <android/log.h>
#endif

/**
 * At the end of a .zip file we have the END OF CENTRAL DIRECTORY RECORD, with
 * this layout:
 *
 *    offset  length
 *	 0	4	End of central directory signature = 0x06054b50
 *	 4	2	Number of this disk
 *	 6	2	Disk where central directory starts
 *	 8	2	Number of central directory record on this disk
 *	10	2	Total number of central directory records
 *	12	4	Size of central directory (bytes)
 *	16	4	Offset to start of central directory, relative to
 *			start of archive
 *	20	2	Comment length (n)
 *	22	n	Comment
 *
 * At offset 16, we have the offset to the central directory record, which is
 * a series of CENTRAL DIRECTORY HEADER (struct zip_cdh).
 *
 * Each CENTRAL DIRECTORY HEADER has an offset to a LOCAL FILE HEADER
 * (struct zip_lfh), and next to each CENTRAL DIRECTORY HEADER is the actual
 * file data.
 */

enum {
	/* Length of END OF CENTRAL DIRECTORY record without the comment */
	EOF_CENTRAL_DIR_LEN = 22,
};

#pragma pack(2)
/* .zip Local file header */
struct zip_lfh {
	unsigned int signature;
	unsigned short version;
	unsigned short bitflag;
	unsigned short compression;
	unsigned short modif_time;
	unsigned short modif_date;
	unsigned int crc32;
	unsigned int compressed_size;
	unsigned int original_size;
	unsigned short fname_len;
	unsigned short extra_len;
};
#pragma pack()

#pragma pack(2)
/* .zip Central directory header */
struct zip_cdh {
	unsigned int signature;
	unsigned short made_by;
	unsigned short version;
	unsigned short bitflag;
	unsigned short compression;
	unsigned short mod_time;
	unsigned short mod_date;
	unsigned int crc32;
	unsigned int compressed_sz;
	unsigned int original_sz;
	unsigned short fname_len;
	unsigned short extra_len;
	unsigned short comment_len;
	unsigned short start_disk;
	unsigned short attr_intern;
	unsigned int attr_extern;
	unsigned int file_hdr;
};
#pragma pack()

static const char *s_base_path;

#if !PP_ANDROID

#define platform_fopen fopen

#endif	/* if !PP_ANDROID */

#if PP_ANDROID
static AAssetManager *s_android_asset_manager;

JNIEXPORT void JNICALL
Java_org_libsdl_app_SDLActivity_loadAssetManager(JNIEnv *env,
						 jobject obj,
						 jobject assetManager)
{
	s_android_asset_manager = AAssetManager_fromJava(env, assetManager);
	if (s_android_asset_manager == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "name.cpp",
			"error loading asset manager");
	} else {
		__android_log_print(ANDROID_LOG_VERBOSE, "name.cpp",
			"loaded asset manager");
	}
}

static int android_file_read(void *cookie, char *buf, int size)
{
	return AAsset_read((AAsset *) cookie, buf, size);
}

static int android_file_write(void *cookie, const char *buf, int size)
{
	return EACCES;
}

static fpos_t android_file_seek(void *cookie, fpos_t offset, int from)
{
	return AAsset_seek((AAsset *) cookie, offset, from);
}

static int android_file_close(void *cookie)
{
	AAsset_close((AAsset *) cookie);
	return 0;
}

static FILE *android_file_open(const char *fname, const char *mode)
{
	AAsset *asset = AAssetManager_open(s_android_asset_manager, fname, 0);
	if (asset) {
		return funopen(asset, android_file_read, android_file_write,
			       android_file_seek, android_file_close);	
	}
	
	return NULL;
}

#define platform_fopen android_file_open

#endif	/* if PP_ANDROID */

/**
 * Seeks the central directory in a .zip file.
 *
 * If found, returns 0 and the file is positioned at the start of it.
 * If not found returns different than 0.
 *
 * * TODO: rewrite this without using a 64K window?
 */
static int seek_central_dir(FILE *fp)
{
	long read, offs;
	int i;
	int commentsz;
	unsigned int signature;
	unsigned char win[65535 + EOF_CENTRAL_DIR_LEN];

	/* The 'end of central directory' record must be somewhere in the
	 * last 22 + 65535 bytes of the file */

	fseek(fp, 0, SEEK_END);
	read = ftell(fp);
	if (read < EOF_CENTRAL_DIR_LEN)
		return -1;

	if (read > sizeof(win))
		read = sizeof(win);
	
	fseek(fp, -read, SEEK_CUR);
	fread(win, read, 1, fp);
	
	/* Find 'end of central directory' signature. */
	for (i = read - EOF_CENTRAL_DIR_LEN; i >= 0; i--) {
		/* Check signature. */
		/*
		if (win[i] != 0x50 || win[i + 1] != 0x4b ||
		    win[i + 2] != 0x05 || win[i + 3] != 0x06)
			continue;
		*/

		if (!((win[i] == 0x50 && win[i + 1] == 0x4b &&
		       win[i + 2] == 0x05 && win[i + 3] == 0x06) ||
		      (win[i] == 0x05 && win[i + 1] == 0xb4 &&
		       win[i + 2] == 0x50 && win[i + 3] == 0x60)))
			continue;


		/* Check comment size. */
		commentsz = win[i + 20];
		commentsz |= ((int) win[i + 21]) << 8;
		if (i + EOF_CENTRAL_DIR_LEN + commentsz != read)
			continue;

		/* TODO: careful with this, signed - unsigned etc */
		/* Get offset to central directory. */
		offs = win[i + 16];
		offs |= ((int) win[i + 17]) << 8;
		offs |= ((int) win[i + 18]) << 16;
		offs |= ((int) win[i + 19]) << 24;

		/* Check signature of central directory. */
		fseek(fp, offs, SEEK_SET);
		fread(&signature, sizeof(signature), 1, fp);
		if (signature == 0x02014b50 || signature == 0x2010b405) {
			fseek(fp, -((long) sizeof(signature)), SEEK_CUR);
			return 0;
		}
	}

	return -1;
}

/**
 * A valid path is a sequence of words [ '/' words ]*.
 *
 * Words are a sequence of alphanumerics or '_' or '.'. No spaces.
 */
static int valid_path(const char *fname)
{
	int separator, ext;

	if (fname == NULL)
		return 0;

	separator = 0;
	ext = 0;
	for ( ; *fname != '\0'; fname++) {
		if (*fname == '/') {
			if (!separator)
				return 0;
			separator = 0;
			ext = 0;
		} else if (*fname == '.') {
			if (!separator)
				return 0;
			else if (ext)
				return 0;
			else
				ext = 1;
		} else if (!isalnum(*fname) && *fname != '_') {
			return 0;
		} else {
			separator = 1;
		}
	}

	return separator;
}

/**
 * 'fp' points on enter to a local file header inside a .zip file.
 * 
 * On return, if there is no error, 'fp' will point to the start of the file
 * data corresponding to this header and the function returns 0.
 *
 * In the case of an error, the funcion returns different than 0.
 *
 * fsize can be passed NULL. If not NULL and the header is valid,
 * it will contain the original size of the file we are searching.
 */
static int seek_file_data(FILE *fp, unsigned int *fsize)
{
	struct zip_lfh lfh;

	if (fread(&lfh, sizeof(lfh), 1, fp) == 0)
		return -1;

	if (lfh.signature != 0x04034b50 && lfh.signature != 0x4030b405)
		return -1;

	if (fsize != NULL)
		*fsize = lfh.original_size;

	fseek(fp, lfh.fname_len + lfh.extra_len, SEEK_CUR);
	return 0;
}

/**
 * Seeks the local header inside a .zip file corresponding to the
 * entry 'fname'. 'fp' must be pointing to the start of the central
 * directory of the .zip file.
 *
 * If found, returns 0 and 'fp' points to the start of the local file header.
 * If not found, returns different than 0.
 *
 * fsize can be passed NULL. If not NULL and the header is found,
 * it will contain the original size of the file we are searching.
 */
static int seek_local_hdr(FILE *fp, const char *fname,
			  unsigned int *fsize)
{
	struct zip_cdh cdh;
	char pfname[OPEN_FILE_MAX_PATH_LEN + 1];

	for (;;) {
		if (fread(&cdh, sizeof(cdh), 1, fp) == 0)
			return -1;

		if (cdh.signature != 0x02014b50 &&
			cdh.signature != 0x2010b405)
			return -1;

		if (cdh.fname_len > sizeof(pfname) - 1) {
			fseek(fp, cdh.fname_len, SEEK_CUR);
			goto next;
		}

		if (fread(pfname, cdh.fname_len, 1, fp) == 0)
			return -1;

		pfname[cdh.fname_len] = '\0';	
		if (strcmp(pfname, fname) != 0)
			goto next;

		if (cdh.compression != 0)
			goto next;

		if (fsize != NULL)
			*fsize = cdh.original_sz;

		/* Position at start of local file header. */
		fseek(fp, cdh.file_hdr, SEEK_SET);
		return 0;

next:		fseek(fp, cdh.extra_len + cdh.comment_len, SEEK_CUR);
	}

	return -1;
}

/**
 * Try to open 'fname' inside a 'pak'.pak file which must be a standard
 * .zip file.
 *
 * 'fname' must not be compressed.
 *
 * fsize can be passed NULL. If not NULL and the file isfound,
 * it will contain the original size of the file.
 *
 * TODO: check if fsz1 can be different from fsz2. Which is more reliable?
 */
static FILE *try_open_pak(const char *pak, const char *fname,
			  unsigned int *fsize)
{
	char path[OPEN_FILE_MAX_PATH_LEN + 1];
	FILE *fp;
	unsigned int fsz1, fsz2;

	fsz1 = fsz2 = 0;
	if (strlen(s_base_path) + strlen(pak) + 4 > sizeof(path) - 1)
		return NULL;

	strcpy(path, s_base_path);
	strcat(path, pak);
	strcat(path, ".pak");

	/* ktrace("try open pak %s", path); */
	fp = platform_fopen(path, "rb");
	if (fp == NULL)
		return NULL;
	/* ktrace("opened"); */

	if (seek_central_dir(fp) != 0)
		goto fail;

	if (seek_local_hdr(fp, fname, &fsz1) != 0)
		goto fail;

	if (seek_file_data(fp, &fsz2) != 0)
		goto fail;

	if (fsize != NULL)
		*fsize = fsz1;

	return fp;

fail:	fclose(fp);
	return NULL;
}

/**
 * open_file()
 *
 * To ilustrate the function behavior, imagine that we call
 * open_file("dir1/dir2/file.bmp").
 *
 * If "dir1.pak" exists and is a valid '.zip' file and an uncompressed
 * entry "dir2/file.bmp" is inside it, we return a 'FILE *', which is
 * "dir1.pak", opened in read mode and binary, and the position inside this
 * file is the start of the "dir2/file.bmp" data.
 *
 * If not, we try with "dir1/dir2.pak" searching for "file.bmp" inside it.
 *
 * If not successful, the function behaves as a call to
 * fopen("dir1/dir2/file.bmp", "rb").
 *
 * 'fname' length must be less than OPEN_FILE_MAX_PATH_LEN.
 *
 * The format of 'fname' should be this:
 *
 *	valid_file_name [ /valid_file_name ]* 
 *
 * where 'valid_file_name' is:
 *
 *	alphanum [ . alphanum ]
 *
 * and 'alphanum' is a sequence of letters A-Z, a-z, 0-9 or '_'.
 *
 * The path set in vfs_set_base_path() is prefixed to fname, but it is
 * not used to search for .zip files: it must be a real path.
 * By default, this path is empty.
 *
 * Note that the file returned is open in read mode and binary.
 * Also, note that this file can be a stream inside a .zip file,
 * so don't assume you will get EOF as normally, you must use a
 * code to know when your file actually ends.
 *
 * If the file is opened correctly and fsize is not NULL, fsize will
 * contain the size of the file ONLY if the file was contained in a
 * ZIP file. For efficiency, if it was not contained in a ZIP file,
 * fsize will be UINT_MAX. You will have to use fseek, ftell, rewind to get
 * the actual size if you need it.
 *
 */
FILE* open_file(const char *fname, unsigned int *fsize)
{
	char path[OPEN_FILE_MAX_PATH_LEN + 1];
	FILE *fp;
	char *p;

	if (!valid_path(fname))
		return NULL;

	if (strlen(fname) + strlen(s_base_path) > sizeof(path) - 1)
		return NULL;

	strcpy(path, fname);
	for (p = path; *p != '\0'; p++) {
		if (*p == '/') {
			*p = '\0';
			fp = try_open_pak(path, p + 1, fsize);
			if (fp != NULL) {
				if (fsize != NULL && *fsize == UINT_MAX) {
					/* UINT_MAX is not allowed as
					 * size for us. */
					fclose(fp);
					return NULL;
				}
				return fp;
			}
			*p = '/';
		}
	}

	if (fsize != NULL)
		*fsize = UINT_MAX;
	strcpy(path, s_base_path);
	strcat(path, fname);
	/* ktrace("open file %s", path); */
	fp = platform_fopen(path, "rb");
	/*
	if (fp != NULL)
		ktrace("opened");
	*/
	return fp;
}

/*
 * Sets the base path for open_file().
 * Only a pointer to this path is stored.
 */
void vfs_set_base_path(const char *path)
{
	s_base_path = path;
}

void vfs_init(void)
{
	vfs_set_base_path("");

#if PP_ANDROID
	// s_android_asset_manager = NULL;
#endif
}
