#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <errno.h>
#include <sys/ioctl.h>

#define CTRL_KEY(k) ((k) & 0x1f)

struct write_buffer
{
    size_t size;
    char *data;
};
struct editor_state
{
    unsigned short columns;
    unsigned short rows;
};

void exit_error(const char *e)
{
    perror(e);
    exit(1);
}
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

int get_cursor_position(unsigned short *columns, unsigned short *rows)
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
int get_window_size(unsigned short *columns, unsigned short *rows)
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

int editor_clear_screen()
{
    // clear screen
    if (write(STDOUT_FILENO, "\x1b[2J", 4) == -1)
    {
        return 1;
    }
    // move cursor to (1, 1)
    if (write(STDOUT_FILENO, "\x1b[H", 3) == -1)
    {
        return 1;
    }
    return 0;
}
int editor_refresh_screen(struct editor_state *state)
{
    if (editor_clear_screen())
    {
        return 1;
    }

    // draw rows
    for (int i = 0; i < state->rows; i++)
    {
        if (write(STDOUT_FILENO, "~", 1) == -1)
        {
            return 1;
        }
        if (i < state->rows - 1)
        {
            if (write(STDOUT_FILENO, "\r\n", 2) == -1)
            {
                return 1;
            }
        }
    }
    // move cursor to (1, 1)
    if (write(STDOUT_FILENO, "\x1b[H", 3) == -1)
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
    // startup
    int loop = 1;
    struct termios original_termios;
    struct editor_state state;
    struct write_buffer wb;
    {
        if (tcgetattr(STDIN_FILENO, &original_termios) == -1)
        {
            return 1;
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
            loop = !(editor_refresh_screen(&state) || editor_proccess_key(&state));
        }
    }
    // exit
    {
        editor_clear_screen();
        termios_exit(&original_termios);
    }
    return 0;
}