#include <termios.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>

#include <util.h>
#include <editorconfig.h>

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) die("tcsetattr");
}

void enableRawMode(){
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");
  atexit(disableRawMode);

  struct termios raw = E.orig_termios;

  // c_lflag (local flags) is a bitflag for storing miscellaneous flags. We need to disable some flags like:
  // ICANON: For canonoical mode
  // ECHO: For printing input text
  // ISIG: For disabling C-c & C-z
  // IEXTEN: For disabling C-v 
  // The constant for these flags represent the bitflag with only their respective as 1 and all other as 0. These can be used to disable them in the current config using bit manipulation
  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
  
  // c_iflag is for input flags. Similar to the previous line, we use:
  // IXON to disable C-s and C-q which are used for software flow control 
  // ICRNL [Carriage Return New Line] to disable C-m returning /n instead of /r
  // BRKINT: Turning on allows the break condition to result in SIGINT signal [C-c]
  // INPCK: Enables parity checking which doesn't seem to apply for modern terminals 
  // ISTRIP: Causes the 8th bit of the each input byte to be stripped (set to 0). Already off, most probaby
  raw.c_iflag &= ~(BRKINT | IXON | ICRNL | ISTRIP | INPCK);

  // Output flags:
  // OPOST: Converts /n to /r/n 
  raw.c_oflag &= ~(OPOST);

  // Control flags: CS8 is a bit mask used in conjuction with the bitwise-or for setting the character size (CS) to 8 bits per byte  
  raw.c_cflag |= (CS8);

  // CC is an array of control characters. VMIN sets the min number of bytes before read() can return and VTIME sets the maximum time (in 1/10s units) to wait before read() returns
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

int editorReadKey() {
  int nread;
  char c;

  while ((nread = read(STDIN_FILENO, &c, 1)) != 1){
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  
  if (c == '\x1b'){
    char seq[3];

    if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

    if (seq[0] == '['){
      if (seq[1] >= '0' && seq[1] <= '9'){
        if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
        if (seq[2] == '~'){
          switch(seq[1]){
            case '1': return HOME_KEY;
            case '3': return DEL_KEY;
            case '4': return END_KEY;
            case '5': return PAGE_UP;
            case '6': return PAGE_DOWN;
            case '7': return HOME_KEY;
            case '8': return END_KEY;
          }
        }
      }
      switch(seq[1]){
        case 'A': return ARROW_UP;
        case 'B': return ARROW_DOWN;
        case 'C': return ARROW_RIGHT;
        case 'D': return ARROW_LEFT;
        case 'H': return HOME_KEY;
        case 'F': return END_KEY;
      }
    }
    else if (seq[0] == 'O') {
      switch (seq[1]) {
        case 'H': return HOME_KEY;
        case 'F': return END_KEY;
      }
    }
    return '\x1b'; //No matching escape sequence
  }
  return c;
}

int getCursorPosition(int *rows, int *cols) {
  char buf[32];
  unsigned int i = 0;

  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;
  
  while (i < sizeof(buf)-1){
    if (read(STDOUT_FILENO, &buf[i], 1) != 1) break;
    if (buf[i] == 'R') break;
    i++;
  }
  
  buf[i] = '\0';

  if (buf[0] != '\x1b' || buf[1] != '[') return -1;
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;
  
  return 0;
}

int getWindowSize(int* rows, int* cols) {
  struct winsize ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0){
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
    return getCursorPosition(rows, cols);
  }

  else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }

}
