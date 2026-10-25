#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <dynamic_array.h>
#include <ctui_general_terminal.h>
#include <ctui_io_control.h>

int main (int argc, char **argv)
{
  // state
  int loop = 1;
  struct termios original_termios;
  struct ctui_terminal_size terminal_size;
  struct ctui_write_buffer wb;
  struct ctui_time_data td;
  // startup
  {
    if (ctui_time_data_create(&td))
    {
      loop = 0;
    }
    if (ctui_write_buffer_create(&wb))
    {
      loop = 0;
    }
    if (ctui_sigwinch_init())
    {
      loop = 0;
    }
    if (tcgetattr(STDIN_FILENO, &original_termios) == -1)
    {
      loop = 0;
    }
    struct termios raw = original_termios;
    ctui_termois_raw_mode_enable(&raw);

    if (ctui_terminal_size_init(&terminal_size))
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
      if (ctui_time_data_update(&td))
      {
        loop = 0;
      }
      if (ctui_sigwinch_catch)
      {
        ctui_get_window_size(&(terminal_size.columns), &(terminal_size.rows));
      }
      // loop = !(ctui_editor_refresh_screen(&terminal_size, write_buffer) || ctui_editor_proccess_key(&terminal_size));
      loop = !ctui_editor_proccess_key(&terminal_size);
      if (ctui_write_buffer_ready_to_write(&wb, td.delta_time))
      {
        loop = !ctui_editor_refresh_screen(&terminal_size, &(wb.data));
        if (ctui_write_buffer_write(&wb))
        {
          loop = 0;
        }
      }
    }
  }
  // exit
  {
    if (write(STDOUT_FILENO, "\x1b[2J\x1b[H", 7))
    {
      
    }
    ctui_termios_exit(&original_termios);
  }
  printf("columns: %d, rows: %d\n", terminal_size.columns, terminal_size.rows);
  printf("%d\n", (int)strlen("\u0077"));
  return 0;
}