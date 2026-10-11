#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <dynamic_array.h>

#define CTRL_KEY(k) ((k) & 0x1f)

struct editor_state
{
  int columns;
  int rows;
};

int termios_exit(struct termios *original_termios)
{
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, original_termios) == -1)
  {
    return 1;
  }
  return 0;
}
void termois_raw_mode_enable(struct termios *termios)
{
  termios->c_iflag &= ~(BRKINT | IXON | ICRNL | INPCK | ISTRIP);
  termios->c_oflag &= ~(OPOST);
  termios->c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
  termios->c_cflag |= (CS8);
  termios->c_cc[VMIN] = 0;
  termios->c_cc[VTIME] = 1;
  return;
}

int get_cursor_position(int *columns, int *rows)
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
int get_window_size(int *columns, int *rows)
{
  struct winsize window_size;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &window_size) == -1 || window_size.ws_col == 0)
  {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
    {
      return 1;
    }
    return get_cursor_position(columns, rows);
  }
  (*columns) = window_size.ws_col;
  (*rows) = window_size.ws_row;
  return 0;
}
int write_buffer_append_string(char *write_buffer, char *string, sds_da_index_t string_size)
{
  if (sds_da_add(&write_buffer, string_size, sizeof(char)))
  {
    return 1;
  }
  memcpy((write_buffer + sds_da_get_size(write_buffer) - string_size), string, string_size);
  return 0;
}
int editor_refresh_screen(struct editor_state *state, char *write_buffer)
{
  sds_da_clear(write_buffer);
  // hide cursor, clear screen, move cursor to (1, 1)
  write_buffer_append_string(write_buffer, "\x1b[?25l\x1b[2J\x1b[H", 13);
  // draw rows
  for (int i = 0; i < state->rows; i++)
  {
    write_buffer_append_string(write_buffer, "~\x1b[K", 4);
    if (i < state->rows - 1)
    {
      write_buffer_append_string(write_buffer, "\r\n", 2);
    }
  }
  // move cursor to (1, 1), unhide cursor
  write_buffer_append_string(write_buffer, "\x1b[H\x1b[?25h", 9);
  // write buffer
  if (write(STDOUT_FILENO, write_buffer, sds_da_get_size(write_buffer)) == -1)
  {
    return 1;
  }
  return 0;
}
int editor_read_key(char *c)
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
int editor_proccess_key(struct editor_state *state)
{
  char c;
  if (editor_read_key(&c))
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
int editor_state_init(struct editor_state *state)
{
  if (get_window_size(&state->columns, &state->rows))
  {
    return 1;
  }
  return 0;
}

int main (int argc, char **argv)
{
  // state
  int loop = 1;
  struct termios original_termios;
  struct editor_state state;
  char *write_buffer;
  // startup
  {
    if (sds_da_create(&write_buffer, sizeof(char), 0))
    {
      loop = 0;
    }
    if (tcgetattr(STDIN_FILENO, &original_termios) == -1)
    {
      loop = 0;
    }
    struct termios raw = original_termios;
    termois_raw_mode_enable(&raw);

    if (editor_state_init(&state))
    {
      loop = 0;
    }

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    {
      loop = 0;
    }
  }
  // loop
  {
    while (loop)
    {
      loop = !(editor_refresh_screen(&state, write_buffer) || editor_proccess_key(&state));
    }
  }
  // exit
  {
    write(STDOUT_FILENO, "\x1b[2J\x1b[H", 7);
    termios_exit(&original_termios);
  }
  return 0;
}