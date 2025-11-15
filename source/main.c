#include <stdio.h>

#include "platform.h"
#include "minirisc.h"
#include "types.h"

int main()
{
    struct platform_t *platform;
    struct minirisc_t *minirisc;

    platform = platform_new();
    minirisc = minirisc_new(RAM_BASE, platform);

    platform_load_program(platform, "embedded_software/test_1/build/esw.bin");
    minirisc_run(minirisc);

    minirisc_free(minirisc);
    platform_free(platform);

    printf("\n\n---------------------\n\nEND OF PROGRAM \n");

    return 0;
}