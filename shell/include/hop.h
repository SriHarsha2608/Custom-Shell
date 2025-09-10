#ifndef HOP_H
#define HOP_H

#include <limits.h>

extern char shell_home[PATH_MAX];
extern char prevDir[PATH_MAX];
extern int prevValid;

void initHop();
void doHop(int argc, char **argv);

#endif