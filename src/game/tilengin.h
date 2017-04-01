/*
Copyright (c) 2016-2017 Jorge Giner Cordero

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

#ifndef TILENGIN_H
#define TILENGIN_H

#ifndef BMP_H
#include "gamelib/bmp.h"
#endif

#ifndef CBASE_H
#include "cbase/cbase.h"
#endif

/*
 * These values can be changed to config the tile engine.
 */
enum te_config {

	/* 
	 * Decimal bits for virtual pixels. We move in a subpixel
	 * coordinate space, in fixed point integers. The first TE_DBITS
	 * bits are decimals in our coordinates.
	 */
	TE_DBITS = 8,

	/*
	 * A background tile must be square and its size power of 2. The tile
	 * width will be 2 ^ TE_BTBITS.
	 * The size of a foreground tile will be half the size of a
	 * background tile.
	 */
	TE_BTBITS = 4,

	/* Background map number of tiles. */
	TE_BMW = 16,
	TE_BMH = 14,

	/* Max number of sprites. */
	TE_NSPRITES = 32,

	/* Max number of animated tile ids. Must be <= 255. */
	TE_NTILEANIM_PLAYERS = 4,

	/* Max number of shapes. */
	TE_NSHAPES = 16,
};

/* Calculated values. */
enum {
	TE_ONE = 1 << TE_DBITS,
	TE_FTBITS = TE_BTBITS - 1,

	/*
	 * Background and foreground tile width in pixels and in virtual
	 * pixels.
	 */
	TE_BTW = 1 << TE_BTBITS,
	TE_FTW = 1 << TE_FTBITS,
	TE_VBTW = TE_BTW << TE_DBITS,
	TE_VFTW = TE_FTW << TE_DBITS,

	/* Virtual Tile Shift. To map a virtual pixel x to a tile. */
	TE_VBTS = TE_DBITS + TE_BTBITS,
	TE_VFTS = TE_DBITS + TE_FTBITS,

	/* Foreground tiles in horizontal and vertical. */
	TE_FMW = TE_BMW * 2,
	TE_FMH = TE_BMH * 2,

	/* Total tiles in background and foreground maps. */
	TE_BMSZ = TE_BMW * TE_BMH,
	TE_FMSZ = TE_FMW * TE_FMH,

	/* Screen in pixels. */
	TE_SCRW = TE_BMW * TE_BTW,
	TE_SCRH = TE_BMH * TE_BTW,

	/* Screen in virtual pixels. */
	TE_VSCRW = TE_SCRW << TE_DBITS,
	TE_VSCRH = TE_SCRH << TE_DBITS,
};

/* Tile flags. */
enum {
	TF_BOTTOM,
	TF_TOP = 128
};

enum {
	SP_F_FLIPH = FLIPH,
	SP_F_FLIPV = FLIPV,
	SP_F_TOP = 1 << 3,	/* Sprite drawn on top of top tiles. */
};

/* flags is a bitwise or of one TF_ and one TT_. */
struct tileinfo {
	unsigned char tileid;
	unsigned char flags;
};

/* ntiles is the size of tileinfos, which is an array. */
struct tileset {
	struct bmp **ppbmp;
	struct tileinfo *tileinfos;
	int ntiles;
};

struct frame {
	struct bmp **ppbmp;
	struct rect r;
	struct rect *pbbox;
};

/* Background block. */
struct block {
	const char *name;
	struct frame *pframe;
};

/* 'nblocks' is the size of 'blocks', which is an array. */
struct blockset {
	struct block *blocks;
	int nblocks;
};

/* Each field holds an item for each direction. */
struct anim {
	unsigned char nframes;
	struct frame **pframes;
};

/* a0, a1, means argument 0, 1, etc. */
enum {
	TE_SHAPEC_END,
	TE_SHAPEC_COLOR,	/* a0 is color in argb */
	TE_SHAPEC_MOVE,		/* a0, a1 is x, y pos to move */
	TE_SHAPEC_REL_MOVE,	/* a0, a1 is dx, dy to add to the cur pos */
	TE_SHAPEC_LINETO,	/* draw a line from current pos to (a0,a1) */
	TE_SHAPEC_REL_LINETO,	/* draw line from cur pos to cur pos+(a0,a1) */
};

