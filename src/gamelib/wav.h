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

#ifndef WAV_H
#define WAV_H

#ifndef STDIO_H
#define STDIO_H
#include <stdio.h>
#endif

enum {
	WAV_FMT_1M08 = 11025 << 2,
	WAV_FMT_1M16,
	WAV_FMT_1S08,
	WAV_FMT_1S16,
	WAV_FMT_2M08 = 22050 << 2,
	WAV_FMT_2M16,
	WAV_FMT_2S08,
	WAV_FMT_2S16,
	WAV_FMT_4M08 = 44100 << 2,
	WAV_FMT_4M16,
	WAV_FMT_4S08,
	WAV_FMT_4S16,
};

#define WAV_FMT_IS_STEREO(format) ((format & 2) >> 1)
#define WAV_FMT_IS_16BITS(format) (format & 1)
#define WAV_FMT_SAMPLE_RATE(format) (format >> 2)

struct wav {
	int format;
	unsigned int nsamples;
	unsigned char *data;
};

#ifdef __cplusplus
extern "C" {
#endif

void free_wav(struct wav *wav);
struct wav *load_wav_fp(FILE *fp);

#ifdef __cplusplus
}
#endif

#endif
