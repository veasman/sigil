#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void sigil_die(const char *msg) {
    if (msg) {
        fprintf(stderr, "sigil: %s\n", msg);
    }
    exit(1);
}

void sigil_warn(const char *msg) {
    if (msg) {
        fprintf(stderr, "sigil: %s\n", msg);
    }
}

bool sigil_command_exists(const char *name) {
    if (!name || !*name) {
        return false;
    }

    const char *path = getenv("PATH");
    if (!path) {
        return false;
    }

    char *copy = strdup(path);
    if (!copy) {
        return false;
    }

    bool found = false;
    char *saveptr = NULL;

    for (char *dir = strtok_r(copy, ":", &saveptr);
         dir;
         dir = strtok_r(NULL, ":", &saveptr)) {
        char buf[4096];
        snprintf(buf, sizeof(buf), "%s/%s", dir, name);
        if (access(buf, X_OK) == 0) {
            found = true;
            break;
        }
    }

    free(copy);
    return found;
}
