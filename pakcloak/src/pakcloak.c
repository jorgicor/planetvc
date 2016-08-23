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

#include <stdlib.h>
#include <stdio.h>

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

enum { NOFFSETS = 16384 };

/* offsets to the end of central signature + central signatures */
static unsigned int s_offsets[NOFFSETS];
static int s_noffsets;

/**
 * Seeks the central directory in a .zip file.
 *
 * If found, returns 0 and the file is positioned at the start of it.
 * If not found returns different than 0.
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
	offs = read;
	if (read < EOF_CENTRAL_DIR_LEN)
		return -1;

	if (read > sizeof(win))
		read = sizeof(win);
	
	fseek(fp, -read, SEEK_CUR);
	offs -= read;
	fread(win, read, 1, fp);
	
	/* Find 'end of central directory' signature. */
	for (i = read - EOF_CENTRAL_DIR_LEN; i >= 0; i--) {
		/* Check signature. */
		if (win[i] != 0x50 || win[i + 1] != 0x4b ||
		    win[i + 2] != 0x05 || win[i + 3] != 0x06)
			continue;

		s_offsets[0] = offs + i;
		s_noffsets++;

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
		if (signature == 0x02014b50) {
			fseek(fp, -((long) sizeof(signature)), SEEK_CUR);
			return 0;
		}
	}

	return -1;
}

static int valid_cdh(struct zip_cdh *cdh)
{
	return cdh->signature == 0x02014b50;
}

static void read_cdh(FILE *fp, struct zip_cdh *cdh)
{
	fread(cdh, sizeof(*cdh), 1, fp);
	fseek(fp, cdh->fname_len, SEEK_CUR);
	fseek(fp, cdh->extra_len + cdh->comment_len, SEEK_CUR);
}

static int read_offsets(const char *fname)
{
	struct zip_cdh cdh;
	FILE *fp;
	int r;
	unsigned int offs;

	s_noffsets = 0;
	r = -1;
	fp = fopen(fname, "rb");
	if (fp == NULL)
		return r;

	if (seek_central_dir(fp) != 0) {
		fprintf(stderr, "cannot find central dir\n");
		goto fail;
	}

	for (; s_noffsets < NOFFSETS; s_noffsets++) {
		offs = ftell(fp);
		read_cdh(fp, &cdh);
		if (!valid_cdh(&cdh))
			break;
		s_offsets[s_noffsets++] = offs;
		if (s_noffsets < NOFFSETS) {
			s_offsets[s_noffsets] = cdh.file_hdr;
		}
	}

	r = 0;
fail:	fclose(fp);
	return r;
}

static int copy_ab(const char *fa, const char *fb)
{
	FILE *fpa, *fpb;
	int c;

	fpa = fopen(fa, "rb");
	if (fpa == NULL)
		return -1;

	fpb = fopen(fb, "wb");
	if (fpb == NULL) {
		fclose(fpa);
		return -1;
	}

	while ((c = getc(fpa)) != EOF) {
		putc(c, fpb);
	}

	fclose(fpa);
	fclose(fpb);
	return 0;
}

static int cloak(const char *fname)
{
	FILE *fpb;
	int i, j;
	int r[4];

	fpb = fopen(fname, "r+b");
	if (fpb == NULL)
		return -1;;

	for (j = 0; j < s_noffsets; j++) {
		fseek(fpb, s_offsets[j], SEEK_SET);
		// printf("offset %d\n", s_offsets[j]);
		for (i = 0; i < 4; i++) {
			r[i] = getc(fpb);
		}
		fseek(fpb, -4, SEEK_CUR);
		for (i = 0; i < 4; i++) {
			putc(((r[i] & 0x0f) << 4) | ((r[i] & 0xf0) >> 4), fpb);
			// printf("got %x put %x ", r[i],
			//	((r[i] & 0x0f) << 4) | ((r[i] & 0xf0) >> 4));
		}
		// printf("\n");
	}

	fclose(fpb);
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "Usage: pakcloack zipfile outfile\n");
		return EXIT_FAILURE;
	}

	if (read_offsets(argv[1]) != 0) {
		fprintf(stderr,
			"packcloak: error reading ZIP directory offsets\n");
		return EXIT_FAILURE;
	}

	fprintf(stderr, "packcloak: %d offsets read\n", s_noffsets);

	if (copy_ab(argv[1], argv[2]) != 0) {
		fprintf(stderr, "packcloak: error creating %s\n", argv[2]);
		return EXIT_FAILURE;
	}

	if (cloak(argv[2]) != 0) {
		fprintf(stderr, "packcloak: error cloaking %s\n", argv[2]);
		return EXIT_FAILURE;
	}

	fprintf(stderr, "packcloack: success\n");
	return EXIT_SUCCESS;
}
