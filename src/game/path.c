/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "path.h"
#include "cbase/cbase.h"
#include "cbase/kassert.h"

/*
 * PATH_INFO_FORWARD: we are going toward to_point from a previous
 *                    from a previous point or from a next point.
 * PATH_INFO_LOOP: if we loop.
 */
enum {
	PATH_INFO_FORWARD = 1,
	PATH_INFO_LOOP = 2
};

enum {
	NPATHS = 8,
	NPATH_POINTS = 64
};

struct path_point {
	int x, y;
};

struct path {
	int npoints;	/* 0 unasigned, > 0 asigned and number of points */
	struct path_point points[NPATH_POINTS];
};

struct path s_paths[NPATHS];
static int s_npaths;

/*
 * Creates a new path with the point px, py as the first point.
 * Returns -1 if no free paths.
 */
int new_path(int px, int py)
{
	int i;

	for (i = 0; i < NPATHS; i++) {
		if (s_paths[i].npoints == 0) {
			add_path_point(i, px, py);
			s_npaths++;
			if (kassert_fails(s_npaths <= NPATHS))
				s_npaths = NPATHS;
			return i;
		}
	}
	return -1;
}

/* Returns the number of paths. */
int num_paths(void)
{
	return s_npaths;
}

void free_path(int i)
{
	if (kassert_fails(i >= 0 && i < NPATHS))
		return;
	if (s_paths[i].npoints > 0) {
		s_paths[i].npoints = 0;
		s_npaths--;
	}
}

void free_paths(void)
{
	int i;

	for (i = 0; i < NPATHS; i++) {
		free_path(i);
	}

	if (kassert_fails(s_npaths == 0))
		s_npaths = 0;
}

void add_path_point(int i, int px, int py)
{
	struct path *p;
	struct path_point *pp;

	if (kassert_fails(i >= 0 && i < NPATHS))
		return;

	p = &s_paths[i];
	if (kassert_fails(p->npoints >= 0 && p->npoints < NPATH_POINTS - 1))
		return;

	pp = &p->points[p->npoints];	
	pp->x = px;
	pp->y = py;
	p->npoints++;
}

int num_path_points(int i)
{
	if (kassert_fails(i >= 0 && i < NPATHS))
		return 0;
	if (kassert_fails(s_paths[i].npoints > 0))
		return 0;
	return s_paths[i].npoints;
}

void get_path_point(int i, int point_num, int *px, int *py)
{
	struct path *p;
	struct path_point *pp;

	if (kassert_fails(i >= 0 && i < NPATHS))
		return;
	if (kassert_fails(point_num >= 0))
		return;
	p = &s_paths[i];
	if (kassert_fails(p->npoints > 0 && point_num < p->npoints))
		return;
	pp = &p->points[point_num];
	*px = pp->x;
	*py = pp->y;
}

/*
 * Given a line segment from x0,y0 to x1,y1 returns in rx,ry the point
 * in the line after advance npoints from x0,y0.
 * Returns the actual number of points advanced.
 */
int line(int *rx, int *ry, int x0, int y0, int x1, int y1, int npoints)
{
	int d1x, d1y, d2x, d2y, m, n;

	if (kassert_fails(npoints >= 0))
		npoints = 0;

	x1 -= x0;
	y1 -= y0;
	d1x = isign(x1);
	d1y = isign(y1);
	d2x = d1x;
	d2y = 0;
	m = iabs(x1);
	n = iabs(y1);
	*rx = x0;
	*ry = y0;
	
	if (m <= n) {
		d2x = 0;
		d2y = d1y;
		m = n;
		n = iabs(x1);
	}
	
	// limita a npoints puntos
	if (m < npoints) {
		npoints = m;
	}
	
	x1 = m >> 1;

#if 1
	x1 += n * npoints;
	if (m == 0)
		y1 = 0;
	else
		y1 = x1 / m;
	x1 = npoints - y1;
	*rx = x0 + d2x * x1 + d1x * y1;
	*ry = y0 + d2y * x1 + d1y * y1;
#endif

#if 0
	for (y1 = 0; y1 < npoints; y1++) {
		*rx = x0;
		*ry = y0;
		
		x1 += n;
		if (x1 >= m) {
			x1 -= m;
			x0 += d1x;
			y0 += d1y;
		} else {
			x0 += d2x;
			y0 += d2y;
		}
	}
#endif

	return npoints;	
}

