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

#ifndef MIXER_H
#define MIXER_H

enum {
	/* Number fo channels the mixer can hold */
	MIXER_NCHANNELS = 8,

	/* Number of sounds that can be queued on a single channel */
	MIXER_NQUEUE = 2,
};

struct wav;

int mixer_get_free_channel(void);
int mixer_get_free_channel_r(int a, int b);
int mixer_queue(int channel, struct wav *pwav, int loop);
int mixer_play(struct wav *pwav);
int mixer_stop_n_play(int chanid, struct wav *pwav, int loop);
int mixer_is_playing(int channel);
void mixer_stop(int channel);
void mixer_stop_all(void);
void mixer_generate(short *ptr, int nsamples, int fill_silence);
void mixer_set_volume(int vol);
int mixer_get_volume(void);

void mixer_init(void);

#endif
