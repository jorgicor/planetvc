/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "data.h"
#include "tilengin.h"
#include "bitmaps.h"
#include "cfg/cfg.h"
#include <stddef.h>

/* To define a series of tile ids for an animation. */
#define BEGIN_TILEANIM(name) unsigned char tileids_##name[] = {
#define ANIM_TILEID(id) id,
#define END_TILEANIM };

/* A tile animation. */
#define DEFTILEANIM(name) \
struct tileanim tileam_##name = {           \
	.nids = NELEMS(tileids_##name), \
	.ids  = tileids_##name,		\
};

/* A set of tiles. */
#define TILESET(name, bmp, infos) 	\
struct tileset name##_tileset = {	\
	.ppbmp = &bmp,			\
	.tileinfos = infos,		\
	.ntiles = NELEMS(infos)		\
};

/* A list of background blocks. */
#define BLOCKSET(name, infos) 		\
struct blockset name##_blockset = {	\
	.blocks = infos,		\
	.nblocks = NELEMS(infos)	\
};

struct bmp *bmp_blocks = NULL;
struct bmp *bmp_tileset = NULL;
static struct bmp *bmp_stargate = NULL;
static struct bmp *bmp_end = NULL;

enum {
	TI_VOID,
	TI_GRASS_0,
	TI_GRASS_1,
	TI_GRASS_2,
	TI_GRASS_3,
	TI_GRASSTOP_0,
	TI_GRASSTOP_1,
	TI_GRASSBOT_0,
	TI_GRASSBOT_1,
	TI_VOIDBLOCK,
	TI_LAVAROCK_L,
	TI_LAVAROCK_R,
	TI_GRASS_L,
	TI_GRASS_R,
	TI_LAVA_0,
	TI_LAVA_1,
	TI_LAVA_2,
	TI_LAVA_3,
	TI_LAVA_4,
	TI_LAVA_5,
	TI_LAVA_6,
	TI_LAVA_7,
	TI_LAVAROCK_0,
	TI_LAVAROCK_1,
	TI_LAVAROCK_2,
	TI_LAVAROCK_3,
	TI_LAVAARC_L,
	TI_LAVAARC_R,
	TI_GRASSLAVA_L,
	TI_GRASSLAVA_R,
	TI_CAVERN_L,
	TI_CAVERN_R,
	TI_CAVERN_0,
	TI_CAVERN_1,
	TI_CAVERN_2,
	TI_CAVERN_3,
	TI_CAVERN_TOP_0,
	TI_CAVERN_TOP_1,
	TI_CAVERN_BOT_0,
	TI_CAVERN_BOT_1,
	TI_WATER_0,
	TI_WATER_1,
	TI_WATER_2,
	TI_WATER_3,
	TI_WATER_4,
	TI_WATER_5,
	TI_WATER_6,
	TI_WATER_7,
	TI_LAVAROCK_L_BLOCK = 0x5C,
	TI_LAVAARC_R_BLOCK,
};

