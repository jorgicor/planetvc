#ifndef READLIN_H
#define READLIN_H

#define READLIN_LINESZ 256

#ifndef STDIO_H
#define STDIO_H
#include <stdio.h>
#endif

int readlin(FILE *fp, char line[]);

#endif
