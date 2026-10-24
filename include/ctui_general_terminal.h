#ifndef CTUI_GENERAL_TERMINAL_H
#define CTUI_GENERAL_TERMINAL_H

#include <termios.h>
#include <signal.h>
#include <pthread.h>

#include <dynamic_array.h>

#define CTRL_KEY(k) ((k) & 0x1f)

struct ctui_editor_state
{
  int columns;
  int rows;
};
struct ctui_thread_io_buffers
{
  char *write_buffer;
  char *read_buffer;
};
extern volatile sig_atomic_t ctui_sigwinch_catch;
void ctui_sigwinch_handler(int sig);
int ctui_init(void);

void ctui_termois_raw_mode_enable(struct termios *termios);
int ctui_termios_exit(struct termios *original_termios);
int ctui_get_cursor_position(int *columns, int *rows);
int ctui_get_window_size(int *columns, int *rows);
int ctui_get_input_buffer_size(int *buffer_size);
/*
struct ctui_thread_char_read_shared_data
{
  pthread_mutex_t data_mutex;
  char *data;
  int checks;
};
void *ctui_thread_char_read(void *args);
*/

int ctui_write_buffer_append_string(char *write_buffer, char *string, sds_da_index_t string_size);
int ctui_editor_refresh_screen(struct ctui_editor_state *state, char *write_buffer);
int ctui_editor_read_key(char *c);
int ctui_editor_proccess_key(struct ctui_editor_state *state);
int ctui_editor_state_init(struct ctui_editor_state *state);

#endif