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

#include "tilengin.h"
#include "kernel/kernel.h"
#include "gamelib/vfs.h"
#include "cbase/kassert.h"
#include <ctype.h>
#include <string.h>

enum {
	NTILES = 256,		/* must be <= 256 */
	TE_TILESET_COLS = 16,
	FIRST_MAPPED_TILE = NTILES - TE_NTILEANIM_PLAYERS,
	TE_NFG_TILESET = 256
};

struct tileanim_player {
	struct tileanim *panim;
	unsigned char speed;
	unsigned char speedc;
	unsigned char framei;
	unsigned char loop;
};

/* If we touched the bg map.
 * TODO: Do this more efficiently, please: only redraw the touched portions.
 */
static int te_bg_touched;

/* Set to draw the bounding boxes of the frames of the sprites. */
int te_draw_bboxes = 0;

/* Sprite table. */
static struct sprite s_sprites[TE_NSPRITES];

/* Blocks to compose the background. */
static struct blockset *s_bg_blockset;

/* The current map. */
static struct te_map_info s_bg_map_info;

/* Each tile id in s_bg_map is actually associated to another tile id,
 * which is the real tile id we finally use.
 * This is used for animated tiles.
 */
static unsigned char s_tileid_map[NTILES];
static unsigned char s_tileid_map_copy[NTILES];

/* Current animations for each animated tile. */
static struct tileanim_player s_tileanim_players[TE_NTILEANIM_PLAYERS];

/* Block background pixels. */
static unsigned int s_bbg_pixels[TE_SCRW * TE_SCRH];

/* Background where we keep the current block background drawn. */
static struct bmp s_bbg_bmp = {
	.w = TE_SCRW,
	.h = TE_SCRH,
	.pitch = TE_SCRW * 4,
	.palsz = 0,
	.pal = NULL,
	.use_key_color = 0,
	.key_color = 0,
	.pixels = (unsigned char *) s_bbg_pixels
};

/* Background pixels. */
static unsigned int s_bg_pixels[NELEMS(s_bbg_pixels)];

/* Background where we keep the current map drawn. */
static struct bmp s_bg_bmp = {
	.w = TE_SCRW,
	.h = TE_SCRH,
	.pitch = TE_SCRW * 4,
	.palsz = 0,
	.pal = NULL,
	.use_key_color = 0,
	.key_color = 0,
	.pixels = (unsigned char *) s_bg_pixels
};

/* The current tileset. Can be NULL. */
static struct bmp *s_bg_tileset_bmp;

/* The real screen. You can draw directly after a te_begin_draw()
 * and before a te_end_draw(). */
struct bmp te_screen; 

/* Only true if we can draw to te_screen. */
static int s_screen_valid;

/* For each tile, holds its flags. */
static unsigned char s_tile_flags[NTILES];

/* If animations should go twice slower. */
static int s_half_speed_mode;

/* 
 * This holds a bit set for each tile position that has part of a sprite
 * on top of it. We update this info each frame, and use it to paint
 * the tiles with flag TF_TOP after painting the sprites.
 */
enum {
	TOP_MAP_W = (TE_BMW / 8) + ((TE_BMW % 8) > 0),
	TOP_MAP_H = TE_BMH
};

static unsigned char s_top_map[TOP_MAP_W * TOP_MAP_H];

/* Foreground map pixels. */
static unsigned int s_fg_pixels[TE_SCRW * TE_SCRH];

/* Foreground where we keep the current foreground drawn. */
static struct bmp s_fg_bmp = {
	.w = TE_SCRW,
	.h = TE_SCRH,
	.pitch = TE_SCRW * 4,
	.palsz = 0,
	.pal = NULL,
	.use_key_color = 1,
	.key_color = 0x8080,
	.pixels = (unsigned char *) s_fg_pixels
};

/* Foreground map.
 * Each element is 8 hi bits = tileset_bmp, 8 lo bits index.
 */
static unsigned short s_fg_map[TE_FMW * TE_FMH];
static unsigned short s_fg_map_copy[TE_FMW * TE_FMH];
static int s_init_fg_map = 1;

/* The current foreground tilesets. Any can be NULL. */
static struct bmp *s_fg_tileset_bmps[TE_NFG_TILESET];

/* If the animations update. */
static int s_enabled_anims = 1;

/**********************************************************/
/* Foreground map                                         */
/**********************************************************/

/*
 * Sets foreground bitmaps, the ones that contain the tiles.
 * NULL is allowed. tseti must be [0, 255].
 */
void te_set_fg_tileset_bmp(int tseti, struct bmp *pbmp)
{
	if (kassert_fails(tseti >= 0 && tseti < TE_NFG_TILESET))
		return;

	s_fg_tileset_bmps[tseti] = pbmp;
}

