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

#include "mixer.h"
#include "wav.h"
#include "cbase/cbase.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#if 0
/* This table is for fast scaling between 8 bit samples and 16 bit samples */
static const short s_byte_word0[] = {
	-32768, -32511, -32254, -31997, -31740, -31483, -31226, -30969,
	-30712, -30455, -30198, -29941, -29684, -29427, -29170, -28913,
	-28656, -28399, -28142, -27885, -27628, -27371, -27114, -26857,
	-26600, -26343, -26086, -25829, -25572, -25315, -25058, -24801,
	-24544, -24287, -24030, -23773, -23516, -23259, -23002, -22745,
	-22488, -22231, -21974, -21717, -21460, -21203, -20946, -20689,
	-20432, -20175, -19918, -19661, -19404, -19147, -18890, -18633,
	-18376, -18119, -17862, -17605, -17348, -17091, -16834, -16577,
	-16320, -16063, -15806, -15549, -15292, -15035, -14778, -14521,
	-14264, -14007, -13750, -13493, -13236, -12979, -12722, -12465,
	-12208, -11951, -11694, -11437, -11180, -10923, -10666, -10409,
	-10152, -9895, -9638, -9381, -9124, -8867, -8610, -8353,
	-8096, -7839, -7582, -7325, -7068, -6811, -6554, -6297,
	-6040, -5783, -5526, -5269, -5012, -4755, -4498, -4241,
	-3984, -3727, -3470, -3213, -2956, -2699, -2442, -2185,
	-1928, -1671, -1414, -1157, -900, -643, -386, -129,
	128, 385, 642, 899, 1156, 1413, 1670, 1927,
	2184, 2441, 2698, 2955, 3212, 3469, 3726, 3983,
	4240, 4497, 4754, 5011, 5268, 5525, 5782, 6039,
	6296, 6553, 6810, 7067, 7324, 7581, 7838, 8095,
	8352, 8609, 8866, 9123, 9380, 9637, 9894, 10151,
	10408, 10665, 10922, 11179, 11436, 11693, 11950, 12207,
	12464, 12721, 12978, 13235, 13492, 13749, 14006, 14263,
	14520, 14777, 15034, 15291, 15548, 15805, 16062, 16319,
	16576, 16833, 17090, 17347, 17604, 17861, 18118, 18375,
	18632, 18889, 19146, 19403, 19660, 19917, 20174, 20431,
	20688, 20945, 21202, 21459, 21716, 21973, 22230, 22487,
	22744, 23001, 23258, 23515, 23772, 24029, 24286, 24543,
	24800, 25057, 25314, 25571, 25828, 26085, 26342, 26599,
	26856, 27113, 27370, 27627, 27884, 28141, 28398, 28655,
	28912, 29169, 29426, 29683, 29940, 30197, 30454, 30711,
	30968, 31225, 31482, 31739, 31996, 32253, 32510, 32767
};

