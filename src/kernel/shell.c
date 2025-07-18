#include "shell.h"
#include "../lib/string.h"
#include "../lib/input.h"
#include "../drivers/vga.h"
#include "../drivers/cmos.h"
#include "../drivers/speaker.h"
#include "../fs/zadfs.h"
#include "../system/reboot.h"
#include "../lib/heap.h"

#define MAX_COMMANDS 32
static command_t command_table[MAX_COMMANDS];
static int command_count = 0;
static int current_cwd = 0;

// Simple delay function to replace pit_delay
static void simple_delay(int ms) {
    for(volatile int i = 0; i < ms * 1000; i++);
}

int shell_get_cwd(void) {
    return current_cwd;
}

void shell_set_cwd(int new_cwd) {
    current_cwd = new_cwd;
}

void shell_register_command(const char *name, void (*handler)(token_list_t*), const char *help) {
    if(command_count < MAX_COMMANDS) {
        command_table[command_count].name = name;
        command_table[command_count].handler = handler;
        command_table[command_count].help = help;
        command_count++;
    }
}

static void execute_command(token_list_t *tokens) {
    if(tokens->count == 0) return;
    
    for(int i = 0; i < command_count; i++) {
        if(strcmp(tokens->tokens[0], command_table[i].name) == 0) {
            command_table[i].handler(tokens);
            return;
        }
    }
    
    prints("Unknown command: ");
    prints(tokens->tokens[0]);
    prints("\nType 'help' for available commands.\n");
}

// ===== ALL THE COMMAND IMPLEMENTATIONS =====

void cmd_help(token_list_t *tokens) {
    prints("XylenOS Shell - Available Commands:\n");
    prints("===================================\n");
    
    for(int i = 0; i < command_count; i++) {
        prints("  ");
        prints(command_table[i].name);
        
        // Add some spacing for alignment
        int name_len = strlen(command_table[i].name);
        for(int j = name_len; j < 12; j++) {
            prints(" ");
        }
        
        prints("- ");
        prints(command_table[i].help);
        prints("\n");
    }
    prints("\nTip: Most commands support arguments. Try 'ls /path' or 'mkdir newfolder'\n");
}

void cmd_clear(token_list_t *tokens) {
    clear_screen();
}

void cmd_time(token_list_t *tokens) {
    prints("Current date and time: ");
    print_time();
}

void cmd_version(token_list_t *tokens) {
    prints("XylenOS Pre-Alpha 0.3.5 Public Testing Build\n");
    prints("Featuring improved shell, memory management, and audio support!\n");
    prints("Built with love by PS2Comrade and VeryEpicKebap <3\n");
}

void cmd_reboot(token_list_t *tokens) {
    prints("Rebooting system in 3 seconds...\n");
    simple_delay(1000);
    prints("3...\n");
    simple_delay(1000);
    prints("2...\n");
    simple_delay(1000);
    prints("1...\n");
    simple_delay(1000);
    do_reboot();
}

void cmd_ls(token_list_t *tokens) {
    const char *path = NULL;
    if(tokens->count > 1) {
        path = tokens->tokens[1];
    }
    zadfs_ls(path, current_cwd);
}

void cmd_mkdir(token_list_t *tokens) {
    if(tokens->count < 2) {
        prints("Usage: mkdir <directory_name>\n");
        prints("Example: mkdir documents\n");
        return;
    }
    
    zadfs_mkdir(tokens->tokens[1]);
    if(zadfs.hdd_mode) {
        zadfs_save_to_hdd();
    }
}

void cmd_cd(token_list_t *tokens) {
    const char *path = "/";
    if(tokens->count > 1) {
        path = tokens->tokens[1];
    }
    
    int new_idx;
    if(path[0] == '/') {
        // Absolute path
        new_idx = zadfs_find(path);
    } else {
        // Relative path - construct absolute path
        char abs_path[256];
        zadfs_get_cwd_path(current_cwd, abs_path);
        
        if(current_cwd != zadfs.root_idx) {
            strncat(abs_path, "/", 1);
        }
        strncat(abs_path, path, 200);
        
        new_idx = zadfs_find(abs_path);
    }
    
    if(new_idx == -1 || zadfs.entries[new_idx].type != ZADFS_DIR) {
        prints("cd: No such directory: ");
        prints(path);
        prints("\n");
        return;
    }
    
    current_cwd = new_idx;
}

void cmd_cat(token_list_t *tokens) {
    if(tokens->count < 2) {
        prints("Usage: cat <filename>\n");
        prints("Example: cat readme.txt\n");
        return;
    }
    
    zadfs_cat(tokens->tokens[1], current_cwd);
}

void cmd_touch(token_list_t *tokens) {
    if(tokens->count < 2) {
        prints("Usage: touch <filename> [content]\n");
        prints("Example: touch hello.txt Hello World!\n");
        return;
    }
    
    // Combine all tokens after filename as content
    char content[512] = "";
    for(int i = 2; i < tokens->count; i++) {
        if(i > 2) strncat(content, " ", 1);
        strncat(content, tokens->tokens[i], 400);
    }
    
    zadfs_create_file(tokens->tokens[1], content, current_cwd);
    if(zadfs.hdd_mode) {
        zadfs_save_to_hdd();
    }
}

void cmd_rm(token_list_t *tokens) {
    if(tokens->count < 2) {
        prints("Usage: rm <file_or_directory>\n");
        prints("Example: rm oldfile.txt\n");
        return;
    }
    
    zadfs_rm(tokens->tokens[1], current_cwd);
    if(zadfs.hdd_mode) {
        zadfs_save_to_hdd();
    }
}