/*
 * Sets the position tx, ty of the foreground tilemap to point
 * to the tile (tseti, tilei).
 */
void te_set_fg_xy(int tx, int ty, int tseti, int tilei)
{
	if (kassert_fails(tseti >= 0 && tseti < TE_NFG_TILESET))
		return;
	if (kassert_fails(tilei >= 0 && tilei < NTILES))
		return;
	if (tx < 0 || tx >= TE_FMW || ty < 0 || ty >= TE_FMH)
		return;
	tx = te_fgi(tx, ty);
	if (kassert_fails(tx >= 0 && tx < TE_FMSZ))
		return;
	s_fg_map[tx] = (tseti << 8) | tilei;
}

/*
 * Gets the tile in the foreground map at position tx, ty.
 * *tseti, *tilei will be 0 if tx,ty are out of the map.
 */
void te_fg_xy(int tx, int ty, int *tseti, int *tilei)
{
	int si, ti;

	si = 0; 
	ti = 0;
	if (tx >= 0 && tx < TE_FMW && ty >= 0 && ty < TE_FMH) {
		tx = te_fgi(tx, ty);
		if (kassert(tx >= 0 && tx < TE_FMSZ)) {
			ti = s_fg_map[tx];
			si = (ti >> 8) & 0xff;
			ti &= 0xff;
		}
	}

	if (tseti != NULL)
		*tseti = si;
	if (tilei != NULL)
		*tilei = ti;
}

/* Fills a square of the fg map with tile. */
void te_fill_fg(int tx, int ty, int w, int h, int tseti, int tilei)
{
	int i;

	if (tx < 0) {
		w -= tx;
		tx = 0;
	}

	if (ty < 0) {
		h -= ty;
		ty = 0;
	}

	if (tx + w > TE_FMW)
		w = TE_FMW - (tx + w) + 1;

	if (ty + h > TE_FMH)
		h = TE_FMH - (ty + h) + 1;

	if (w <= 0 || h <= 0)
		return;

	w = tx + w;
	h = ty + h;
	while (ty < h) {
		for (i = tx; i < w; i++) {
			te_set_fg_xy(i, ty, tseti, tilei);
		}
		ty++;
	}
}

void te_clear_fg(int tseti, int tilei)
{
	te_fill_fg(0, 0, TE_FMW, TE_FMH, tseti, tilei);
}

static void flip_fg(void)
{
	draw_bmp(&s_fg_bmp, 0, 0, &te_screen, NULL);
}

static void draw_fg(void)
{
	int tseti;
	int x, y, dx, dy, i;
	struct rect srect;
	unsigned short tid;
	struct bmp *pbmp;

	if (s_init_fg_map) {
		s_init_fg_map = 0;
		for (i = 0; i < TE_FMSZ; i++)
			s_fg_map_copy[i] = s_fg_map[i] + 1;
	}

	srect.w = TE_FTW;
	srect.h = TE_FTW;
	i = 0;
	dy = 0;
	for (y = 0; y < TE_FMH; y++) {
		dx = 0;
		for (x = 0; x < TE_FMW; x++, i++, dx += TE_FTW) {
			tid = s_fg_map[i];
			if (tid == s_fg_map_copy[i])
				continue;

			s_fg_map_copy[i] = tid;
			tseti = tid >> 8;
			if (kassert_fails(tseti >= 0 && tseti < TE_NFG_TILESET))
				continue;

			pbmp = s_fg_tileset_bmps[tseti];
			if (pbmp == NULL)
				continue;

			tid &= 0xff;
			srect.x = (tid % TE_TILESET_COLS) *
				  TE_FTW;
			srect.y = (tid / TE_TILESET_COLS) *
				  TE_FTW;
			draw_bmp_kct(pbmp, dx, dy, &s_fg_bmp,
				     &srect, 0, 0);
		}
		dy += TE_FTW;
	}
}

/**********************************************************/
/* Map                                                    */
/**********************************************************/

/*
 * Sets the tileset bitmap. NULL is allowed.
 */
void te_set_bg_tileset_bmp(struct bmp *pbmp)
{
	s_bg_tileset_bmp = pbmp;
}

/* NULL is allowed. */
void te_set_bg_blockset(struct blockset *blockset)
{
	s_bg_blockset = blockset;
}

static void reset_bg_map(void)
{
	memset(s_bg_map_info.map, 0, TE_BMSZ);
}

#if 0
/*
 * Clears background to black.
 */
static void clear_bg(void)
{
	memset(s_bg_pixels, 0, sizeof(s_bg_pixels));
}
#endif

/* Copies the block background to the background. */
static void flip_bbg(void)
{
	kasserta(sizeof(s_bg_pixels) == sizeof(s_bbg_pixels));
	memcpy(&s_bg_pixels, &s_bbg_pixels, sizeof(s_bg_pixels)); 
}