static struct tileinfo s_tileinfos[] = {
	{ TI_VOID, TT_FREE },
	{ TI_GRASS_0, TT_BLOCK },
	{ TI_GRASS_1, TT_BLOCK },
	{ TI_GRASS_2, TT_BLOCK },
	{ TI_GRASS_3, TT_BLOCK },
	{ TI_GRASSTOP_0, TT_FREE | TF_TOP },
	{ TI_GRASSTOP_1, TT_FREE | TF_TOP },
	{ TI_GRASSBOT_0, TT_FREE | TF_TOP },
	{ TI_GRASSBOT_1, TT_FREE | TF_TOP },
	{ TI_VOIDBLOCK, TT_BLOCK },
	{ TI_LAVAROCK_L, TT_FREE },
	{ TI_LAVAROCK_R, TT_FREE },
	{ TI_GRASS_L, TT_FREE },
	{ TI_GRASS_R, TT_FREE },
	{ TI_LAVA_0, TT_BLOCK | TT_LAVA | TF_TOP },
	{ TI_LAVA_1, TT_BLOCK | TT_LAVA | TF_TOP },
	{ TI_LAVA_2, TT_BLOCK | TT_LAVA | TF_TOP },
	{ TI_LAVA_3, TT_BLOCK | TT_LAVA | TF_TOP },
	{ TI_LAVA_4, TT_BLOCK | TT_LAVA | TF_TOP },
	{ TI_LAVA_5, TT_BLOCK | TT_LAVA | TF_TOP },
	{ TI_LAVA_6, TT_BLOCK | TT_LAVA | TF_TOP },
	{ TI_LAVA_7, TT_BLOCK | TT_LAVA | TF_TOP },
	{ TI_LAVAROCK_0, TT_BLOCK },
	{ TI_LAVAROCK_1, TT_BLOCK },
	{ TI_LAVAROCK_2, TT_BLOCK },
	{ TI_LAVAROCK_3, TT_BLOCK },
	{ TI_LAVAARC_L, TT_FREE },
	{ TI_LAVAARC_R, TT_FREE },
	{ TI_GRASSLAVA_L, TT_BLOCK },
	{ TI_GRASSLAVA_R, TT_BLOCK },
	{ TI_CAVERN_L, TT_FREE },
	{ TI_CAVERN_R, TT_FREE },
	{ TI_CAVERN_0, TT_BLOCK },
	{ TI_CAVERN_1, TT_BLOCK },
	{ TI_CAVERN_2, TT_BLOCK },
	{ TI_CAVERN_3, TT_BLOCK },
	{ TI_CAVERN_TOP_0, TT_FREE | TF_TOP },
	{ TI_CAVERN_TOP_1, TT_FREE | TF_TOP },
	{ TI_CAVERN_BOT_0, TT_FREE | TF_TOP },
	{ TI_CAVERN_BOT_1, TT_FREE | TF_TOP },
	{ TI_WATER_0, TT_FREE | TF_TOP },
	{ TI_WATER_1, TT_FREE | TF_TOP },
	{ TI_WATER_2, TT_FREE | TF_TOP },
	{ TI_WATER_3, TT_FREE | TF_TOP },
	{ TI_WATER_4, TT_FREE | TF_TOP },
	{ TI_WATER_5, TT_FREE | TF_TOP },
	{ TI_WATER_6, TT_FREE | TF_TOP },
	{ TI_WATER_7, TT_FREE | TF_TOP },
	{ TI_LAVAROCK_L, TT_FREE },
	{ TI_LAVAROCK_L_BLOCK, TT_BLOCK },
	{ TI_LAVAARC_R_BLOCK, TT_BLOCK },
};

TILESET(main, bmp_tileset, s_tileinfos)

BEGIN_TILEANIM(lava)
ANIM_TILEID(TI_LAVA_0)
ANIM_TILEID(TI_LAVA_1)
ANIM_TILEID(TI_LAVA_2)
ANIM_TILEID(TI_LAVA_3)
ANIM_TILEID(TI_LAVA_4)
ANIM_TILEID(TI_LAVA_5)
ANIM_TILEID(TI_LAVA_6)
ANIM_TILEID(TI_LAVA_7)
END_TILEANIM

DEFTILEANIM(lava)

BEGIN_TILEANIM(water)
ANIM_TILEID(TI_WATER_0)
ANIM_TILEID(TI_WATER_1)
ANIM_TILEID(TI_WATER_2)
ANIM_TILEID(TI_WATER_3)
ANIM_TILEID(TI_WATER_4)
ANIM_TILEID(TI_WATER_5)
ANIM_TILEID(TI_WATER_6)
ANIM_TILEID(TI_WATER_7)
END_TILEANIM

DEFTILEANIM(water)

static FRAME(planet, bmp_blocks, 0, 0, 48, 48)
static FRAME(star1, bmp_blocks, 48, 0, 16, 16)
static FRAME(stargate, bmp_stargate, 0, 0, 96, 96)
static FRAME(endscr, bmp_end, 0, 0, 256, 224)

static struct block s_blocks[] = {
	{ "planet", &fr_planet },
	{ "star1", &fr_star1 },
	{ "stargate", &fr_stargate },
	{ "endscr", &fr_endscr },
};

BLOCKSET(main, s_blocks)

void data_init(void)
{
	register_bitmap(&bmp_blocks, "blocks", 0, 0);
	register_bitmap(&bmp_tileset, "tileset", 1, 0xff00ff);
	register_bitmap(&bmp_stargate, "stargate", 1, 0x8000);
	if (PP_DEMO)
		return;
	register_bitmap(&bmp_end, "end", 0, 0);
}