static const short s_byte_word1[] = {
	-32768, -32512, -32256, -32000, -31744, -31488, -31232, -30976,
	-30720, -30464, -30208, -29952, -29696, -29440, -29184, -28928,
	-28672, -28416, -28160, -27904, -27648, -27392, -27136, -26880,
	-26624, -26368, -26112, -25856, -25600, -25344, -25088, -24832,
	-24576, -24320, -24064, -23808, -23552, -23296, -23040, -22784,
	-22528, -22272, -22016, -21760, -21504, -21248, -20992, -20736,
	-20480, -20224, -19968, -19712, -19456, -19200, -18944, -18688,
	-18432, -18176, -17920, -17664, -17408, -17152, -16896, -16640,
	-16384, -16128, -15872, -15616, -15360, -15104, -14848, -14592,
	-14336, -14080, -13824, -13568, -13312, -13056, -12800, -12544,
	-12288, -12032, -11776, -11520, -11264, -11008, -10752, -10496,
	-10240, -9984, -9728, -9472, -9216, -8960, -8704, -8448,
	-8192, -7936, -7680, -7424, -7168, -6912, -6656, -6400,
	-6144, -5888, -5632, -5376, -5120, -4864, -4608, -4352,
	-4096, -3840, -3584, -3328, -3072, -2816, -2560, -2304,
	-2048, -1792, -1536, -1280, -1024, -768, -512, -256,
	0, 256, 512, 768, 1024, 1280, 1536, 1792,
	2048, 2304, 2560, 2816, 3072, 3328, 3584, 3840,
	4096, 4352, 4608, 4864, 5120, 5376, 5632, 5888,
	6144, 6400, 6656, 6912, 7168, 7424, 7680, 7936,
	8192, 8448, 8704, 8960, 9216, 9472, 9728, 9984,
	10240, 10496, 10752, 11008, 11264, 11520, 11776, 12032,
	12288, 12544, 12800, 13056, 13312, 13568, 13824, 14080,
	14336, 14592, 14848, 15104, 15360, 15616, 15872, 16128,
	16384, 16640, 16896, 17152, 17408, 17664, 17920, 18176,
	18432, 18688, 18944, 19200, 19456, 19712, 19968, 20224,
	20480, 20736, 20992, 21248, 21504, 21760, 22016, 22272,
	22528, 22784, 23040, 23296, 23552, 23808, 24064, 24320,
	24576, 24832, 25088, 25344, 25600, 25856, 26112, 26368,
	26624, 26880, 27136, 27392, 27648, 27904, 28160, 28416,
	28672, 28928, 29184, 29440, 29696, 29952, 30208, 30464,
	30720, 30976, 31232, 31488, 31744, 32000, 32256, 32512,
};

#define u8tos16(a) s_byte_word[a]
#endif

#define u8tos16(a) (((short) (a) - 128) << 8)

struct mixer_sound {
	struct wav *pwav;
};

struct channel_queue {
	struct channel_queue *next;
	struct mixer_sound msound;
	int sound_id;	/* Sound position in s_sounds. */
	int loop;
	int pos;	/* Next sample that should play in 44kHz resolution. */
};

struct channel {
	struct channel_queue *first;
	struct channel_queue *last;
	int n;
};

static int s_volume;

static struct channel_queue s_nodes[MIXER_NCHANNELS * MIXER_NQUEUE];
static struct channel_queue *s_free_node; 

static struct channel s_channels[MIXER_NCHANNELS];

/* To sum two 16 bit samples. */
// #define sum(a, b) (((int) (a) + (int) (b)) >> 1)
short sum(short a, short b)
{
	int i;

	i = (int) a + b;
	if (i > SHRT_MAX)
		return SHRT_MAX;
	else if (i < SHRT_MIN)
		return SHRT_MIN;
	else
		return i;
}

/* Extracts a node from the pool.
 * If none is left or the channel has got MIXER_NQUEUE sounds queued,
 * NULL is returned.
 */
static struct channel_queue *get_node_for_channel(int chanid)
{
	struct channel_queue *pnode;

	if (chanid < 0 || chanid >= MIXER_NCHANNELS)
		return NULL;

	if (s_channels[chanid].n == MIXER_NQUEUE)
		return NULL;

	pnode = s_free_node;
	if (pnode == NULL)
		return NULL;

	s_free_node = pnode->next;
	return pnode;
}

/* Return a node to the pool. */
static void store_node(struct channel_queue *pnode)
{
	pnode->next = s_free_node;
	s_free_node = pnode;
}

/* Finds a free channel in range [a, b[ and returns the channel index
 * or - 1. */
int mixer_get_free_channel_r(int a, int b)
{
	int i;

	if (b > MIXER_NCHANNELS)
		b = MIXER_NCHANNELS;

	for (i = a; i < b; i++) {
		if (s_channels[i].first == NULL)
			return i;
	}

	return -1;
}

/* Finds a free channel and returns the channel index or - 1. */
int mixer_get_free_channel(void)
{
	return mixer_get_free_channel_r(0, MIXER_NCHANNELS);
}

/* 
 * Queues sound 'pwav' in 'chanid' for playing.
 *
 * Returns 0 on success, different on failure.
 */
