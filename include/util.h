#ifndef SIGIL_UTIL_H
#define SIGIL_UTIL_H

#include "sigil.h"

void sigil_die(const char *msg);
void sigil_warn(const char *msg);
bool sigil_command_exists(const char *name);

#endif
