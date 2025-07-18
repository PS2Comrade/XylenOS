#ifndef SHELL_H
#define SHELL_H

#include "../kernel/types.h"
#include "../lib/input.h"

typedef struct {
    const char *name;
    void (*handler)(token_list_t *tokens);
    const char *help;
} command_t;

void shell_init(void);
void shell_run(void);
void shell_register_command(const char *name, void (*handler)(token_list_t*), const char *help);

// Built-in commands
void cmd_help(token_list_t *tokens);
void cmd_clear(token_list_t *tokens);
void cmd_time(token_list_t *tokens);
void cmd_ls(token_list_t *tokens);
void cmd_cat(token_list_t *tokens);
void cmd_touch(token_list_t *tokens);
void cmd_mkdir(token_list_t *tokens);
void cmd_cd(token_list_t *tokens);
void cmd_rm(token_list_t *tokens);
void cmd_cp(token_list_t *tokens);
void cmd_reboot(token_list_t *tokens);
void cmd_version(token_list_t *tokens);
void cmd_edit(token_list_t *tokens);
void cmd_beep(token_list_t *tokens);
void cmd_play(token_list_t *tokens);
void cmd_heap(token_list_t *tokens);

#endif