int mixer_queue(int chanid, struct wav *pwav, int loop)
{
	struct channel *ch;
	struct channel_queue *node;

	if (pwav == NULL)
		return -1;

	if (chanid < 0 || chanid >= MIXER_NCHANNELS)
		return -1;

	if ((node = get_node_for_channel(chanid)) == NULL)
		return -1;

	node->msound.pwav = pwav;
	node->loop = loop;
	node->pos = 0;
	node->next = NULL;

	ch = &s_channels[chanid];
	if (ch->first == NULL)
		ch->first = node;
	else
		ch->last->next = node;
	
	ch->last = node;
	return 0;
}

/*
 * Like:
 * 	mixer_stop(chanid);
 * 	mixer_queue(chanid, pwac, loop);
 */
int mixer_stop_n_play(int chanid, struct wav *pwav, int loop)
{
	mixer_stop(chanid);
	return mixer_queue(chanid, pwav, loop);
}

/* Like calling mixer_queue(mixer_get_free_channel(), pwav, 0). */
int mixer_play(struct wav *pwav)
{
	return mixer_queue(mixer_get_free_channel(), pwav, 0);
}

/**
 * Returns 1 if 'chanid' is playing (has any sound queued).
 */
int mixer_is_playing(int chanid)
{
	if (chanid < 0 || chanid >= MIXER_NCHANNELS)
		return 0;

	if (s_channels[chanid].first == NULL)
		return 0;

	return 1;
}

/* All sounds queued in 'chanid' will be discarded. */
void mixer_stop(int chanid)
{
	struct channel_queue *p, *q;

	if (chanid < 0 || chanid >= MIXER_NCHANNELS)
		return;

	for (q = s_channels[chanid].first; q != NULL; q = p) {
		p = q->next;
		store_node(q);
	}

	s_channels[chanid].first = NULL;
	s_channels[chanid].last = NULL;
}

/* Discards all sounds in all channels. */
void mixer_stop_all(void)
{
	int i;

	for (i = 0; i < MIXER_NCHANNELS; i++)
		mixer_stop(i);
}

/* Frees all resources occupied by channels. */
void mixer_free_channels(void)
{
	if (s_channels != NULL) {
		mixer_stop_all();
	}
}

/* Source is 11025, 8 bit mono. */
static void channel_gen1m08(short *ptr, int nsamples, unsigned char *data,
    int first_sample)
{
	short a;
	int i;

	i = first_sample & 3;
	data += first_sample >> 2;
	while (nsamples--) {
		a = u8tos16(*data);
		*ptr = sum(*ptr, a);
		ptr++;
		*ptr = sum(*ptr, a);
		ptr++;
		i++;
		data += i >> 2;
		i &= 3;
	}
}

/* Source is 11025, 16 bit mono. */
static void channel_gen1m16(short *ptr, int nsamples, unsigned char *data,
			    int first_sample)
{
	short *dat;
	short a;
	int i;

	dat = (short *) data;
	i = first_sample & 3;
	dat += first_sample >> 2;
	while (nsamples--) {
		a = *dat;
		*ptr = sum(*ptr, a);
		ptr++;
		*ptr = sum(*ptr, a);
		ptr++;
		i++;
		dat += i >> 2;
		i &= 3;
	}
}

/* Source is 11025, 8 bit stereo. */
static void channel_gen1s08(short *ptr, int nsamples, unsigned char *data,
    int first_sample)
{
	int i;

	i = first_sample & 3;
	data += (first_sample & ~3) >> 1;
	while (nsamples--) {
		*ptr = sum(*ptr, u8tos16(*data));
		ptr++;
		*ptr = sum(*ptr, u8tos16(*(data + 1)));
		ptr++;
		i++;
		data += (i & ~3) >> 1;
		i &= 3;
	}
}

/* Source is 11025, 16 bit stereo. */
static void channel_gen1s16(short *ptr, int nsamples, unsigned char *data,
			    int first_sample)
{
	short *dat;
	int i;

	dat = (short *) data;
	i = first_sample & 3;
	dat += (first_sample & ~3) >> 1;
	while (nsamples--) {
		*ptr = sum(*ptr, *dat);
		ptr++;
		*ptr = sum(*ptr, *(dat + 1));
		ptr++;
		i++;
		dat += (i & ~3) >> 1;
		i &= 3;
	}
}

