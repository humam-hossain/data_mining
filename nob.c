#define NOB_IMPLEMENTATION
#include "nob.h"

bool build_program(const char *source_path, const char *output_path){
    Nob_Cmd cmd = {0};

    nob_cmd_append(&cmd, "gcc");
    nob_cmd_append(&cmd, "-Wall", "-Wextra", "-ggdb");
    nob_cmd_append(&cmd, source_path);
    nob_cmd_append(&cmd, "-o", output_path);
    nob_cmd_append(&cmd, "-lraylib", "-lGL", "-lm", "-lpthread", "-ldl", "-lrt", "-lX11");

    return nob_cmd_run_sync(cmd);
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);
    // if(!build_program("2d.c", "2d")) return 1;
    // if(!build_program("3d.c", "3d")) return 1;
    if(!build_program("knn.c", "knn")) return 1;

    return 0;
}