#ifndef HAL_H
#define HAL_H

#include "../kernel/types.h"

typedef struct {
    int (*detect)(void);
    int (*read_sector)(uint32_t lba, uint8_t *buf);
    int (*write_sector)(uint32_t lba, const uint8_t *buf);
    const char *name;
} storage_driver_t;

typedef struct {
    void (*putchar)(char c);
    void (*puts)(const char *s);
    void (*clear)(void);
    void (*set_cursor)(int x, int y);
} display_driver_t;

extern storage_driver_t *current_storage;
extern display_driver_t *current_display;

void hal_init(void);
int hal_storage_read(uint32_t lba, uint8_t *buf);
int hal_storage_write(uint32_t lba, const uint8_t *buf);

#endif