/* Source is 22050, 8 bit mono. */
static void channel_gen2m08(short *ptr, int nsamples, unsigned char *data,
			    int first_sample)
{
	int i;
	short a;

	i = first_sample & 1;
	data += first_sample >> 1;
	while (nsamples--) {
		a = u8tos16(*data);
		*ptr = sum(*ptr, a);
		ptr++;
		*ptr = sum(*ptr, a);
		ptr++;
		data += i;
		i ^= 1;
	}
}

/* Source is 22050, 16 bit mono. */
static void channel_gen2m16(short *ptr, int nsamples, unsigned char *data,
			    int first_sample)
{
	short *dat;
	short a;
	int i;

	dat = (short *) data;
	i = first_sample & 1;
	dat += first_sample >> 1;
	while (nsamples--) {
		a = *dat;
		*ptr = sum(*ptr, a);
		ptr++;
		*ptr = sum(*ptr, a);
		ptr++;
		dat += i;
		i ^= 1;
	}
}

/* Source is 22050, 8 bit stereo. */
static void channel_gen2s08(short *ptr, int nsamples, unsigned char *data,
			    int first_sample)
{
	int i;

	i = (first_sample & 1) << 1;
	data += first_sample & ~1;
	while (nsamples--) {
		*ptr = sum(*ptr, u8tos16(*data));
		ptr++;
		*ptr = sum(*ptr, u8tos16(*(data + 1)));
		ptr++;
		data += i;
		i ^= 2;
	}
}

/* Source is 22050, 16 bit stereo. */
static void channel_gen2s16(short *ptr, int nsamples, unsigned char *data,
			    int first_sample)
{
	short *dat;
	int i;

	dat = (short *) data;
	i = (first_sample & 1) << 1;
	dat += first_sample & ~1;
	while (nsamples--) {
		*ptr = sum(*ptr, *dat);
		ptr++;
		*ptr = sum(*ptr, *(dat + 1));
		ptr++;
		dat += i;
		i ^= 2;
	}
}

/* Source is 44100, 8 bit mono. */
static void channel_gen4m08(short *ptr, int nsamples, unsigned char *data,
    int first_sample)
{
	short a;

	data += first_sample;
	while (nsamples--) {
		a = u8tos16(*data++);
		*ptr = sum(*ptr, a);
		ptr++;
		*ptr = sum(*ptr, a);
		ptr++;
	}
}

/* Source is 44100, 16 bit mono. */
static void channel_gen4m16(short *ptr, int nsamples, unsigned char *data,
    int first_sample)
{
	short *dat;
	short a;

	dat = (short *) data;
	dat += first_sample;
	while (nsamples--) {
		a = *dat++;
		*ptr = sum(*ptr, a);
		ptr++;
		*ptr = sum(*ptr, a);
		ptr++;
	}
}

/* Source is 44100, 8 bit stereo. */
static void channel_gen4s08(short *ptr, int nsamples, unsigned char *data,
    int first_sample)
{
	short a, b;

	data += first_sample * 2;
	while (nsamples--) {
		a = u8tos16(*data++);
		b = u8tos16(*data++);
		*ptr = sum(*ptr, a);
		ptr++;
		*ptr = sum(*ptr, b);
		ptr++;
	}
}

/* Source is 44100, 16 bit stereo. */
static void channel_gen4s16(short *ptr, int nsamples, unsigned char *data,
    int first_sample)
{
	short *dat;
	short a, b;

	dat = (short *) data;
	dat += first_sample * 2;
	while (nsamples--) {
		a = *dat++;
		b = *dat++;
		*ptr = sum(*ptr, a);
		ptr++;
		*ptr = sum(*ptr, b);
		ptr++;
	}
}

/**
 * Mixes 'nsamples' of the sounds in 'ch', into 'ptr'.
 *
 * Output format is always 44100, 16 bit stereo.
 */
