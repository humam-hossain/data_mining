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

const char *klass_names[] = {"World", "Sports", "Business", "Sci/Tech"};

typedef struct {
    float distance;
    size_t klass;
} NCD;

typedef struct {
    NCD *items;
    size_t count;
    size_t capacity;
} NCDs;

float ncd(Nob_String_View a, Nob_String_View b, float cb)
{
    Nob_String_View ab = nob_sv_from_cstr(nob_temp_sprintf(SV_Fmt SV_Fmt, SV_Arg(a), SV_Arg(b)));
    float ca = deflate_sv(a).count;
    float cab = deflate_sv(ab).count;
    float min = ca; if(cb < min) min = cb;
    float max = ca; if(cb > max) max = cb;
    
    return (cab - min)/max;
}

int compare_ncds(const void *a, const void *b)
{
    NCD *na = a;
    NCD *nb = b;

    if(na->distance < nb->distance) return -1;
    if(na->distance > nb->distance) return 1;
    return 0;
}

size_t klassify_sample(Samples train, Nob_String_View text, size_t k)
{
    NCDs ncds = {0};

    float cb = deflate_sv(text).count;
    for(size_t i=0; i<train.count; ++i){
        float distance = ncd(train.items[i].text, text, cb);
        nob_temp_reset();

        NCD ncd = {
            .distance = distance,
            .klass = train.items[i].klass
        };
        nob_da_append(&ncds, ncd);
        printf("\rClassifying %zu/%zu", i, train.count);
    }
    printf("\n");

    qsort(ncds.items, ncds.count, sizeof(*ncds.items), compare_ncds);

    size_t klass_freq[NOB_ARRAY_LEN(klass_names)] = {0};
    for(size_t i = 0; i < ncds.count && i < k; ++i){
        klass_freq[ncds.items[i].klass] += 1;
    }

    size_t predicted_class = 0;
    for(size_t i=1; i<NOB_ARRAY_LEN(klass_names); ++i){
        if(klass_freq[i] > klass_freq[predicted_class]){
            predicted_class = i;
        }
    }

    return predicted_class;
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

    const char* text = "Operational athletics test weeks before start of Olympic Games in Paris. An operational athletics test was held Tuesday at the Stade de France, just a few weeks before the start of the Olympic Games in Paris. Young athletes from France and abroad gathered to test the facilities for the various athletics events that will take place at the stadium.";

    size_t predicted_class = klassify_sample(train_samples, nob_sv_from_cstr(text), 2);
    nob_log(NOB_INFO, "Predicted class: %s", klass_names[predicted_class]);

    // Sample sample = test_samples.items[0];
    // size_t predicted_class = klassify_sample(train_samples, sample.text, 2);

    // nob_log(NOB_INFO, "Text: "SV_Fmt, SV_Arg(sample.text));
    // nob_log(NOB_INFO, "Predicted class: %s", klass_names[predicted_class]);
    // nob_log(NOB_INFO, "Actual class: %s", klass_names[sample.klass]);

    return 0;
}