/*
 * Clears the block background to black.
 */
static void clear_bbg(void)
{
	memset(s_bbg_pixels, 0, sizeof(s_bbg_pixels));
}

/*
 * Draws the current map to the background.
 */
static void draw_bg(void)
{
	int x, y, dx, dy, tid, i;
	struct rect srect;

	if (s_bg_tileset_bmp == NULL)
		return;

	srect.w = TE_BTW;
	srect.h = TE_BTW;
	i = 0;
	dy = 0;
	for (y = 0; y < TE_BMH; y++) {
		dx = 0;
		for (x = 0; x < TE_BMW; x++) {
			tid = s_tileid_map[s_bg_map_info.map[i++]];
			srect.x = (tid % TE_TILESET_COLS) * TE_BTW;
			srect.y = (tid / TE_TILESET_COLS) * TE_BTW;
			draw_bmp_kct(s_bg_tileset_bmp, dx, dy, &s_bg_bmp,
				     &srect, 1, 0);
			dx += TE_BTW;
		}
		dy += TE_BTW;
	}
}

static int hexa_to_num(int c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	else if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else
		return 0;
}

/*
 * Reads a hexadecimal number, skip whitespace first.
 *
 * Return 0 if ok.
 * -1 if EOF.
 * -2 if not a number found.
 */
static int read_hexnum(FILE *fp, int *num)
{
	int c, n;

	n = 0;
	do {
		c = fgetc(fp);
	} while (isspace(c));

	if (c == EOF)
		return -1;

	if (isxdigit(c)) {
		do {
			n = n * 16 + hexa_to_num(c);
			c = fgetc(fp);
		} while (isxdigit(c));
		*num = n;
		return 0;
	}

	return -2;
}


/*
 * Loads tile map from a file into the struct minfo.
 * Loads the map "data/" + fname.
 */
void te_load_map_to_mem(const char *fname, struct te_map_info *minfo)
{
	FILE *fp;
	int i, r, id;
	char cfname[18];

	minfo->blocks_fname[0] = '\0';

	kassert(strlen(fname) <= 12);

	snprintf(cfname, sizeof(cfname), "data/%s", fname);

	fp = open_file(cfname, NULL);
	if (fp == NULL) {
		ktrace("cannot open %s", cfname);
		return;
	}

	fscanf(fp, " %12s ", minfo->blocks_fname);
	if (minfo->blocks_fname[0] == '~')
		minfo->blocks_fname[0] = '\0';

	r = 0;
	for (i = 0; i < TE_BMSZ && r == 0; i++) {
		r = read_hexnum(fp, &id);
		if (r == 0) {
			if (id < 0 || id > 255) {
				ktrace("id out of range in map %s (%d)",
					fname, id);
				id = 0;
			}
			minfo->map[i] = (unsigned char) id;
		}
	}

	if (r == -1) {
		ktrace("premature end of map %s", fname);
	} else if (r == -2) {
		ktrace("hexa number expected in map %s",
			fname);
	}

	fclose(fp);
}

/*
 * Loads the map "data/" + fname.
 */
static void load_map_from_file(const char *fname)
{
	te_load_map_to_mem(fname, &s_bg_map_info);
}

static void reset_tileid_map(void)
{
	int i;

	for (i = 0; i < NTILES; i++) {
		s_tileid_map[i] = i;
		s_tileid_map_copy[i] = i;
	}
}

static void fill_tile_flags(struct tileinfo *tileinfos, int ntiles)
{
	int i;

	if (kassert_fails(ntiles >= 0))
		return;

	if (kassert_fails(ntiles <= NTILES))
		ntiles = NTILES;

	for (i = 0; i < NTILES; i++)
		s_tile_flags[i] = TF_BOTTOM;

	for (i = 0; i < ntiles; i++)
		s_tile_flags[tileinfos[i].tileid] = tileinfos[i].flags;
}

/* Copies a BTW x BTW tile from bbg to bg. */
static void copy_bbg_tile(int dx, int dy)
{
	struct rect r;

	r.x = dx;
	r.y = dy;
	r.w = TE_BTW;
	r.h = TE_BTW;
	draw_bmp(&s_bbg_bmp, dx, dy, &s_bg_bmp, &r);
}

static void draw_bbg_block(const char *name, int tx, int ty)
{
	int i;
	struct frame *pframe;

	pframe = NULL;
	for (i = 0; i < s_bg_blockset->nblocks; i++) {
		if (strcmp(name, s_bg_blockset->blocks[i].name) == 0) {
			pframe = s_bg_blockset->blocks[i].pframe;
			break;
		}
	}

	if (pframe != NULL && pframe->ppbmp != NULL &&
		*pframe->ppbmp != NULL)
       	{
		draw_bmp(*pframe->ppbmp,
			tx * TE_BTW, ty * TE_BTW,
			&s_bbg_bmp, &pframe->r);
	}
}

