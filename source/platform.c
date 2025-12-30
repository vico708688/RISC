#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "platform.h"

struct platform_t *platform_new()
{
    struct platform_t *plt;

    if ((plt = malloc(sizeof(struct platform_t))) == NULL)
    {
        printf("Malloc error.\n");
        exit(EXIT_FAILURE);
    }

    plt->size = 32 * 1024 * 1024;
    // regarder aligned_alloc ?
    if ((plt->memory = malloc(plt->size * sizeof(uint8_t))) == NULL)
    {
        printf("Malloc error.\n");
        exit(EXIT_FAILURE);
    }

    return plt;
}

void platform_free(struct platform_t *platform)
{
    free(platform->memory);
    free(platform);
}

void platform_load_program(struct platform_t *platform, const char *file_name)
{
    FILE *fp = fopen(file_name, "r");

    if (fp == NULL)
    {
        perror("Error while opening the bin file");
        exit(EXIT_FAILURE);
    }
    else
    {
        fread(platform->memory, 1, platform->size, fp);
    }
    fclose(fp);
}

int platform_read(struct platform_t *platform, enum access_type_t access_type, uint32_t addr, uint32_t *data)
{
    if (addr == CHAROUT_BASE || addr == CHAROUT_BASE + 4 || addr == CHAROUT_BASE + 8)
    {
        *data = 0x0;
        return 0;
    }

    /* Add: condition RAM */
    if ((addr < RAM_BASE) || (addr >= (RAM_BASE + platform->size)))
    {
        printf("Segmentation Fault\n");
        exit(1);
    }

    switch (access_type)
    {
    case ACCESS_BYTE:
    {
        uint8_t *p = (uint8_t *)platform->memory;
        *data = p[(addr - RAM_BASE)];
    }
    break;

    case ACCESS_HALF:
    {
        /* Alignement check */
        if (addr & 0x1)
        {
            printf("Load half address misaligned exception.\n");
            return -1;
        }
        uint16_t *p = (uint16_t *)platform->memory;
        *data = p[(addr - RAM_BASE) >> 1];
    }
    break;

    case ACCESS_WORD:
        /* Alignement check */
        if (addr & 0x3)
        {
            printf("Load word address misaligned exception.\n");
            return -1;
        }
        *data = platform->memory[(addr - RAM_BASE) >> 2];
        break;

    default:
        printf("Error reading access.\n");
        return -1;
        break;
    }

    return 0;
}

int platform_write(struct platform_t *platform, enum access_type_t access_type, uint32_t addr, uint32_t data)
{
    /* RAM */
    if ((addr >= RAM_BASE) && (addr < (RAM_BASE + platform->size)))
    {
        switch (access_type)
        {
        case ACCESS_BYTE:
        {
            uint8_t *p = (uint8_t *)platform->memory;
            p[(addr - RAM_BASE)] = data;
            break;
        }
        case ACCESS_HALF:
        {
            /* Alignement check */
            if (addr & 0x1)
            {
                printf("Write half address misaligned exception.\n");
                exit(1);
            }
            uint16_t *p = (uint16_t *)platform->memory;
            p[(addr - RAM_BASE) >> 1] = data;
            break;
        }
        case ACCESS_WORD:
        {
            /* Alignement check */
            if (addr & 0x3)
            {
                printf("Write word address misaligned exception.\n");
                exit(1);
            }
            platform->memory[(addr - RAM_BASE) >> 2] = data;
            break;
        }
        default:
            printf("Error reading access.\n");
            exit(1);
            break;
        }
    }
    else if (addr == CHAROUT_BASE)
    {
        printf("%c", (char)data);
    }
    else if (addr == CHAROUT_BASE + 4)
    {
        printf("%d", (int32_t)data);
    }
    else if (addr == CHAROUT_BASE + 8)
    {
        printf("0x%08x", data);
    }
    else
    {
        printf("Segmentation Fault.\n");
        exit(1);
    }

    return 0;
}