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

#ifndef KERNEL_SND_H
#define KERNEL_SND_H

typedef void (*kernel_snd_callback_t)(unsigned char *ptr, int nsamples);

struct kernel_snd_iface
{
	int (*init)(int fps, int sample_rate, int nchannels, int bit_depth);
	void (*release)(void);
	void (*set_callback)(kernel_snd_callback_t fn);
	void (*play)(void);
	void (*stop)(void);
	void (*update)(void);
};

int kernel_snd_init(int fps, int sample_rate, int nchannels, int bit_depth);
void kernel_snd_release(void);
void kernel_snd_set_callback(kernel_snd_callback_t fn);
void kernel_snd_play(void);
void kernel_snd_stop(void);
void kernel_snd_update(void);

#endif
