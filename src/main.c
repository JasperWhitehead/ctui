#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <dynamic_array.h>
#include <general_terminal.h>

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
    ctui_termois_raw_mode_enable(&raw);

    if (ctui_editor_state_init(&state))
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
      loop = !(ctui_editor_refresh_screen(&state, write_buffer) || ctui_editor_proccess_key(&state));
    }
  }
  // exit
  {
    write(STDOUT_FILENO, "\x1b[2J\x1b[H", 7);
    ctui_termios_exit(&original_termios);
  }
  return 0;
}