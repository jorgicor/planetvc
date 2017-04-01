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

#include "wav.h"
#include "vfs.h"
#include <string.h>

#ifndef STDLIB_H
#define STDLIB_H
#include <stdlib.h>
#endif

/*
 * A .wav file has this layout:
 *
 *    bytes
 *	4	'RIFF'
 *	4	size of the rest
 *	4	type, we only support 'WAVE'
 *
 *	--- Now we have a series of chunks, we try to find two of them: ---
 *	--- the format chunk and the data chunk. ---
 *
 *      --- FORMAT CHUNK ---
 *	4	'fmt '
 *	4	size of this chunk, it contains a WAVEFORMATEX or a
 *		WAVEFORMATEXTENSIBLE
 *      n	data
 *
 *	--- DATA CHUNK ---
 *	4	'data'
 *	4	size of this chunk
 *	n	data of this chunk
 */

/* 'FFIR' */
#define RIFF_ID	0x46464952
/* 'EVAW' */
#define WAVE_ID	0x45564157
/* ' tmf' */
#define FMT_ID	0x20746d66
/* 'atad' */
#define DATA_ID	0x61746164

/* 
 * To avoid including Windows headers, we redefine here some structs.
 */

typedef unsigned int        DWORD;
typedef unsigned short      WORD;

#define WAVE_FORMAT_EXTENSIBLE	0xFFFE
#define WAVE_FORMAT_PCM		0x0001

typedef struct {
  WORD  wFormatTag;
  WORD  nChannels;
  DWORD nSamplesPerSec;
  DWORD nAvgBytesPerSec;
  WORD  nBlockAlign;
  WORD  wBitsPerSample;
  WORD  cbSize;
} WAVEFORMATEX;

typedef struct _GUID {
    unsigned int   Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[ 8 ];
} GUID;

static const GUID KSDATAFORMAT_SUBTYPE_PCM = {
	0x00110000,
	0x0000,
	0x1000,
	{ '\x80', '\x00', '\x00', '\xAA', '\x00', '\x38', '\x9B', '\x71' }
};

typedef struct {
  WAVEFORMATEX Format;
  union {
    WORD wValidBitsPerSample;
    WORD wSamplesPerBlock;
    WORD wReserved;
  } Samples;
  DWORD dwChannelMask;
  GUID  SubFormat;
} WAVEFORMATEXTENSIBLE;

struct chunk_hdr {
	unsigned int id;
	unsigned int size;
};

/**
 * Pads value to word boundary.
 *
 * Can't handle n being UINT_MAX, will return 0 in that case.
 */
static unsigned int word_pad(unsigned int n)
{
	return n + (n & 1);
}

/**
 * Finds chunk id, positions fp at the start of its data and
 * in size the size of the data BUT, in the file, this 'data' is 
 * always padded to 16 bits, so you may have to pad (add 1) to the size
 * if you are going to skip the data to find the next chunk.
 * start_fpos is the start position in fp, where the file began, in case
 * the wav file was embedded inside another file.
 * Returns -1 on error, 0 on success.
 */
static int find_chunk(FILE *fp, long start_fpos, unsigned int id,
		      unsigned int *size)
{
	struct chunk_hdr header;

	fseek(fp, start_fpos, SEEK_SET);
	for (;;) {
		if (fread(&header, sizeof(header), 1, fp) == 0)
			return -1;

		if (header.id == RIFF_ID)
			header.size = 4;

		if (header.id == id) {
			*size = header.size;
			return 0;
		}

		fseek(fp, word_pad(header.size), SEEK_CUR);
	}

	return -1; 
}

/**
 * Loads a .wav file.
 *
 * Only supports loading:
 *	- bitrates: 11025, 22050, 44100 samples per second.
 *	- sample bit depth: 8 or 16 bits.
 *	- channels: 1 or 2 (mono or stereo).
 */
struct wav *load_wav_fp(FILE *fp)
{
	WAVEFORMATEXTENSIBLE fmt;
	struct wav *snd;
	unsigned int ftype;
	unsigned int size;
	unsigned char *data;
	int snd_fmt;
	long start_fpos;

	start_fpos = ftell(fp);

	snd = NULL;
	memset(&fmt, 0, sizeof(fmt));
	if (find_chunk(fp, start_fpos, RIFF_ID, &size) != 0)
		return NULL;

	if (fread(&ftype, sizeof(ftype), 1, fp) == 0)
		return NULL;

	if (ftype != WAVE_ID)
		return NULL;

	if (find_chunk(fp, start_fpos, FMT_ID, &size) != 0)
		return NULL;

	if (fread(&fmt, word_pad(size), 1, fp) == 0)
		return NULL;

	if (fmt.Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
		if (memcmp(&fmt.SubFormat, &KSDATAFORMAT_SUBTYPE_PCM,
		    sizeof(GUID)) != 0)
			return NULL;
	} else if (fmt.Format.wFormatTag != WAVE_FORMAT_PCM)
		return NULL;

	snd_fmt = 0;
	if (fmt.Format.wBitsPerSample == 16)
		snd_fmt = 1;
	else if (fmt.Format.wBitsPerSample != 8)
		return NULL;

	if (fmt.Format.nChannels == 2)
		snd_fmt |= 2;
	else if (fmt.Format.nChannels != 1)
		return NULL;

	if (fmt.Format.nSamplesPerSec != 11025 &&
	    fmt.Format.nSamplesPerSec != 22050 &&
	    fmt.Format.nSamplesPerSec != 44100)
		return NULL;

	snd_fmt |= fmt.Format.nSamplesPerSec << 2;

	if (find_chunk(fp, start_fpos, DATA_ID, &size) != 0)
		return NULL;
	
	if ((snd = malloc(sizeof(*snd))) == NULL)
		return NULL;

	/* Pad the size just in case we need this in the future.
	 * BUT don't use it to calculate the snd->nsamples! */
	data = malloc(word_pad(size));
	if (data == NULL)
		goto fresnd;

	if (fread(data, word_pad(size), 1, fp) == 0)
		goto fredat;

	snd->format = snd_fmt;
	snd->data = data;
	size >>= WAV_FMT_IS_STEREO(snd_fmt) + WAV_FMT_IS_16BITS(snd_fmt);
	/* Here we are not padding 'size' */
	snd->nsamples = size;
	return snd;

fredat: free(data);
fresnd:	free(snd);
	return NULL;
}

/**
 * Loads a .wav file.
 *
 * Only supports loading:
 *	- bitrates: 11025, 22050, 44100 samples per second.
 *	- sample bit depth: 8 or 16 bits.
 *	- channels: 1 or 2 (mono or stereo).
 *
 * Opens fname using 'open_file()'. The length of 'fname' must be less
 * than OPEN_FILE_MAX_PATH_LEN.
 */
struct wav *load_wav(const char *fname)
{
	FILE *fp;
	struct wav *snd;

	fp = open_file(fname, NULL);
	if (fp == NULL) {
		return NULL;
	}

	snd = load_wav_fp(fp);

	fclose(fp);
	return snd;
}

void free_wav(struct wav *snd)
{
	if (snd) {
		if (snd->data) {
			free(snd->data);
			snd->data = NULL;
		}
		free(snd);
	}
}
