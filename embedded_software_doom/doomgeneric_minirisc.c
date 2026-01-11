#include "doomgeneric/doomgeneric.h"
#include <stdint.h>
#include <sys/time.h>

int main(void)
{
    doomgeneric_Create(0, 0);
    while (1)
    {
        doomgeneric_Tick();
    }
}

void DG_Init(void)
{
    *(char *)0x10000000 = 'I';
    *(char *)0x10000000 = 'N';
    *(char *)0x10000000 = 'I';
    *(char *)0x10000000 = 'T';
    *(char *)0x10000000 = '\n';
}

void DG_DrawFrame(void)
{
    *(char *)0x10000000 = 'D';

    static int frame = 0;
    if (++frame % 100 == 0)
    {
        *(char *)0x10000000 = 'F';
    }
}

void DG_SleepMs(uint32_t ms)
{
    uint32_t start = DG_GetTicksMs();
    while (DG_GetTicksMs() - start < ms)
    {
        __asm volatile("nop");
    }
}

uint32_t DG_GetTicksMs(void)
{
    *(char *)0x10000000 = 'T';

    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0)
    {
        static uint32_t dt = 0;
        dt += 16; // 60 FPS
        return dt;
    }
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int DG_GetKey(int *pressed, unsigned char *key)
{
    return 0; // aucun clavier (pour l'instant)
}

void DG_SetWindowTitle(const char *title)
{
}
