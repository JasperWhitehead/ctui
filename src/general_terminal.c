#include <general_terminal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <dynamic_array.h>s

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
int ctui_write_buffer_append_string(char *write_buffer, char *string, sds_da_index_t string_size)
{
  if (sds_da_add(&write_buffer, string_size, sizeof(char)))
  {
    return 1;
  }
  memcpy((write_buffer + sds_da_get_size(write_buffer) - string_size), string, string_size);
  return 0;
}
int ctui_editor_refresh_screen(struct editor_state *state, char *write_buffer)
{
  sds_da_clear(write_buffer);
  // hide cursor, clear screen, move cursor to (1, 1)
  ctui_write_buffer_append_string(write_buffer, "\x1b[?25l\x1b[2J\x1b[H", 13);
  // draw rows
  for (int i = 0; i < state->rows; i++)
  {
    ctui_write_buffer_append_string(write_buffer, "~\x1b[K", 4);
    if (i < state->rows - 1)
    {
      ctui_write_buffer_append_string(write_buffer, "\r\n", 2);
    }
  }
  // move cursor to (1, 1), unhide cursor
  ctui_write_buffer_append_string(write_buffer, "\x1b[H\x1b[?25h", 9);
  // write buffer
  if (write(STDOUT_FILENO, write_buffer, sds_da_get_size(write_buffer)) == -1)
  {
    return 1;
  }
  return 0;
}
int ctui_editor_read_key(char *c)
{
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
int ctui_editor_proccess_key(struct editor_state *state)
{
  char c;
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
int ctui_editor_state_init(struct editor_state *state)
{
  if (ctui_get_window_size(&state->columns, &state->rows))
  {
    return 1;
  }
  return 0;
}