static void draw_bbg_blocks()
{
	FILE *fp;
	int r, tx, ty;
	char name[9];
	char fname[64];

	if (s_bg_blockset == NULL)
		return;

	if (s_bg_map_info.blocks_fname[0] == '\0')
		return;

	snprintf(fname, sizeof(fname), "data/%s", s_bg_map_info.blocks_fname);
	fp = open_file(fname, NULL);
	if (fp == NULL) {
		ktrace("cannot open %s", fname);
		return;
	}

	r = fscanf(fp, " %8s ", name);
	while (r == 1 && strcmp(name, ".") != 0) {
		r = fscanf(fp, " %d %d ", &tx, &ty);
		if (r != 2) {
			break;
		}
		draw_bbg_block(name, tx, ty);
		r = fscanf(fp, " %8s ", name);
	}

	fclose(fp);
}

/*
 * Loads a map from "data/" + fname.
 */
void te_load_map(struct tileset *tileset, const char *fname)
{
	reset_clip(TE_SCRW, TE_SCRH);
	reset_bg_map();
	te_set_bg_tileset_bmp(*tileset->ppbmp);
	fill_tile_flags(tileset->tileinfos, tileset->ntiles);
	reset_tileid_map();
	load_map_from_file(fname);
	clear_bbg();
	draw_bbg_blocks();
	flip_bbg();
	draw_bg();
}

/* The background is normally fixed, we only flip it to the screen.
 * But if we have animated tiles, we update the animated backgroud portions.
 */
static void update_bg(void)
{
	int x, y, dx, dy, tid, i;
	struct rect srect;

	if (s_bg_tileset_bmp == NULL)
		goto end;

	srect.w = TE_BTW;
	srect.h = TE_BTW;
	i = 0;
	dy = 0;
	for (y = 0; y < TE_BMH; y++) {
		dx = 0;
		for (x = 0; x < TE_BMW; x++) {
			tid = s_bg_map_info.map[i++];
			if (s_tileid_map[tid] != s_tileid_map_copy[tid]) {
				tid = s_tileid_map[tid];
				srect.x = (tid % TE_TILESET_COLS) * TE_BTW;
				srect.y = (tid / TE_TILESET_COLS) * TE_BTW;
				copy_bbg_tile(dx, dy);
				draw_bmp_kct(s_bg_tileset_bmp, dx, dy,
					     &s_bg_bmp, &srect, 1, 0);
			}
			dx += TE_BTW;
		}
		dy += TE_BTW;
	}

end:
	memcpy(&s_tileid_map_copy, &s_tileid_map, sizeof(s_tileid_map));
}

/*
 * Copies the background to the foreground.
 */
static void flip_bg(void)
{
#if 1
	int y;
	unsigned char *src, *dst;

	dst = te_screen.pixels;
	src = s_bg_bmp.pixels;
	for (y = 0; y < TE_SCRH; y++) {
		memcpy(dst, src, TE_SCRW * 4);
		src += te_screen.pitch;
		dst += s_bg_bmp.pitch;
	}
#else
	draw_bmp_kct(&s_bg_bmp, 0, 0, &te_screen, NULL, 0, 0);
#endif
}

/**********************************************************/
/* Top tiles                                              */
/**********************************************************/

#define is_top_tile(tid) ((s_tile_flags[tid] & TF_TOP) != 0)

static void reset_top_map(void)
{
	memset(s_top_map, 0, TOP_MAP_W * TOP_MAP_H);
}

/*
 * Marks with bits in s_top_map, the tile positions that should be
 * repainted.
 * x, y, w, h are in pixels.
 */
static void mark_top_tiles(int x, int y, int w, int h)
{
	int tx, ty, tx2, ty2, xt;

	tx = te_p2t(x);
	ty = te_p2t(y);
	tx2 = te_p2t(x + w - 1);
	ty2 = te_p2t(y + h - 1);
	if (tx2 < 0 || ty2 < 0 || tx >= TE_BMW || ty >= TE_BMH)
		return;
	if (tx < 0)
		tx = 0;
	if (ty < 0)
		ty = 0;
	if (tx2 >= TE_BMW)
		tx2 = TE_BMW - 1;
	if (ty2 >= TE_BMH)
		ty2 = TE_BMH - 1;
	for ( ; ty <= ty2; ty++) {
		for (xt = tx; xt <= tx2; xt++) {
			s_top_map[ty * TOP_MAP_W + (xt >> 3)] |=
				(unsigned char) 1 << (xt & 7);
		}
	}
}

