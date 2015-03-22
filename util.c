#include "util.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

off_t get_file_size(FILE *f) {
    // TODO(emily): error checking
    fseeko(f, 0, SEEK_END);
    off_t size = ftello(f);
    // TODO(emily): error checking
    fseeko(f, 0, SEEK_SET);

    return size;
}

char *read_whole_file(const char *file_name) {
    FILE *f = fopen(file_name, "rb");
    if (!f) {
        return NULL;
    }

    size_t file_size = (size_t)get_file_size(f);

    char *file_data = malloc(file_size + 1);

    size_t read = fread(file_data, 1, file_size, f);
    if (read < file_size) {
        if (ferror(f) != 0) {
            fprintf(stderr, "Got error when trying to read all of '%s': %s",
                    file_name, strerror(errno));
        } else if (feof(f)) {
            fprintf(stderr, "Got EOF when trying to read all of '%s'",
                    file_name);
        } else {
            fprintf(stderr, "Got unknown error trying to read '%s'",
                    file_name);
        }
        fclose(f);
        free(file_data);
        return NULL;
    }

    // Zero out the last byte
    file_data[file_size] = '\0';

    if (fclose(f) == EOF) {
        free(file_data);
        return NULL;
    }

    return file_data;
}
