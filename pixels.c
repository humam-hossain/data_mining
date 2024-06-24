#include <stdio.h>
#include <stdint.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

uint32_t pixels[] = {
    0xFF0000FF,
    0xFF0000FF,
    0xFF0000FF,
    0xFF0000FF,
};

int main()
{
    stbi_write_png("assets/pixels.png", 2, 2, 4, pixels, 2*4);

    return 0;
}