static void draw_top_tiles(void)
{
	int x, y, xb, tid;
	unsigned char *tmap;
	struct rect srect;

	if (s_bg_tileset_bmp == NULL)
		return;

	tmap = s_top_map;
	srect.w = TE_BTW;
	srect.h = TE_BTW;
	for (y = 0; y < TOP_MAP_H; y++) {
		for (xb = 0; xb < TOP_MAP_W; xb++) {
			unsigned char b = *tmap++;
			x = xb << 3;
			while (b != 0) {
				if (b & 1) {
					tid = s_bg_map_info.map[te_bgi(x, y)];
					tid = s_tileid_map[tid];
					if (is_top_tile(tid)) {
						srect.x =
							(tid % TE_TILESET_COLS)
							<< TE_BTBITS;
						srect.y =
						       	(tid / TE_TILESET_COLS)
						       	<< TE_BTBITS;
						draw_bmp(s_bg_tileset_bmp,
							x << TE_BTBITS,
						       	y << TE_BTBITS,
							&te_screen, &srect);
					}
				}
				b >>= 1;
				x++;
			}
		}
	}
}

void te_set_bg_xy(int tx, int ty, int id)
{
	/* disabled, why? */
	/* assert(0); */
	if (kassert_fails(id >= 0 && id < NTILES))
		return;
	if (tx < 0 || ty < 0 || tx >= TE_BMW || ty >= TE_BMH)
		return;
	s_bg_map_info.map[te_bgi(tx, ty)] = id;
	te_bg_touched = 1;
}

/*
 * Returns tile id at tile position x, y.
 * If outside the map, always returns 0.
 * If that tile id has been mapped to another tile id, returns the
 * mapped tile id.
 * To get the tile id unmapped, use te_bg_xy_raw().
 */
int te_bg_xy(int tx, int ty)
{
	if (tx < 0 || ty < 0 || tx >= TE_BMW || ty >= TE_BMH)
		return 0;
	return s_tileid_map[s_bg_map_info.map[te_bgi(tx, ty)]];
};

int te_bg_xy_raw(int tx, int ty)
{
	if (tx < 0 || ty < 0 || tx >= TE_BMW || ty >= TE_BMH)
		return 0;
	return s_bg_map_info.map[te_bgi(tx, ty)];
};

int te_tile_type(int tileid)
{
	if (kassert_fails(tileid >= 0 && tileid < NTILES))
		return TF_BOTTOM;

	return s_tile_flags[tileid];
}

/* Fills a square of the map with tile id. */
void te_fill_bg(int tx, int ty, int w, int h, int id)
{
	int i;

	if (tx < 0) {
		w -= tx;
		tx = 0;
	}

	if (ty < 0) {
		h -= ty;
		ty = 0;
	}

	if (tx + w > TE_BMW)
		w = TE_BMW - (tx + w) + 1;

	if (ty + h > TE_BMH)
		h = TE_BMH - (ty + h) + 1;

	if (w <= 0 || h <= 0)
		return;

	w = tx + w;
	h = ty + h;
	while (ty < h) {
		for (i = tx; i < w; i++) {
			te_set_bg_xy(i, ty, id);
		}
		ty++;
	}
}

/**********************************************************/
/* Shapes                                                 */
/**********************************************************/

static struct shape s_shapes[TE_NSHAPES];

/*
 * Frees the shape i, all its member are zeroed.
 */
void te_free_shape(int i)
{
	if (kassert_fails(i > 0 && i < TE_NSHAPES))
		return;
	memset(&s_shapes[i], 0, sizeof(s_shapes[i]));
}

/*
 * Frees all the sprites.
 */
void te_free_shapes(void)
{
	memset(s_shapes, 0, sizeof(s_shapes));
}

/* i must be >= 1 and < TE_NSHAPES. */
struct shape *te_get_shape(int i)
{
	if (kassert_fails(i > 0 && i < TE_NSHAPES))
		i = 1;
	return &s_shapes[i];
}

/*
 * Gets a free shape index in the range [a, b[
 * Returns 0 if there is not any.
 */
int te_get_free_shape(int a, int b)
{
	if (a <= 0) {
		a = 1;
	}
	if (b > TE_NSHAPES) {
		b = TE_NSHAPES;
	}
	while (a < b) {
		if (s_shapes[a].code == NULL) {
			return a;
		}
		a++;
	}
	return 0;
}

