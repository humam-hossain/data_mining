#include <stdio.h>

#define NOB_IMPLEMENTATION
#include "nob.h"

int main()
{
    const char *file_path = "data/train.csv";
    Nob_String_Builder sb = {0};
    if(!nob_read_entire_file(file_path, &sb)) return 1;

    Nob_String_View content = nob_sv_from_parts(sb.items, sb.count);
    size_t lines_count = 0;
    size_t klasses[4] = {0};
    while(content.count > 0){
        Nob_String_View line = nob_sv_chop_by_delim(&content, '\n');
        Nob_String_View klass = nob_sv_chop_by_delim(&line, ',');
        klasses[*klass.data - '0' - 1] += 1;
        lines_count += 1;
    }
    nob_log(NOB_INFO, "%s contains %zu points", file_path, lines_count);
    for(size_t i=0; i<NOB_ARRAY_LEN(klasses); ++i){
        nob_log(NOB_INFO, " %zu => %zu", i+1, klasses[i]);
    }

    return 0;
}