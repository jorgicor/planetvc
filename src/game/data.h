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

#ifndef DATA_H
#define DATA_H

/* A bounding box. */
#define BBOX(name, x, y, w, h) \
struct rect bbox_##name = { x, y, w, h };

/* A portion of a bitmap. */
#define FRAME(name, bmp, x, y, w, h) \
struct frame fr_##name = { \
	.ppbmp = &bmp, \
	.r = { x, y, w, h }, \
	.pbbox = NULL \
};

/* A portion of a bitmap, specifying a bounding box. */
#define FRAMEB(name, bmp, x, y, w, h, bboxname) \
struct frame fr_##name = { \
	.ppbmp = &bmp, \
	.r = { x, y, w, h }, \
	.pbbox = &bbox_##bboxname \
};

/* To define a series of frames for an animation. */
#define BEGIN_ANIM(name) struct frame *frames_##name[] = {
#define ANIM_FRAME(name) &fr_##name,
#define END_ANIM };

/* An animation. */
#define DEFANIM(name) \
struct anim am_##name = { \
	.nframes = NELEMS(frames_##name), \
	.pframes = frames_##name, 	  \
};

/* Tile types. Must be in range [0, 127]. */
enum {
	TT_FREE,
	TT_BLOCK = 1,
	TT_LAVA = 2
};

/* Virtual tiles for animation */
enum {
	TI_WATER = 254,
	TI_LAVA = 255
};

extern struct bmp *bmp_blocks;
extern struct bmp *bmp_tileset;
extern struct bmp *bmp_font;

extern struct tileset main_tileset;
extern struct blockset main_blockset;

extern struct tileanim tileam_lava;
extern struct tileanim tileam_water;

void apply_stargate_symbols(void);

void data_init(void);

#endif