static void draw_shape(struct shape *psh, int x, int y)
{
	int pc;
	int dx, dy;
	int draw;
	unsigned int color;

	if (kassert_fails(psh->code != NULL))
		return;

	x = te_w2p(x);
	y = te_w2p(y);
	pc = 0;
	color = 0;
	draw = 1;
	while (draw) {
		switch (psh->code[pc++]) {
		case TE_SHAPEC_COLOR:
			color = (unsigned int) psh->code[pc++];
			set_draw_color(color);
			break;
		case TE_SHAPEC_MOVE:
			x = psh->code[pc++];
			y = psh->code[pc++];
			break;
		case TE_SHAPEC_REL_MOVE:
			x += psh->code[pc++];
			y += psh->code[pc++];
			break;
		case TE_SHAPEC_LINETO:
			dx = psh->code[pc++];
			dy = psh->code[pc++];
			draw_line(&te_screen, x, y, dx, dy);
			break;
		case TE_SHAPEC_REL_LINETO:
			dx = psh->code[pc++];
			dy = psh->code[pc++];
			draw_line(&te_screen, x, y, x + dx, y + dy);
			break;
		default:
			draw = 0;
			break;
		}
	}
}

static void draw_sprite_shapes(struct sprite *psp, int top)
{
	int i;
	struct shape *psh;

	i = psp->shapei;
	while (i > 0 && i < TE_NSHAPES) {
		psh = &s_shapes[i];
		if (psh->code != NULL && psh->top == top) {
			draw_shape(psh, psp->x, psp->y);
		}

		if (psh->nexti > i)
			i = psh->nexti;
		else
			break;
	}
}

/**********************************************************/
/* Sprites                                                */
/**********************************************************/

struct sprite *te_get_sprite(int i)
{
	if (kassert_fails(i >= 0 && i < TE_NSPRITES))
		i = i % TE_NSPRITES;
	return &s_sprites[i];
}

/*
 * Frees the sprite sp, all its member are zeroed.
 */
void te_free_sprite(struct sprite *sp)
{
	if (kassert_fails(sp != NULL))
		return;
	memset(sp, 0, sizeof(*sp));
}

/*
 * Frees all the sprites.
 */
void te_free_sprites(void)
{
	memset(s_sprites, 0, sizeof(s_sprites));
}

/*
 * Gets a free sprite in the range [a, b[
 * Returns null there is not any.
 */
struct sprite *te_get_free_sprite(int a, int b)
{
	if (a < 0) {
		a = 0;
	}
	if (b > TE_NSPRITES) {
		b = TE_NSPRITES;
	}
	while (a < b) {
		if (s_sprites[a].pframe == NULL) {
			return &s_sprites[a];
		}
		a++;
	}
	return NULL;
}

/*
 * Fills r with the bounding box of the current frame of the sprite
 * in world coordinates, displaced to the sprite position,
 * and considering the sprite transform flags (FLIPH, FLIPV).
 * Sets r.w and r.h as 0 if there is not a bounding box info;
 * r.x and r.y will have the sprite x and y position.
 */
void te_get_sprite_bbox(struct rect *r, struct sprite *psp)
{
	if (psp->pframe == NULL || psp->pframe->pbbox == NULL) {
		r->x = psp->x;
		r->y = psp->y;
		r->w = r->h = 0;
		return;
	}

	*r = *psp->pframe->pbbox;

	if (psp->flags & SP_F_FLIPH)
		r->x = psp->pframe->r.w - (r->x + r->w);

	if (psp->flags & SP_F_FLIPV)
		r->y = psp->pframe->r.h - (r->y + r->h);

	r->x = psp->x + te_p2w(r->x);
	r->y = psp->y + te_p2w(r->y);
	r->w = te_p2w(r->w);
	r->h = te_p2w(r->h);
}

int te_sprite_bbox_overlaps(struct sprite *pspa, struct sprite *pspb)
{
	struct rect a, b;

	if (kassert_fails(pspa != NULL && pspb != NULL))
		return 0;
	if (pspa->pframe == NULL || pspb->pframe == NULL)
		return 0;
	if (pspa->pframe->pbbox == NULL || pspb->pframe->pbbox == NULL)
		return 0;

	/* This is working in virtual pixel space. */
	te_get_sprite_bbox(&a, pspa);
	te_get_sprite_bbox(&b, pspb);
	return te_bbox_overlaps(&a, &b);
}

int te_bbox_overlaps(struct rect *a, struct rect *b)
{
	if (a->x + a->w <= b->x)
		return 0;
	if (a->y + a->h <= b->y)
		return 0;
	if (a->x >= b->x + b->w)
		return 0;
	if (a->y >= b->y + b->h)
		return 0;

#if 0
	ktrace("%d %d %d %d %d %d %d %d",
		a->x, a->y, a->w, a->h,
		b->x, b->y, b->w, b->h);
#endif

	return 1;
}

static void draw_frame_bbox(struct bmp *bmp, int x, int y,
			    struct frame *pframe, int flags)
{
	int a, b, c, d;
	struct rect r;

	if (pframe->pbbox == NULL) {
		return;
	}

