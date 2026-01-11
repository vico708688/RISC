#include <stdio.h>

#include "platform.h"
#include "minirisc.h"
#include "types.h"

int main()
{
    struct platform_t *platform;
    struct minirisc_t *minirisc;

    printf("Creating platform...\n");
    platform = platform_new();

    printf("Creating minirisc...\n");
    minirisc = minirisc_new(RAM_BASE, platform);

    printf("Loading program...\n");
    platform_load_program(platform, "embedded_software/build/esw.bin");

    printf("Starting VM...\n");

    printf("First instruction at RAM_BASE: %08x\n", platform->memory[0]);
    printf("Stack top should be at: %08x\n", RAM_BASE + platform->size - 16);
    fflush(stdout);
    minirisc_run(minirisc);

    minirisc_free(minirisc);
    platform_free(platform);

    printf("\n---------------------\n\nVM STOPPED \n");

    return 0;
}