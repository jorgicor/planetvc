#include "log.h"
#include "confpath.h"
#include "gamelib/vfs.h"
#include "kernel/kernel.h"
#include "cbase/kassert.h"
#include <string.h>
#include <time.h>

enum {
	MAX_LOG_SIZE = 1024 * 8
};

static FILE *s_fp;

static void ktrace_date(void)
{
	time_t curtime;
	char *strtime;
	char *p;
	char buf[256];

	curtime = time(NULL);
	strtime = ctime(&curtime);
	if (strtime != NULL) {
		snprintf(buf, sizeof(buf), "%s", strtime);
		for (p = buf; *p != '\0'; p++) {
			if (*p == '\r' || *p == '\n') {
				*p = '\0';
				break;
			}
		}
		ktrace(buf);
	}
}

/*
 * Gets the configuration path from the kernel and use it to store a file
 * log.txt that we will assign to kasssert to route all log.
 * In case of error, won't touch the current kassert asigned file.
 * The log file is rotated if needed.
 */
void log_init(void)
{
	FILE *fp;
	long fsize;
	const char *cpath;
	static const char *fname = "log.txt";
	char path[OPEN_FILE_MAX_PATH_LEN + 1];
	char oldpath[OPEN_FILE_MAX_PATH_LEN + 1];

	if (s_fp != NULL)
		return;

	cpath = confpath_get();
	if (cpath == NULL)
		return;

	if (strlen(cpath) + strlen(fname) > OPEN_FILE_MAX_PATH_LEN)
		return;

	strcpy(path, cpath);
	strcat(path, fname);
	strcpy(oldpath, cpath);
	strcat(oldpath, "log.old");	/* must have same size as log.txt */

	fsize = 0;
	if ((fp = fopen(path, "rb")) != NULL) {
		fseek(fp, 0, SEEK_END);
		fsize = ftell(fp);
		fclose(fp);
	}

	if (fsize > MAX_LOG_SIZE) {
		/* remove log.old if any */
		fprintf(stderr, "%s", oldpath);
		fprintf(stderr, "%s", path);
		remove(oldpath);
		rename(path, oldpath);
		fp = fopen(path, "wb");
	} else {
		fp = fopen(path, "ab");
	}

	if (fp == NULL)
		return;

	s_fp = fp;
	kassert_set_log_file(s_fp);
	ktrace_date();
}

void log_done(void)
{
	if (s_fp != NULL) {
		kassert_set_log_file(NULL);
		fclose(s_fp);
		s_fp = NULL;
	}
}