void cmd_cp(token_list_t *tokens) {
    if(tokens->count < 3) {
        prints("Usage: cp <source> <destination>\n");
        prints("Example: cp file1.txt file2.txt\n");
        return;
    }
    
    zadfs_cp(tokens->tokens[1], tokens->tokens[2], current_cwd);
    if(zadfs.hdd_mode) {
        zadfs_save_to_hdd();
    }
}

void cmd_edit(token_list_t *tokens) {
    if(tokens->count < 2) {
        prints("Usage: edit <filename>\n");
        prints("Example: edit document.txt\n");
        prints("Note: Editor not available in this build\n");
        return;
    }
    
    prints("Text editor would open here for: ");
    prints(tokens->tokens[1]);
    prints("\n(Editor not implemented yet - use 'touch' to create files)\n");
}

void cmd_beep(token_list_t *tokens) {
    uint32_t frequency = NOTE_A4; // Default A4 note
    uint32_t duration = 500;      // Default half second
    
    if(tokens->count > 1) {
        // Simple frequency parsing
        if(strcmp(tokens->tokens[1], "low") == 0) frequency = NOTE_C4;
        else if(strcmp(tokens->tokens[1], "high") == 0) frequency = NOTE_C5;
        else if(strcmp(tokens->tokens[1], "mid") == 0) frequency = NOTE_G4;
    }
    
    if(tokens->count > 2) {
        // Simple duration parsing
        if(strcmp(tokens->tokens[2], "short") == 0) duration = 200;
        else if(strcmp(tokens->tokens[2], "long") == 0) duration = 1000;
    }
    
    prints("*BEEP*\n");
    speaker_beep(frequency, duration);
}

void cmd_play(token_list_t *tokens) {
    prints("Playing a little tune...\n");
    
    // Simple melody - Mary Had a Little Lamb
    uint32_t melody[] = {
        NOTE_E4, NOTE_D4, NOTE_C4, NOTE_D4, 
        NOTE_E4, NOTE_E4, NOTE_E4, 0,
        NOTE_D4, NOTE_D4, NOTE_D4, 0,
        NOTE_E4, NOTE_G4, NOTE_G4, 0
    };
    
    uint32_t durations[] = {
        400, 400, 400, 400,
        400, 400, 600, 200,
        400, 400, 600, 200,
        400, 400, 600, 200
    };
    
    speaker_play_melody(melody, durations, 16);
    prints("Tune finished!\n");
}

void cmd_heap(token_list_t *tokens) {
    prints("Memory heap status:\n");
    heap_dump();
}

void cmd_savefs(token_list_t *tokens) {
    if(!zadfs.hdd_mode) {
        prints("Error: Not in HDD mode. Use 'formatfs' first.\n");
        return;
    }
    
    zadfs_save_to_hdd();
    prints("Filesystem saved to HDD.\n");
}

void cmd_loadfs(token_list_t *tokens) {
    zadfs_load_from_hdd();
    prints("Filesystem loaded from HDD.\n");
}

// Initialize shell with all built-in commands
void shell_init(void) {
    current_cwd = zadfs.root_idx;
    command_count = 0;
    
    // Register all built-in commands
    shell_register_command("help", cmd_help, "Show this help message");
    shell_register_command("clear", cmd_clear, "Clear the screen");
    shell_register_command("time", cmd_time, "Show current date and time");
    shell_register_command("version", cmd_version, "Show OS version info");
    shell_register_command("reboot", cmd_reboot, "Restart the system");
    
    // File system commands
    shell_register_command("ls", cmd_ls, "List directory contents");
    shell_register_command("mkdir", cmd_mkdir, "Create a new directory");
    shell_register_command("cd", cmd_cd, "Change current directory");
    shell_register_command("cat", cmd_cat, "Display file contents");
    shell_register_command("touch", cmd_touch, "Create file with content");
    shell_register_command("rm", cmd_rm, "Remove file or directory");
    shell_register_command("cp", cmd_cp, "Copy file");
    shell_register_command("edit", cmd_edit, "Edit file (placeholder)");
    
    // Storage commands
    shell_register_command("savefs", cmd_savefs, "Save filesystem to HDD");
    shell_register_command("loadfs", cmd_loadfs, "Load filesystem from HDD");
    
    // Fun commands
    shell_register_command("beep", cmd_beep, "Make system beep");
    shell_register_command("play", cmd_play, "Play a musical tune");
    
    // System commands
    shell_register_command("heap", cmd_heap, "Show memory heap status");
    
    prints("Shell initialized with ");
    // Simple number printing
    char count_str[8];
    int num = command_count;
    int i = 0;
    do {
        count_str[i++] = '0' + (num % 10);
        num /= 10;
    } while(num > 0);
    
    for(int j = i-1; j >= 0; j--) {
        putchar(count_str[j]);
    }
    prints(" commands.\n");
}

void shell_run(void) {
    char input[MAX_INPUT_LINE];
    
    prints("Type 'help' to see available commands.\n");
    
    while(1) {
        // Show prompt with current directory
        char cwd_path[256];
        zadfs_get_cwd_path(current_cwd, cwd_path);
        prints(cwd_path);
        prints(" ~# ");
        
        // Get user input safely
        int len = safe_gets(input, MAX_INPUT_LINE);
        
        // Skip empty input
        if(len == 0) continue;
        
        // Parse input into tokens
        token_list_t tokens = tokenize(input);
        
        // Execute the command
        execute_command(&tokens);
    }
}
