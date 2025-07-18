#include "editor.h"
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../lib/string.h"
#include "../fs/zadfs.h"

static editor_state_t editor;

static void editor_draw_screen() {
    clear_screen();
    
    // Title bar
    prints("=== XylenOS Text Editor === ");
    prints(editor.filename);
    prints(" ===\n");
    
    // Content
    for(int i = 0; i < editor.line_count && i < 20; i++) {
        if(i == editor.cursor_line) {
            prints("> ");
        } else {
            prints("  ");
        }
        prints(editor.lines[i]);
        prints("\n");
    }
    
    // Status bar
    prints("\n--- CTRL+S: Save, CTRL+Q: Quit, CTRL+N: New Line ---");
}

static void editor_insert_char(char c) {
    if(editor.cursor_col < EDITOR_MAX_LINE_LEN - 1) {
        // Shift characters right
        for(int i = strlen(editor.lines[editor.cursor_line]); i > editor.cursor_col; i--) {
            editor.lines[editor.cursor_line][i] = editor.lines[editor.cursor_line][i-1];
        }
        editor.lines[editor.cursor_line][editor.cursor_col] = c;
        editor.cursor_col++;
    }
}

static void editor_delete_char() {
    if(editor.cursor_col > 0) {
        editor.cursor_col--;
        // Shift characters left
        int len = strlen(editor.lines[editor.cursor_line]);
        for(int i = editor.cursor_col; i < len; i++) {
            editor.lines[editor.cursor_line][i] = editor.lines[editor.cursor_line][i+1];
        }
    }
}

static void editor_new_line() {
    if(editor.line_count < EDITOR_MAX_LINES - 1) {
        // Move lines down
        for(int i = editor.line_count; i > editor.cursor_line + 1; i--) {
            strcpy(editor.lines[i], editor.lines[i-1]);
        }
        
        // Split current line
        strcpy(editor.lines[editor.cursor_line + 1], 
               &editor.lines[editor.cursor_line][editor.cursor_col]);
        editor.lines[editor.cursor_line][editor.cursor_col] = '\0';
        
        editor.line_count++;
        editor.cursor_line++;
        editor.cursor_col = 0;
    }
}

static void editor_save() {
    // Combine all lines into one string
    char content[EDITOR_MAX_LINES * EDITOR_MAX_LINE_LEN];
    content[0] = '\0';
    
    for(int i = 0; i < editor.line_count; i++) {
        strncat(content, editor.lines[i], EDITOR_MAX_LINE_LEN);
        if(i < editor.line_count - 1) {
            strncat(content, "\n", 1);
        }
    }
    
    zadfs_create_file(editor.filename, content, zadfs.root_idx);
    if(zadfs.hdd_mode) zadfs_save_to_hdd();
}

void editor_run(const char *filename) {
    // Initialize editor state
    editor.line_count = 1;
    editor.cursor_line = 0;
    editor.cursor_col = 0;
    strcpy(editor.filename, filename);
    
    // Clear all lines
    for(int i = 0; i < EDITOR_MAX_LINES; i++) {
        editor.lines[i][0] = '\0';
    }
    
    // Try to load existing file
    int file_idx = zadfs_find(filename);
    if(file_idx != -1 && zadfs.entries[file_idx].type == ZADFS_FILE) {
        zadfs_entry_t *entry = &zadfs.entries[file_idx];
        
        // Parse file content into lines
        int line = 0, col = 0;
        for(int i = 0; i < entry->size && line < EDITOR_MAX_LINES; i++) {
            char c = zadfs.data[entry->data_offset + i];
            if(c == '\n') {
                editor.lines[line][col] = '\0';
                line++;
                col = 0;
            } else if(col < EDITOR_MAX_LINE_LEN - 1) {
                editor.lines[line][col++] = c;
            }
        }
        editor.line_count = line + 1;
    }
    
    while(1) {
        editor_draw_screen();
        
        char c = get_key();
        
        if(c == 19) {  // CTRL+S
            editor_save();
            prints("\nFile saved!");
            // Replace pit_delay(1000) with:
            for(volatile int i = 0; i < 2000000; i++); // Simple delay loop
        }
        else if(c == 17) {  // CTRL+Q
            break;
        }
        else if(c == 14) {  // CTRL+N
            editor_new_line();
        }
        else if(c == '\b') {
            editor_delete_char();
        }
        else if(c >= 32 && c <= 126) {  // Printable characters
            editor_insert_char(c);
        }
    }
}
