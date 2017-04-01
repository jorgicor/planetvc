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
int str_height_ww(const char *s, int x, int w);
void draw_icon(const char *s, int x, int y, int color);

void strdraw_init(void);

#endif
