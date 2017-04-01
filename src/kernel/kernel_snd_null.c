/*
Copyright (c) 2016-2016 Jorge Giner Cordero

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

#include "kernel_snd_null.h"
#include "kernel.h"
#include "cbase/kassert.h"
#include <SDL.h>
#include <stdlib.h>

static int s_init;
static int s_sample_rate;
static int s_nchannels;
static int s_bit_depth;
static int s_playing;
static kernel_snd_callback_t s_callback;
static Uint64 s_clock_freq, s_ticks;
static int s_samples_in;	/* samples currently in device */
static int s_samples_keep;	/* samples that must be kept in device */
static int s_sample_bytes;
static int s_buf_len;
static unsigned char *s_buf;

static int init(int fps, int sample_rate, int nchannels, int bit_depth)
{
	int fps2;

	if (s_init)
		return KERNEL_E_ERROR;

	/* That this is not zero is asserted by kernel.c */
	s_clock_freq = SDL_GetPerformanceFrequency();

	s_sample_rate = sample_rate;
	s_nchannels = nchannels;
	s_bit_depth = bit_depth;
	s_callback = NULL;
	s_playing = 0;
	s_samples_in = 0;
	s_sample_bytes = (s_bit_depth / 8) * s_nchannels;

	/* enough samples for a game going at fps / 2. */
	fps2 = fps / 2;
	if (fps2 <= 0)
		fps2 = 1;
	s_samples_keep = sample_rate / fps2;
	s_buf_len = s_samples_keep * s_sample_bytes;

	/* kernel_get_device()->trace("samples keep %d", s_samples_keep); */
	
	s_buf = malloc(s_buf_len);
	if (s_buf == NULL)
		return KERNEL_E_ERROR;

	s_init = 1;
	return KERNEL_E_OK;
}

static void play(void)
{
	if (s_init && !s_playing) {
		s_ticks = SDL_GetPerformanceCounter();
		s_playing = 1;
	}
}

static void stop(void)
{
	if (s_init)
	       s_playing = 0;
}

static void update(void)
{
	Uint64 t;
	double d;
	int tosend;

	if (!s_init)
		return;

	t = SDL_GetPerformanceCounter();

	/* elapsed seconds */
	d = (double) (t - s_ticks) / s_clock_freq;

	/* samples played */
	d = s_sample_rate * d;
	
	/* kernel_get_device()->trace("samples played %f", d); */

	s_samples_in -= (int) d;
	if (s_samples_in < 0)
		s_samples_in = 0;

	tosend = s_samples_keep - s_samples_in;
	if (tosend > 0 && s_callback != NULL) {
		s_callback(s_buf, tosend);
	}
	s_samples_in += tosend;
	kasserta(s_samples_in == s_samples_keep);

	/* kernel_get_device()->trace("samples in device %d", s_samples_in); */

	s_ticks = t;
}

static void release(void)
{
	if (s_init) {
		s_init = 0;
		free(s_buf);
		s_buf = NULL;
	}
}

static void set_callback(kernel_snd_callback_t fn)
{
	s_callback = fn;
}

struct kernel_snd_iface s_kernel_snd_null = {
	.init = init,
	.release = release,
	.set_callback = set_callback,
	.play = play,
	.stop = stop,
	.update = update
};
