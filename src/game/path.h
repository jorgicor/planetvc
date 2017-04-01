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

#ifndef PATH_H
#define PATH_H

enum {
	PATH_NODIR,
	PATH_RDIR,
	PATH_LDIR
};

/*
 * flags: or'ed PATH_INFO_ .
 * x, y: calculated point at this moment.
 * dir: calcualted hint direction PATH_LDIR, PATH_RDIR.
 * path_id: id of the path.
 * path_point: index of the point on the path from where we started.
 * speed: points to advance each call to update_path_info.
 */
struct path_info {
	int x, y;
	int dir;
	int flags;
	int path_id;
	int path_point;
	int speed;
};

void init_path_info(struct path_info *pinfo, int path_id, int path_point,
                    int speed, int loop);
void update_path_info(struct path_info *pinfo);
int is_path_info_active(struct path_info *pinfo);

int new_path(int px, int py);
int num_paths(void);
void free_path(int i);
void free_paths(void);
void add_path_point(int i, int px, int py);
int num_path_points(int i);
void get_path_point(int i, int point_num, int *px, int *py);

int line(int *rx, int *ry, int x0, int y0, int x1, int y1, int npoints);

#endif
