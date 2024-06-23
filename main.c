#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <time.h>
#include <float.h>

#define NOB_IMPLEMENTATION
#include "nob.h"

#define MIN_X -20.0
#define MAX_X 20.0
#define MIN_Y -20.0
#define MAX_Y 20.0

#define SAMPLE_RADIUS 2.5f
#define MEAN_RADIUS (2*SAMPLE_RADIUS)
#define K 5

static Vector2 project_sample_to_screen(Vector2 sample)
{
    float w = GetScreenWidth();
    float h = GetScreenHeight();

    float nx = (sample.x - MIN_X)/(MAX_X - MIN_X);
    float ny = (sample.y - MIN_Y)/(MAX_Y - MIN_Y);

    return CLITERAL(Vector2) {
        .x = w * nx,
        .y = h - h * ny
    };
}

typedef struct {
    Vector2 *items;
    size_t count;
    size_t capacity;
} Samples;

static inline float rand_float()
{
    return (float)rand()/RAND_MAX;
}

static void generate_cluster(Vector2 center, float radius, size_t count, Samples *samples)
{
    for(size_t i=0; i<count; ++i){
        float angle = rand_float()*2*PI;
        float mag = rand_float();

        Vector2 sample = {
            .x = center.x + cosf(angle)*mag*radius,
            .y = center.y + sinf(angle)*mag*radius
        };
        nob_da_append(samples, sample);
    }
}

static Samples set = {0};
static Samples clusters[K] = {0};
static Vector2 means[K] = {0};
static Color colors[] = {
    GOLD,
    PINK,
    LIME,
    SKYBLUE,
    VIOLET,
};
#define colors_count NOB_ARRAY_LEN(colors)

void generate_new_state()
{
    set.count = 0;
    generate_cluster(CLITERAL(Vector2){0}, 10, 100, &set);
    generate_cluster(CLITERAL(Vector2){MIN_X*0.5f, MAX_Y*0.5f}, 5, 50, &set);
    generate_cluster(CLITERAL(Vector2){MAX_X*0.5f, MAX_Y*0.5f}, 5, 50, &set);
    generate_cluster(CLITERAL(Vector2){MIN_X*0.5f, MIN_Y*0.5f}, 5, 50, &set);
    generate_cluster(CLITERAL(Vector2){MAX_X*0.5f, MIN_Y*0.5f}, 5, 50, &set);

    for(size_t i=0; i<K; ++i){
        means[i].x = rand_float()*(MAX_X - MIN_X) + MIN_X;
        means[i].y = rand_float()*(MAX_Y - MIN_Y) + MIN_Y;
    }
}

void recluster_state()
{
    for(size_t i=0; i<K; ++i){
        clusters[i].count = 0;
    }

    for(size_t i=0; i<set.count; ++i){
        Vector2 p = set.items[i];
        size_t k = -1;
        float s = FLT_MAX;
        for(size_t j=0; j<K; ++j){
            Vector2 m = means[j];
            float sm = Vector2LengthSqr(Vector2Subtract(p, m));
            if(sm < s){
                s = sm;
                k = j;
            }
        }
        nob_da_append(&clusters[k], p);
    }
}

void update_means()
{
    for(size_t i=0; i<K; ++i){
        if(clusters[i].count > 0){
            means[i] = Vector2Zero();
            for(size_t j=0; j<clusters[i].count; ++j){
                means[i] = Vector2Add(means[i], clusters[i].items[j]);
            }
            means[i].x /= clusters[i].count;
            means[i].y /= clusters[i].count;
        }else{
            means[i].x = rand_float()*(MAX_X - MIN_X) + MIN_X;
            means[i].y = rand_float()*(MAX_Y - MIN_Y) + MIN_Y;
        }
        
    }
}

int main()
{
    srand(time(0));
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "K-means");

    generate_new_state();
    recluster_state();    

    while(!WindowShouldClose()){
        if(IsKeyPressed(KEY_R)){
            generate_new_state();
            recluster_state();
        }

        if(IsKeyPressed(KEY_SPACE)){
            update_means();
            recluster_state();
        }

        BeginDrawing();
        ClearBackground(GetColor(0x181818AA));

        for(size_t i=0; i<K; ++i){
            Color color = colors[i%colors_count];
            for(size_t j=0; j<clusters[i].count; ++j){
                Vector2 it = clusters[i].items[j];
                DrawCircleV(project_sample_to_screen(it), SAMPLE_RADIUS, color);
            }
            DrawCircleV(project_sample_to_screen(means[i]), MEAN_RADIUS, colors[i%colors_count]);
        }

        EndDrawing();
    }
    CloseWindow();
    return 0;
}