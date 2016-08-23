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

void data_init(void);

#endif