static void channel_generate(struct channel *ch, short *ptr, int nsamples)
{
	struct channel_queue *q, *p;
	struct wav *pwav;
	int n, tsamples, tmul;

next:	if (ch->first == NULL || nsamples == 0)
		return;

	q = ch->first;
	pwav = q->msound.pwav;

	/* Do operations in target (44100) rate.
	 * q->pos is in target rate.
	 * pwav->nsamples is in original wav rate.
	 * nsamples is in target rate.
	 * n will be the number of samples in target rate.
	 */
	tmul = 44100 / WAV_FMT_SAMPLE_RATE(pwav->format);
	tsamples = pwav->nsamples * tmul;
	n = tsamples - q->pos;
	if (n > nsamples)
		n = nsamples;

	switch (pwav->format) {
	case WAV_FMT_1M08: channel_gen1m08(ptr, n, pwav->data, q->pos); break;
	case WAV_FMT_1M16: channel_gen1m16(ptr, n, pwav->data, q->pos); break;
	case WAV_FMT_1S08: channel_gen1s08(ptr, n, pwav->data, q->pos); break;
	case WAV_FMT_1S16: channel_gen1s16(ptr, n, pwav->data, q->pos); break;
	case WAV_FMT_2M08: channel_gen2m08(ptr, n, pwav->data, q->pos); break;
	case WAV_FMT_2M16: channel_gen2m16(ptr, n, pwav->data, q->pos); break;
	case WAV_FMT_2S08: channel_gen2s08(ptr, n, pwav->data, q->pos); break;
	case WAV_FMT_2S16: channel_gen2s16(ptr, n, pwav->data, q->pos); break;
	case WAV_FMT_4M08: channel_gen4m08(ptr, n, pwav->data, q->pos); break;
	case WAV_FMT_4M16: channel_gen4m16(ptr, n, pwav->data, q->pos); break;
	case WAV_FMT_4S08: channel_gen4s08(ptr, n, pwav->data, q->pos); break;
	case WAV_FMT_4S16: channel_gen4s16(ptr, n, pwav->data, q->pos); break;
	}

	q->pos += n;
	ptr += n * 2;
	nsamples -= n;
	
	if (q->pos == tsamples) {
		/* This wav has been played entirely. */
		if (q->loop) {
			q->pos = 0;
			goto next;
		} else {
			/* We have finished with this node. */
			p = q;
			q = q->next;
			store_node(p);
			ch->first = q;
			if (ch->first == NULL)
				ch->last = NULL;
			else
				goto next;
		}
	}
}

void mixer_set_volume(int vol)
{
	if (vol < 0) {
		vol = 0;
	} else if (vol > 100) {
		vol = 100;
	}
	s_volume = vol;
}

int mixer_get_volume(void)
{
	return s_volume;
}

/**
 * Mixes and advances 'nsamples' from all channels, and generates the mixed
 * data in 'ptr' always in format 44100 samples per second, 16 bit, stereo.
 */
void mixer_generate(short *ptr, int nsamples, int fill_silence)
{
	int i;
	float fvol;
	short *pend;

	if (s_volume > 0 && fill_silence) {
		memset(ptr, 0, nsamples * 4);
	}
	for (i = 0; i < MIXER_NCHANNELS; i++) {
		channel_generate(&s_channels[i], ptr, nsamples);
	}

	/* Apply volume */
	if (s_volume == 0) {
		memset(ptr, 0, nsamples * 4);
	} else if (s_volume < 100) {
		fvol = s_volume / 100.f;
		pend = ptr + nsamples * 2;
		while (ptr != pend) {
			*ptr *= fvol; 
			ptr++;
		}
	}
}

/* Initializes the mixer module.
 */
void mixer_init(void)
{
	int i;

	s_volume = 100;
	for (i = 1; i < NELEMS(s_nodes); i++) {
		s_nodes[i - 1].next = &s_nodes[i];
	}
	s_nodes[NELEMS(s_nodes) - 1].next = NULL;
	s_free_node = &s_nodes[0];
	memset(s_channels, 0, sizeof(s_channels));
}