/* A shape, to draw lines, etc.
 * 'code' is a code list that must finish with TE_SHAPEC_END.
 * 'top' is true if this shape must paint in front of the sprite.
 * 'nexti' is an index to another shape, to chain shapes painting.
 */
struct shape {
	int *code;
	char top;
	unsigned char nexti;
};

/* A sprite is valid is pframe != NULL.
 * x, y are in virtual pixels.
 * panim can be NULL.
 * speed is in how many frames we change the frame of the animation:
 * 	0 stopped, 1 -> each 1 frame, 2 -> each 2 frames, etc.
 * speedc is the speed counter (increments until reaches speed).
 * loop if 0 the animation plays and stays in the last frame;
 * 	if 1 the animation is looped.
 * flags is SP_F_ flags.
 * shapei is a shape to paint, only if pframe != NULL.
 * 	If 0, no shape is considered.
 */
struct sprite {
	struct frame *pframe;
	int x, y;
	struct anim *panim;
	unsigned char speed;
	unsigned char speedc;
	unsigned char framei;
	unsigned char loop;
	unsigned char flags;
	unsigned char shapei;
};

/* Tile animation. */
struct tileanim {
	unsigned char *ids;
	int nids;
};

/* Tiles */

int te_tile_type(int tileid);

/* Foreground map */

void te_set_fg_tileset_bmp(int tseti, struct bmp *pbmp);
void te_set_fg_xy(int tx, int ty, int tseti, int tilei);
void te_fg_xy(int tx, int ty, int *tseti, int *tilei);
void te_fill_fg(int tx, int ty, int w, int h, int tseti, int tilei);
void te_clear_fg(int tseti, int tilei);

/* Map */

/* map contains the tile ids.
 * blocks_fname is the name of the file with the background blocks drawlist
 * (max 8 chars + ext):
 * name tx ty
 * name tx ty
 */
struct te_map_info {
	unsigned char map[TE_BMSZ];
	char blocks_fname[13];
};


void te_load_map_to_mem(const char *fname, struct te_map_info *minfo);
void te_set_bg_blockset(struct blockset *blockset);
void te_load_map(struct tileset *tileset, const char *fname);
void te_set_bg_tileset_bmp(struct bmp *pbmp);
void te_set_bg_xy(int tx, int ty, int id);
int te_bg_xy(int tx, int ty);
int te_bg_xy_raw(int tx, int ty);
void te_fill_bg(int tx, int ty, int w, int h, int id);

/* Shapes */

void te_free_shape(int i);
void te_free_shapes(void);
struct shape *te_get_shape(int i);
int te_get_free_shape(int a, int b);

/* Sprites */

struct sprite *te_get_sprite(int i);
void te_free_sprite(struct sprite *sp);
void te_free_sprites(void);
struct sprite *te_get_free_sprite(int a, int b);
void te_get_sprite_bbox(struct rect *r, struct sprite *psp);
int te_sprite_bbox_overlaps(struct sprite *pspa, struct sprite *pspb);
int te_bbox_overlaps(struct rect *a, struct rect *b);

/* Animations */

void te_set_half_speed_mode(int set);

void te_set_anim(struct sprite *psp, struct anim *panim, int speed, int loop);
void te_set_anim_frame(struct sprite *psp, int framei);
void te_stop_anim(struct sprite *psp);
int te_is_anim_playing(struct sprite *psp);

void te_set_tileanim(int tid, struct tileanim *panim, int speed, int loop);

void te_enable_anims(int b);

/* Debug */

extern int te_draw_bboxes;

/* On frame */

extern struct bmp te_screen;

void te_begin_draw(void);
void te_draw(void);
void te_end_draw(void);

#define te_w2p(wx)	asr(wx, TE_DBITS)
#define te_p2w(px)	((px) << TE_DBITS)
#define te_w2t(wx)	asr(wx, TE_VBTS)
#define te_t2w(tx)	((tx) << TE_VBTS)
#define te_p2t(px)	asr(px, TE_BTBITS)
#define te_t2p(tx)	((tx) << TE_BTBITS)
#define te_bgi(tx, ty)	((ty) * TE_BMW + (tx))
#define te_fgi(tx, ty)	((ty) * TE_FMW + (tx))

void tilengin_init(void);

#endif
