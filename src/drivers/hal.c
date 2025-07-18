#include "hal.h"
#include "ata.h"
#include "vga.h"

// Storage drivers
static storage_driver_t ata_driver = {
    .detect = detect_hdd,
    .read_sector = ata_read_sector,
    .write_sector = ata_write_sector,
    .name = "ATA/IDE"
};

// Display drivers
static display_driver_t vga_driver = {
    .putchar = putchar,
    .puts = prints,
    .clear = clear_screen,
    .set_cursor = NULL  // We could implement this
};

storage_driver_t *current_storage = NULL;
display_driver_t *current_display = &vga_driver;

void hal_init(void) {
    // Try to detect storage devices
    if(ata_driver.detect()) {
        current_storage = &ata_driver;
        prints("HAL: Found storage device: ");
        prints(current_storage->name);
        prints("\n");
    } else {
        prints("HAL: No storage devices found\n");
    }
}

int hal_storage_read(uint32_t lba, uint8_t *buf) {
    if(!current_storage) return 0;
    return current_storage->read_sector(lba, buf);
}

int hal_storage_write(uint32_t lba, const uint8_t *buf) {
    if(!current_storage) return 0;
    return current_storage->write_sector(lba, buf);
}
