/*
Copyright (c) 2014, 2015, 2016 Jorge Giner Cordero

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

#include "kernel_snd.h"
#include "kernel.h"
#include "kernel_snd_sdl.h"
#include "kernel_snd_null.h"
#include <stddef.h>

static struct kernel_snd_iface *s_iface = NULL;

int kernel_snd_init(int fps, int sample_rate, int nchannels, int bit_depth)
{
	int error;
	struct kernel_snd_iface *iface;

	if (s_iface != NULL)
		return KERNEL_E_ERROR;

	/* Set defaults. */
	if (sample_rate == 0)
		sample_rate = KERNEL_SND_44K;

	if (nchannels == 0)
		nchannels = 2;

	if (bit_depth == 0)
		bit_depth = 16;

	iface = &s_kernel_snd_sdl;
	error = iface->init(fps, sample_rate, nchannels, bit_depth);
	if (error) {
		/*
		 * Inform the error but set the null driver to consume
		 * sound.
		 */
		s_iface = &s_kernel_snd_null;
		s_iface->init(fps, sample_rate, nchannels, bit_depth);
		return error;
	}

	s_iface = iface;
	return KERNEL_E_OK;
}

void kernel_snd_release(void)
{
	if (s_iface != NULL) {
		s_iface->release();
		s_iface = NULL;
	}
}

void kernel_snd_set_callback(kernel_snd_callback_t fn)
{
	if (s_iface != NULL)
		s_iface->set_callback(fn);
}

void kernel_snd_play(void)
{
	if (s_iface != NULL)
		s_iface->play();
}

void kernel_snd_stop(void)
{
	if (s_iface != NULL)
		s_iface->stop();
}

void kernel_snd_update(void)
{
	if (s_iface != NULL)
		s_iface->update();
}
