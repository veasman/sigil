#ifndef SIGIL_CLI_H
#define SIGIL_CLI_H

#include "sigil.h"

bool sigil_parse_cli(int argc, char **argv, SigilOptions *out);
void sigil_print_usage(FILE *stream, const char *argv0);

#endif