void init_path_info(struct path_info *pinfo, int path_id, int path_point,
                    int speed, int loop)
{
	if (kassert_fails(path_id >= 0 && path_id < num_paths()))
		return;
	if (kassert_fails(num_path_points(path_id) > 0))
		return;
	pinfo->path_id = path_id;
	path_point %= num_path_points(path_id);
	pinfo->path_point = path_point;
	pinfo->flags = PATH_INFO_FORWARD;
	pinfo->speed = speed;
	pinfo->dir = 0;
	get_path_point(path_id, path_point, &pinfo->x, &pinfo->y);
	if (loop) {
		pinfo->flags |= PATH_INFO_LOOP;
	}
}

/*
 * Returns the next point num on a path if we are at 'point' number
 * and we go forward (not 0) or not (0).
 */
static int next_path_point(struct path *ppath, int point, int forward)
{
	int a, p;

	if (kassert_fails(ppath->npoints > 0))
		return 0;

	if (forward) {
		a = ppath->npoints - 1;
		p = 1;
	} else {
		a = 0;
		p = -1;
	}

	if (point != a) {
		return point + p;
	}

	if (ppath->points[0].x == ppath->points[ppath->npoints - 1].x &&
		ppath->points[0].y == ppath->points[ppath->npoints - 1].y)
	{
		/* it's a loop */
		return 0;
	}

	/* reverse */
	if (ppath->npoints > 1) {
		return point - p;
	} else {
		return point; 
	}
}

int is_path_info_active(struct path_info *pinfo)
{
	return pinfo->speed > 0;
}

void update_path_info(struct path_info *pinfo)
{
	int nextp, rx, ry, n;
	struct path *ppath;

	if (!is_path_info_active(pinfo)) {
		return;
	}

	ppath = &s_paths[pinfo->path_id];
	nextp = next_path_point(ppath, pinfo->path_point,
		pinfo->flags & PATH_INFO_FORWARD);
	n = line(&rx, &ry, pinfo->x, pinfo->y, ppath->points[nextp].x,
		ppath->points[nextp].y, pinfo->speed);
	if (n < pinfo->speed) {
		pinfo->path_point = nextp;
		nextp = next_path_point(ppath, nextp,
			pinfo->flags & PATH_INFO_FORWARD);
		if (nextp == 0 && pinfo->path_point > 1) {
			/* circular path */
			if (!(pinfo->flags & PATH_INFO_LOOP)) {
				pinfo->speed = 0;
			}
		} else if (nextp < pinfo->path_point) {
			/* reached the end */
			if (!(pinfo->flags & PATH_INFO_LOOP) &&
				(pinfo->flags & PATH_INFO_FORWARD))
		       	{
				/* if no loop, stop */
				pinfo->speed = 0;
			}
			/* change backwards */
			pinfo->flags &= ~PATH_INFO_FORWARD;
		} else if (nextp > 0 && pinfo->path_point == 0) {
			/* reached the end */
			if (!(pinfo->flags & PATH_INFO_LOOP) &&
				!(pinfo->flags & PATH_INFO_FORWARD))
			{
				/* if no loop, stop */
				pinfo->speed = 0;
			}
			/* change forward */
			pinfo->flags |= PATH_INFO_FORWARD;
		} else {
			pinfo->flags |= PATH_INFO_FORWARD;
		}
	}

	n = rx - pinfo->x;
	if (n != 0) {
		pinfo->dir = (n > 0) ? PATH_RDIR : PATH_LDIR;
	}

	pinfo->x = rx;
	pinfo->y = ry;
}
