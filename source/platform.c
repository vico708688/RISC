#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "types.h"
#include "platform.h"

/**
 * plateform: comprend une mémoire, un périphérique “CharOut” ainsi qu’un bus d’interconnexion
 */

struct platform_t *platform_new()
{
    struct platform_t *plt;

    if ((plt = malloc(sizeof(struct platform_t))) == NULL)
    {
        printf("Malloc error.\n");
        return NULL;
    }

    plt->size = 32 * 1024 * 1024;
    // regarder aligned_alloc ?
    if ((plt->memory = malloc(plt->size * sizeof(uint8_t))) == NULL)
    {
        printf("Malloc error.\n");
        return NULL;
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
    int fd = open(file_name, O_RDONLY);

    if (fd == -1)
    {
        perror("Error while opening the bin file");
        return NULL;
    }
    else
    {
        struct stat infos;
        int e = stat(file_name, &infos);

        if (e == -1)
        {
            perror("Error reading the length of the file");
            return NULL;
        }

        /* the size of the file (in uint32) */
        int size = infos.st_size / 4;

        uint32_t *buffer;

        if ((buffer = malloc(size * sizeof(uint32_t))) == NULL)
        {
            perror("Malloc error");
            return NULL;
        }

        if (read(fd, buffer, 4 * size) == -1)
        {
            perror("Error while reading the file");
            return NULL;
        }
        for (int i = 0; i < size; i += 1)
        {
            platform->memory[i] = buffer[i];
        }

        // save_to_memory(platform, buffer, size);

        free(buffer);
    }
    close(fd);
}

void save_to_memory(struct platform_t *platform, uint32_t *code, int size)
{
    for (int i = 0; i < size; i += 1)
    {
        /* convert big endian to little-endian */
        uint32_t little_endian = __builtin_bswap32(code[i]);

        platform->memory[i] = little_endian; // remplacer par platform_write
        printf("Byte: %d :\n\t - little endian : %08X\n\t - big endian    : %08X\n", i, little_endian, code[i]);
    }
}

int platform_read(struct platform_t *platform, enum access_type_t access_type, uint32_t addr, uint32_t *data)
{
}

int platform_write(struct platform_t *platform, enum access_type_t access_type, uint32_t addr, uint32_t data)
{
    /* RAM */
    if (addr >= RAM_BASE && addr < RAM_BASE + platform->size)
    {
        if (addr & 0x3)
        {
            printf("Error: the target address is not aligned.\n");
            return -1;
        }

        uint32_t index = (addr - RAM_BASE) >> 2;

        platform->memory[index] = data;
    }
    /* Char out */
    else if (addr == 0x10000000)
    {
        putchar(data);
        return 0;
    }
    else if (addr == 0x10000004)
    {
        printf("%d", data);
        return 0;
    }
    else if (addr == 0x10000008)
    {
        printf("%x", data);
        return 0;
    }
    else
    {
        printf("Illegal address.\n");
        return -1;
    }
}