#ifndef H_PLATFORM
#define H_PLATFORM

#include <inttypes.h>

struct platform_t
{
    int size;
    uint32_t *memory;
};

/**
 * Type of memory acess
 */
enum access_type_t
{
    ACCESS_BYTE = 0, //  8 bits
    ACCESS_HALF = 1, // 16 bits
    ACCESS_WORD = 2  // 32 bits
};

/**
 * Allocates and initializes a new platform and its memory.
 */
struct platform_t *platform_new();

/**
 * Cleanup the platform's allocated memories.
 */
void platform_free(struct platform_t *platform);

/**
 * Read one item from the platform.
 * @param platform The platform object
 * @param type     Width of the access
 * @param addr     Address where to read the item
 * @param data     The item is placed in data.
 * @return         0 on success, -1 on error (illegal or misaligned access)
 */
int platform_read(struct platform_t *plt, enum access_type_t access_type, uint32_t addr, uint32_t *data);

/**
 * Write one item to the platform.
 * @param platform The platform object
 * @param type     Width of the access
 * @param addr     Address where to write the item
 * @param data     The item to write
 * @return         0 on success, -1 on error (illegal or misaligned access)
 */
int platform_write(struct platform_t *plt, enum access_type_t access_type, uint32_t addr, uint32_t data);

/**
 * Read the file named file_name and write its content
 * in the platform's memory.
 */
void platform_load_program(struct platform_t *platform, const char *file_name);

#endif