	r = *pframe->pbbox;
	if (flags & SP_F_FLIPH) {
		r.x = pframe->r.w - (r.x + r.w);
	}
	if (flags & SP_F_FLIPV) {
		r.y = pframe->r.h - (r.y + r.h);
	}
	a = x + r.x;
	b = y + r.y;
	c = a + r.w - 1;
	d = b + r.h - 1;
	set_draw_color(0x00ff0000);
	draw_line(bmp, a, b, c, b);
	draw_line(bmp, a, d, c, d);
	draw_line(bmp, a, b, a, d);
	draw_line(bmp, c, b, c, d);
}

static void draw_sprite(struct sprite *psp)
{
	int x, y;
	struct frame *pfr;
	struct bmp *pim;

	pfr = psp->pframe;
	if (pfr == NULL || pfr->ppbmp == NULL)
		return;

	pim = *pfr->ppbmp;
	if (pim == NULL)
		return;

	draw_sprite_shapes(psp, 0);
	x = te_w2p(psp->x);
	y = te_w2p(psp->y);
	draw_bmp_kct(pim, x, y, &te_screen, &pfr->r, 1, psp->flags & 3);
	if ((psp->flags & SP_F_TOP) == 0) {
		mark_top_tiles(x, y, pfr->r.w, pfr->r.h);
	}
	if (te_draw_bboxes && pfr->pbbox != NULL) {
		draw_frame_bbox(&te_screen, x, y, pfr, psp->flags & 3);
	}
	draw_sprite_shapes(psp, 1);
}

/* 
 * If top, draws sprites with SP_F_TOP only.
 * If !top, draws sprites without SP_F_TOP.
 */
static void draw_sprites(int top)
{
	int i, is_top;

	for (i = 0; i < TE_NSPRITES; i++) {
		is_top = s_sprites[i].flags & SP_F_TOP;
		if ((top && is_top) || (!top && !is_top)) {
			draw_sprite(&s_sprites[i]);
		}
	}
}

/**********************************************************/
/* Animations                                             */
/**********************************************************/

/*
 * If set != 0, the animation speed specified by calling te_set_anim()
 * will be multiplied by 2, so the animation will be twice slower.
 */
void te_set_half_speed_mode(int set)
{
	s_half_speed_mode = set;
}


/**********************************************************/
/* Tile animations                                        */
/**********************************************************/

/*
 * Maps the tile tapid + FIRST_MAPPED_TILE to the tile set by the
 * animation frame in s_tileanim_players[tapid].
 */
static void update_tileid_map(int tapid)
{
	struct tileanim_player *ptap;

	if (kassert_fails(tapid >= 0 && tapid < TE_NTILEANIM_PLAYERS))
		return;

	ptap = &s_tileanim_players[tapid];
	s_tileid_map[tapid + FIRST_MAPPED_TILE] =
		ptap->panim->ids[ptap->framei];
}

static void update_tileanim_player(int tapid)
{
	struct tileanim_player *ptap;
	unsigned char framei;

	if (kassert_fails(tapid >= 0 && tapid < TE_NTILEANIM_PLAYERS))
		return;

	ptap = &s_tileanim_players[tapid];
	if (ptap->panim == NULL)
		return;

	if (ptap->speedc > 0)
		ptap->speedc--;

	if (ptap->speedc > 0 || ptap->speed == 0) {
		update_tileid_map(tapid);
		return;
	}

	framei = ptap->framei + 1;
	if (framei == ptap->panim->nids)
		framei = 0;

	if (framei == 0 && !ptap->loop) {
	       ptap->speed = 0;	
	       return;
	}

	ptap->framei = framei;
	ptap->speedc = ptap->speed;
	update_tileid_map(tapid);
}

/* Maps the tile tid to an animation.
 *
 * tid: the tile id to map. Only tiles from 0xf0 to 0xff can be mapped.
 *
 * speed: 0 stopped, 1 change each frame, 2 change each 2 frames, etc.
 * If less than 0 will be set to 0; if greater than 255 will be set to 255.
 *
 * loop: 0 no loop, else loop.
 */
void te_set_tileanim(int tid, struct tileanim *panim, int speed, int loop)
{
	if (kassert_fails(tid >= FIRST_MAPPED_TILE && tid < NTILES))
		return;

	if (kassert_fails(panim != NULL))
		return;

	if (s_half_speed_mode)
		speed <<= 1;
	if (speed < 0)
		speed = 0;
	else if (speed > 255)
		speed = 255;

	tid -= FIRST_MAPPED_TILE;
	s_tileanim_players[tid].panim = panim;
	s_tileanim_players[tid].speed = speed;
	s_tileanim_players[tid].speedc = speed;
	s_tileanim_players[tid].framei = 0;
	s_tileanim_players[tid].loop = loop;
	update_tileid_map(tid);
}

