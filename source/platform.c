#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

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
    /* A refaire: utiliser fopen, fread */
    /* Charger le code dans la mémoire */
    int fp = open(file_name, O_RDONLY);

    if (fp == -1)
    {
        perror("Error while opening the bin file");
        exit(EXIT_FAILURE);
    }
    else
    {
        struct stat infos;
        int e = stat(file_name, &infos);

        if (e == -1)
        {
            perror("Error reading the length of the file");
            exit(EXIT_FAILURE);
        }

        /* the size of the file (in uint32) */
        int size = infos.st_size / 4;

        uint32_t *buffer;

        if ((buffer = malloc(size * sizeof(uint32_t))) == NULL)
        {
            perror("Malloc error");
            exit(EXIT_FAILURE);
        }

        if (read(fp, buffer, 4 * size) == -1)
        {
            perror("Error while reading the file");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < size; i += 1)
        {
            platform->memory[i] = buffer[i];
        }

        free(buffer);
    }
    close(fp);
}

int platform_read(struct platform_t *platform, enum access_type_t access_type, uint32_t addr, uint32_t *data)
{
    if (addr == CHAROUT_BASE || addr == CHAROUT_BASE + 4 || addr == CHAROUT_BASE + 8)
    {
        *data = 0x0;
        return 0;
    }

    /* Add: condition RAM */

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