#include "types.h"
#include "../lib/string.h"
#include "../lib/heap.h"
#include "../lib/input.h"
#include "../drivers/keyboard.h"
#include "../drivers/vga.h"
#include "../drivers/cmos.h"
#include "../drivers/ata.h"
#include "../drivers/hal.h"
#include "../drivers/speaker.h"
#include "../system/reboot.h"
#include "../fs/zadfs.h"
#include "shell.h"
#include "errors.h"
#include <stdint.h>

void kernel_main() {
    // Initialize display
    clear_screen(); 
    enable_cursor();
    
    // Welcome message
    prints("==========================================\n");
    prints("    Welcome to XylenOS 0.3.5 Enhanced    \n");
    prints("==========================================\n");
    prints("Initializing system components...\n");
    
    // Initialize heap allocator
    prints("- Initializing memory heap...");
    heap_init();
    prints(" OK\n");
    
    // Initialize hardware abstraction layer
    prints("- Initializing hardware layer...");
    hal_init();
    prints(" OK\n");
    
    // Initialize speaker
    prints("- Initializing audio system...");
    speaker_init();
    prints(" OK\n");
    
    // Startup beep to show audio works
    speaker_beep(NOTE_C4, 100);
    speaker_beep(NOTE_E4, 100);
    speaker_beep(NOTE_G4, 200);
    
    // Initialize filesystem
    prints("- Initializing filesystem...");
    zadfs_init();
    prints(" OK\n");
    
    // Check for hard drive and handle filesystem setup
    int hdd_found = detect_hdd();
    
    if(hdd_found) {
        prints("\n=== STORAGE DETECTED ===\n");
        prints("Hard drive found! Choose filesystem mode:\n");
        prints("  'load'   - Load existing filesystem from HDD\n");
        prints("  'format' - Create new filesystem (DESTROYS DATA!)\n");
        prints("  'ram'    - Use RAM only (temporary)\n");
        prints("Choice: ");
        
        char choice[16];
        safe_gets(choice, 16);
        
        if(strcmp(choice, "load") == 0) {
            prints("Loading filesystem from HDD...");
            zadfs_load_from_hdd();
            zadfs.hdd_mode = 1;
            prints(" OK\n");
            prints("Filesystem loaded successfully!\n");
        } 
        else if(strcmp(choice, "format") == 0) {
            prints("Are you sure? This will destroy all data! (yes/no): ");
            char confirm[8];
            safe_gets(confirm, 8);
            
            if(strcmp(confirm, "yes") == 0) {
                prints("Formatting and creating new filesystem...");
                zadfs_init();
                zadfs.hdd_mode = 1;
                zadfs_save_to_hdd();
                prints(" OK\n");
                prints("New filesystem created and saved to HDD!\n");
            } else {
                prints("Format cancelled. Using RAM mode.\n");
                zadfs.hdd_mode = 0;
            }
        }
        else {
            prints("Using RAM-only mode.\n");
            zadfs.hdd_mode = 0;
        }
    } 
    else {
        prints("- No hard drive detected, using RAM-only mode.\n");
        zadfs.hdd_mode = 0;
    }
    
    // Create some default directories if this is a fresh filesystem
    if(zadfs.num_entries == 1) { // Only root directory exists
        prints("- Setting up default directories...");
        zadfs_mkdir("/bin");
        zadfs_mkdir("/home");
        zadfs_mkdir("/tmp");
        zadfs_mkdir("/docs");
        
        // Create a welcome file
        zadfs_create_file("/welcome.txt", 
            "Welcome to XylenOS!\n\n"
            "This is your new operating system. Here are some things to try:\n"
            "- Type 'help' to see all commands\n"
            "- Use 'edit filename.txt' to create and edit files\n"
            "- Try 'beep' and 'play' for some audio fun\n"
            "- Use 'ls', 'cd', 'mkdir' to navigate the filesystem\n"
            "\nHave fun exploring!", 
            zadfs.root_idx);
        
        if(zadfs.hdd_mode) {
            zadfs_save_to_hdd();
        }
        prints(" OK\n");
    }
    
    prints("\n=== SYSTEM READY ===\n");
    prints("Heap: 128KB allocated\n");
    if(zadfs.hdd_mode) {
        prints("Storage: HDD persistent mode\n");
    } else {
        prints("Storage: RAM temporary mode\n");
    }
    
    // Show some system stats
    prints("Files: ");
    char num_str[8];
    int num = zadfs.num_entries;
    int i = 0;
    do {
        num_str[i++] = '0' + (num % 10);
        num /= 10;
    } while(num > 0);
    for(int j = i-1; j >= 0; j--) putchar(num_str[j]);
    prints(" entries\n");
    
    // Initialize and start the shell
    prints("\n");
    shell_init();
    
    prints("\n");
    prints("Starting XylenOS shell...\n");
    prints("=========================\n");
    
    // Welcome sound
    speaker_beep(NOTE_C5, 150);
    speaker_beep(NOTE_E4, 150);
    
    // This never returns - shell runs forever
    shell_run();
}