/**********************************************************/
/* Sprite animations                                      */
/**********************************************************/
/*
 * Updates pframe of psp with the current animation frame.
 */
static void update_spr_anim(struct sprite *psp)
{
	psp->pframe = psp->panim->pframes[psp->framei];
}

/*
 * Goes to the next frame of animation and handles looped animations.
 * Updates the sprite pframe accordingly.
 */
static void update_anim(struct sprite *psp)
{
	unsigned char framei;

	if (psp->pframe == NULL || psp->panim == NULL)
		return;

	if (psp->speedc > 0)
		psp->speedc--;

	if (psp->speedc > 0 || psp->speed == 0) {
		update_spr_anim(psp);
		return;
	}

	framei = psp->framei + 1;
	if (framei == psp->panim->nframes)
		framei = 0;

	if (framei == 0 && !psp->loop) {
	       psp->speed = 0;	
	       return;
	}

	psp->framei = framei;
	psp->speedc = psp->speed;
	update_spr_anim(psp);
}

/*
 * Set the animation for a sprite and set ready the first frame of the
 * animation.
 *
 * speed: 0 stopped, 1 change each frame, 2 change each 2 frames, etc.
 * If less than 0 will be set to 0; if greater than 255 will be set to 255.
 *
 * loop: 0 no loop, else loop.
 */
void te_set_anim(struct sprite *psp, struct anim *panim, int speed, int loop)
{
	if (s_half_speed_mode)
		speed <<= 1;
	if (speed < 0)
		speed = 0;
	else if (speed > 255)
		speed = 255;
	psp->panim = panim;
	psp->speed = speed;
	psp->speedc = speed;
	psp->framei = 0;
	psp->loop = loop;
	update_spr_anim(psp);
}

void te_set_anim_frame(struct sprite *psp, int framei)
{
	if (kassert_fails(framei >= 0))
		return;
	if (psp->panim == NULL || psp->panim->nframes == 0)
		return;
	psp->framei = framei % psp->panim->nframes;
	update_spr_anim(psp);
}

/*
 * Stops a sprite animation in the current frame.
 */
void te_stop_anim(struct sprite *psp)
{
	psp->speed = 0;
	psp->speedc = 0;
}

/*
 * Returns 1 if an animation is set and its speed is greater than 0.
 * Returns 0 otherwise.
 */
int te_is_anim_playing(struct sprite *psp)
{
	if (psp->panim == NULL)
		return 0;
	if (psp->speed == 0)
		return 0;
	return 1;
}

/* Disable the animations update. */
void te_enable_anims(int b)
{
	s_enabled_anims = b;
}

static void update_anims(void)
{
	int i;

	if (!s_enabled_anims)
		return;

	for (i = 0; i < TE_NSPRITES; i++)
		update_anim(&s_sprites[i]);

	for (i = 0; i < TE_NTILEANIM_PLAYERS; i++)
		update_tileanim_player(i);
}

/**********************************************************/
/* New frame                                              */
/**********************************************************/

void te_begin_draw(void)
{
	const struct kernel_device *d;
	struct kernel_canvas *kcanvas;

	d = kernel_get_device();
	kcanvas = d->get_canvas();

	te_screen.pixels = (unsigned char *) kcanvas->pixels;
	te_screen.w = kcanvas->w;
	te_screen.h = TE_SCRH;
	te_screen.pitch = kcanvas->pitch;

	reset_clip(te_screen.w, te_screen.h);
	s_screen_valid = 1;
}

void te_draw(void)
{
	if (kassert_fails(s_screen_valid))
		return;

	if (te_bg_touched) {
		te_bg_touched = 0;
		flip_bbg();
		draw_bg();
	}

	update_bg();
	flip_bg();
	draw_sprites(0);
	draw_top_tiles();
	reset_top_map();
	draw_sprites(1);
	draw_fg();
	flip_fg();
	update_anims();
}

void te_end_draw(void)
{
	s_screen_valid = 0;
}

void tilengin_init(void)
{
	te_bg_touched = 0;
	te_draw_bboxes = 0;
	te_free_sprites();
	s_bg_blockset = NULL;
	reset_bg_map();
	reset_tileid_map();
	memset(s_tileanim_players, 0, sizeof(s_tileanim_players));
	clear_bbg();
	s_bg_tileset_bmp = NULL;
	s_screen_valid = 0;
	memset(s_tile_flags, 0, sizeof(s_tile_flags));
	s_half_speed_mode = 0;
	reset_top_map();
	s_init_fg_map = 1;
	memset(s_fg_tileset_bmps, 0, sizeof(s_fg_tileset_bmps));
	s_enabled_anims = 1;
	te_free_shapes();
}
