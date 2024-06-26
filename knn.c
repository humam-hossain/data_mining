#include <stdio.h>
#include <zlib.h>

#define NOB_IMPLEMENTATION
#include "nob.h"

Nob_String_View deflate_sv(Nob_String_View sv)
{
    void *output = nob_temp_alloc(sv.count*2);

    z_stream defstream = {0};
    defstream.avail_in = (uInt)sv.count;
    defstream.next_in = (Bytef *)sv.data;
    defstream.avail_out = (uInt)sv.count*2;
    defstream.next_out = (Bytef *)output;

    deflateInit(&defstream, Z_BEST_COMPRESSION);
    deflate(&defstream, Z_FINISH);
    deflateEnd(&defstream);

    return nob_sv_from_parts(output, defstream.total_out);
}

typedef struct {
    size_t klass;
    Nob_String_View text;
} Sample;

typedef struct {
    Sample *items;
    size_t count;
    size_t capacity;
} Samples;

Samples parse_samples(Nob_String_View content)
{
    size_t klasses[4] = {0};
    size_t lines_count = 0;
    Samples samples = {0};
    for(; content.count > 0; ++lines_count){
        Nob_String_View line = nob_sv_chop_by_delim(&content, '\n');
        if(lines_count == 0) continue;

        Nob_String_View klass = nob_sv_chop_by_delim(&line, ',');
        size_t klass_index = *klass.data - '0' - 1;
        Sample sample = {
            .klass = klass_index,
            .text = line,
        };
        nob_da_append(&samples, sample);
    }

    return samples;
}

const char *klass_names[] = {"Class", "Title", "Description"};

typedef struct {
    float distance;
    size_t klass;
} NCD;

typedef struct {
    NCD *items;
    size_t count;
    size_t capacity;
} NCDs;

size_t klassify_sample(Samples train, Nob_String_View text)
{
    assert(0 && "TODO: not implemented");

    return 0;
}

int main(int argc, char **argv)
{
    const char *program = nob_shift_args(&argc, &argv);

    if(argc <= 0){
        nob_log(NOB_ERROR, "Usage: %s <input.csv> <test.csv>", program);
        nob_log(NOB_ERROR, "ERROR: no train dataset is provided");
        return 1;
    }

    const char *train_path = nob_shift_args(&argc, &argv);
    Nob_String_Builder train_content = {0};
    if(!nob_read_entire_file(train_path, &train_content)) return 1;
    Samples train_samples = parse_samples(nob_sv_from_parts(train_content.items, train_content.count));

    if(argc <= 0){
        nob_log(NOB_ERROR, "Usage: %s <input.csv> <test.csv>", program);
        nob_log(NOB_ERROR, "ERROR: no test dataset is provided");
        return 1;
    }

    const char *test_path = nob_shift_args(&argc, &argv);
    Nob_String_Builder test_content = {0};
    if(!nob_read_entire_file(test_path, &test_content)) return 1;
    Samples test_samples = parse_samples(nob_sv_from_parts(test_content.items, test_content.count));

    Sample sample = test_samples.items[0];
    size_t predicted_class = klassify_sample(train_samples, sample.text);

    nob_log(NOB_INFO, "Text: "SV_Fmt, SV_Arg(sample.text));
    nob_log(NOB_INFO, "Predicted class: %s"SV_Fmt, klass_names[predicted_class]);
    nob_log(NOB_INFO, "Actual class: "SV_Fmt, klass_names[sample.klass]);


    return 0;
}