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

#include "kernel_snd_sdl.h"
#include "kernel.h"
#include "cbase/cbase.h"
#include "cbase/kassert.h"
#include <SDL.h>
#include <stdlib.h>

static int s_snd_device;
static int s_snd_playing;
static kernel_snd_callback_t s_on_sound;
static int s_snd_channels;
static int s_snd_bits;
static int s_snd_i;
static int s_snd_len;
static int s_snd_write_i;
static int s_snd_sample_bytes;
static int s_snd_buf_len;
static int s_snd_dev_buf_len;
static unsigned char *s_snd_buf;

static void null_callback(unsigned char *ptr, int nsamples)
{
}

static int is_valid_device(int snd_device_id)
{
	return snd_device_id != 0;
}

#if 0
static void list_devices(void)
{
	int i, n;
	const struct kernel_device *kdev;

	kdev = kernel_get_device();
	n = SDL_GetNumAudioDevices(0);
	for (i = 0; i < n; i++) {
		kdev->trace("%d %s", i, SDL_GetAudioDeviceName(i, 0));
		kdev->trace("%d %s", i, SDL_GetAudioDriver(i));
	}
}
#endif

static int init(int fps, int sample_rate, int nchannels, int bit_depth)
{
	SDL_AudioSpec want, got;

	if (is_valid_device(s_snd_device))
		return KERNEL_E_ERROR;

#if 0
	list_devices();
#endif

	memset(&want, 0, sizeof(want));
	want.freq = sample_rate;
	want.channels = nchannels;
	s_snd_channels = want.channels;
	if (bit_depth == 8) {
		s_snd_bits = 8;
		want.format = AUDIO_U8;
	} else {
		s_snd_bits = 16;
		want.format = AUDIO_S16;
	}
	s_on_sound = null_callback;

	s_snd_sample_bytes = (s_snd_bits / 8) * s_snd_channels;

	/* keep enough for 2 frames at fps.
	 * SDL says this must be power of 2.
	 */
	want.samples = ipow2ge((want.freq / fps) * 2);
	s_snd_dev_buf_len = want.samples * s_snd_sample_bytes; 
	
	/* TODO: Is this necessary? Will not suffice s_snd_dev_buf_len? */
	s_snd_buf_len = s_snd_dev_buf_len * 2;

	s_snd_buf = malloc(s_snd_buf_len);
	if (s_snd_buf == NULL)
		return KERNEL_E_ERROR;

	s_snd_device = SDL_OpenAudioDevice(NULL, 0, &want, &got, 0);
	if (!is_valid_device(s_snd_device)) {
		free(s_snd_buf);
		s_snd_buf = NULL;
		return KERNEL_E_ERROR;
	}

	s_snd_i = 0;
	s_snd_len = 0;
	s_snd_write_i = 0;
	s_snd_playing = 0;
	return KERNEL_E_OK;
}

static void play(void)
{
	if (!is_valid_device(s_snd_device))
		return;
	if (!s_snd_playing) {
		s_snd_playing = 1;
		SDL_PauseAudioDevice(s_snd_device, 0);
	}
}

static void stop(void)
{
	if (!is_valid_device(s_snd_device))
		return;
	if (s_snd_playing) {
		s_snd_playing = 0;
		SDL_PauseAudioDevice(s_snd_device, 1);
	}
}

static void update(void)
{
	int n, m, remain, sent;

	if (!is_valid_device(s_snd_device))
		return;

	/* This can't happen. */
	if (kassert_fails(s_snd_buf != NULL))
		return;

	/* This is guaranteed by init() */
	kasserta((s_snd_buf_len & s_snd_sample_bytes) == 0);

	/* poll how many has been sent */
	remain = SDL_GetQueuedAudioSize(s_snd_device);
	kasserta((remain % s_snd_sample_bytes) == 0);

	sent = s_snd_len - remain;
	s_snd_i = (s_snd_i + sent) % s_snd_buf_len;
	s_snd_len -= sent;

	/* ask app to fill empty space */
	n = s_snd_dev_buf_len - s_snd_len;

	/* This is guaranteed. */
	kasserta((n % s_snd_sample_bytes) == 0);

	kasserta(s_snd_write_i < s_snd_buf_len);
	if (n > 0) {
		if (s_snd_write_i + n <= s_snd_buf_len) {
			s_on_sound(s_snd_buf + s_snd_write_i,
				   n / s_snd_sample_bytes);
			s_snd_write_i = (s_snd_write_i + n) % s_snd_buf_len;
		} else {
			m = s_snd_buf_len - s_snd_write_i;
			kasserta((m % s_snd_sample_bytes) == 0);
			kasserta(s_snd_write_i + m <= s_snd_buf_len);
			s_on_sound(s_snd_buf + s_snd_write_i,
				   m / s_snd_sample_bytes); 
			n -= m;
			kasserta(n <= s_snd_buf_len);
			kasserta((n % s_snd_sample_bytes) == 0);
			s_on_sound(s_snd_buf, n / s_snd_sample_bytes);
			s_snd_write_i = n;
		}
	}

	/* send to device */

	n = s_snd_write_i;
	if (s_snd_write_i < s_snd_i)
		n += s_snd_buf_len;

	/* how much to write */
	n = n - (s_snd_i + s_snd_len);

	/* first position to write */
	m = (s_snd_i + s_snd_len) % s_snd_buf_len;
	if (n > 0) {
		if (m < s_snd_write_i) {
			kasserta(m + n <= s_snd_buf_len);
			SDL_QueueAudio(s_snd_device, s_snd_buf + m, n);
		} else {
			SDL_QueueAudio(s_snd_device, s_snd_buf + m,
				       s_snd_buf_len - m);
			kasserta(s_snd_write_i <= s_snd_buf_len);
			SDL_QueueAudio(s_snd_device, s_snd_buf, s_snd_write_i); 
		}
		s_snd_len += n;
	}
}

static void release(void)
{
	if (!is_valid_device(s_snd_device))
		return;

	stop();
	SDL_CloseAudioDevice(s_snd_device);
	s_snd_device = 0;
	free(s_snd_buf);
	s_snd_buf = NULL;
}

static void set_callback(kernel_snd_callback_t fn)
{
	if (!is_valid_device(s_snd_device))
		return;

	if (fn == NULL) {
		s_on_sound = null_callback;
	} else {
		s_on_sound = fn;
	}
}

struct kernel_snd_iface s_kernel_snd_sdl = {
	.init = init,
	.release = release,
	.set_callback = set_callback,
	.play = play,
	.stop = stop,
	.update = update
};
