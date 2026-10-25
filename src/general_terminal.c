#include <ctui_general_terminal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <pthread.h>

#include <dynamic_array.h>
// terminal size
int ctui_terminal_size_init(struct ctui_terminal_size *terminal_size)
{
  if (ctui_get_window_size(&terminal_size->columns, &terminal_size->rows))
  {
    return 1;
  }
  return 0;
}
// sigwinch
volatile sig_atomic_t ctui_sigwinch_catch;
void ctui_sigwinch_handler(int sig)
{
  (void)sig;
  ctui_sigwinch_catch = 1;
}
int ctui_sigwinch_init(void)
{
  ctui_sigwinch_catch = 0;
  struct sigaction sa;
  sa.sa_handler = ctui_sigwinch_handler;
  if (sigemptyset(&(sa.sa_mask)))
  {
    return 1;
  }
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGWINCH, &sa, NULL) == -1)
  {
    return 1;
  }
  return 0;
}
int ctui_termios_exit(struct termios *original_termios)
{
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, original_termios) == -1)
  {
    return 1;
  }
  return 0;
}
void ctui_termois_raw_mode_enable(struct termios *termios)
{
  termios->c_iflag &= ~(BRKINT | IXON | ICRNL | INPCK | ISTRIP);
  termios->c_oflag &= ~(OPOST);
  termios->c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
  termios->c_cflag |= (CS8);
  termios->c_cc[VMIN] = 0;
  termios->c_cc[VTIME] = 1;
  return;
}
int ctui_get_cursor_position(int *columns, int *rows)
{
  char buffer[32];
  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
  {
    return 1;
  }

  {
    size_t i;
    for (i = 0; (i < sizeof(buffer) - 1); i++)
    {
      if (read(STDIN_FILENO, &buffer[i], 1) != 1)
      {
        break;
      }
      if (buffer[i] == 'R')
      {
        break;
      }
    }
    buffer[i] = '\0';
  }
  if (buffer[0] != '\x1b' || buffer[1] != '[')
  {
    return 1;
  }
  if (sscanf(&buffer[2], "%d;%d", rows, columns) != 2)
  {
    return 1;
  }
  return 0;
}
int ctui_get_window_size(int *columns, int *rows)
{
  struct winsize window_size;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &window_size) == -1 || window_size.ws_col == 0)
  {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
    {
      return 1;
    }
    return ctui_get_cursor_position(columns, rows);
  }
  (*columns) = window_size.ws_col;
  (*rows) = window_size.ws_row;
  return 0;
}
int ctui_get_input_buffer_size(int *buffer_size)
{
  if (ioctl(STDIN_FILENO, FIONREAD, buffer_size))
  {
    return 1;
  }
  return 0;
}

/*
void *ctui_thread_char_read(void *args)
{
  sds_da_create(&((struct ctui_thread_char_read_shared_data *)args)->data, sizeof(char), 4);
  int read_output;

  while ((read_output = read(STDIN_FILENO, c, 1)) != 1)
  {
    if ((read_output == -1) && (errno != EAGAIN))
    {
      return 1;
    }
  }
  return 0;
}
*/











int ctui_write_buffer_append_string(char **write_buffer, char *string, sds_da_index_t string_size)
{
  if (sds_da_add(write_buffer, string_size, sizeof(char)))
  {
    return 1;
  }
  memcpy(((*write_buffer) + sds_da_get_size(*write_buffer) - string_size), string, string_size);
  return 0;
}
int ctui_editor_refresh_screen(struct ctui_terminal_size *terminal_size, char **write_buffer)
{
  // hide cursor, clear screen, move cursor to (1, 1)
  ctui_write_buffer_append_string(write_buffer, "\x1b[?25l\x1b[3J\x1b[H", 17);
  // draw rows
  for (int i = 0; i < terminal_size->rows / 2; i++)
  {
    ctui_write_buffer_append_string(write_buffer, "\u2211\x1b[K", 6);
    
    if (i < terminal_size->rows - 1)
    {
      ctui_write_buffer_append_string(write_buffer, "\r\n", 2);
    }
  }
  // move cursor to (1, 1), unhide cursor
  ctui_write_buffer_append_string(write_buffer, "\x1b[H\x1b[?25h", 9);

  return 0;
}
int ctui_editor_read_key(char *c)
{
  int read_output;
  read_output = read(STDIN_FILENO, c, 1);

  if ((read_output == -1) && (errno != EAGAIN))
  {
    return 1;
  }

  return 0;
}
int ctui_editor_proccess_key(struct ctui_terminal_size *terminal_size)
{
  char c = '\0';
  int buffer_size;
  if (ctui_editor_read_key(&c))
  {
    return 1;
  }
  
  switch (c)
  {
    case CTRL_KEY('q'):
    {
      return 1;
    }
  }
  return 0;
}
