#ifndef STRDRAW_H
#define STRDRAW_H

enum {
	CHR_ARROW_R = 'm',
	CHR_ARROW_L = 'n',
};

extern struct bmp *bmp_font0;
extern struct bmp *bmp_font1;

int chri(int uc);
int utf8_next_code(const char *s, int *code);
int utf8_strlen(const char *s);
void draw_str(const char *s, int x, int y, int color);
void draw_str_ww(const char *s, int x, int y, int color, int len, int w);

void strdraw_init(void);

#endif
