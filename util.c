#include "util.h"

#include <stdlib.h>

off_t get_file_size(FILE *f) {
    // TODO(emily): error checking
    fseeko(f, 0, SEEK_END);
    off_t size = ftello(f);
    // TODO(emily): error checking
    fseeko(f, 0, SEEK_SET);

    return size;
}

char *read_whole_file(const char *file_name) {
    // TODO(emily): error checking
    FILE *f = fopen(file_name, "rb");
    off_t file_size = get_file_size(f);

    // TODO(emily): error checking
    char *file_data = malloc(file_size);
    // TODO(emily): error checking
    fread(file_data, 1, file_size, f);

    fclose(f);

    return file_data;
}
