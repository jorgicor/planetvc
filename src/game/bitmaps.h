#ifndef BITMAPS_H
#define BITMAPS_H

enum {
	E_BITMAPS_LOAD_OK,
	E_BITMAPS_LOAD_EOF,
	E_BITMAPS_LOAD_ERROR,
	E_BITMAPS_BIGFNAME,
};

struct bmp;

int load_next_bitmap(void);
const char *cur_bitmap_name(void);

void register_bitmap(struct bmp **ppbmp, const char *fname, int use_keycolor,
	             unsigned int keycolor);	

void bitmaps_done(void);

#endif
