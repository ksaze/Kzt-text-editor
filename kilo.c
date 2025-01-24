/*** includes ***/
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <errno.h>


/*** defines ***/
#define KILO_VERSION "0.0.1"
#define CTRL_KEY(k) ((k) & 0x1f) //Bitmap applied to the ASCII value of the key pressed to strip away the 2 MSB (Retaining 5 bits). Take 'A' (65: 1000001) => 'C-a' (1: 00001)

enum editorKey {
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  DEL_KEY,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN,
};

/*** data ***/
struct editorConfig {
  int cx, cy;
  struct termios orig_termios; //stores the original terminal configuration to return to after exiting the program
  int screencols;
  int screenrows;

};

struct editorConfig E;

/*** terminal config ***/
void die(const char *s){
  // Clear the screen and position the cursor 
  write(STDOUT_FILENO, "\x1b[2J", 4); 
  write(STDOUT_FILENO, "\x1b[H", 3);

  perror(s);
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) die("tcsetattr");
}

void enableRawMode(void){
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

/*** append buffer ***/
// Defines a dynamic string buffer
typedef struct abuf {
  char *b;
  int len;
} abuf;

#define ABUF_INIT {NULL, 0}

void abAppend(abuf* ab, const char* s, int len){
  char* new = realloc(ab->b, ab->len + len);

  if (new == NULL) return;

  memcpy(&new[ab->len], s, len);
  ab->b = new;
  ab->len += len;
}

void abFree(abuf* ab){
  free(ab->b);
}

/*** output ***/
void editorDrawRows(abuf *ab) {
  for (int y = 0; y < E.screenrows; y++){
    if (y == E.screenrows/3) {
      char footer[80];
      int footerlen = snprintf(footer, sizeof(footer), "kzt editor --version %s", KILO_VERSION);
      if (footerlen > E.screencols) footerlen = E.screencols;

      int padding = (E.screencols - footerlen)/2;
      if (padding){
        abAppend(ab, "~", 1);
        padding--;
      }
      while (padding--) abAppend(ab, " ", 1);
      abAppend(ab, footer, footerlen);
    }
    else{
      abAppend(ab, "~", 1);
    }

    abAppend(ab, "\x1b[K", 3); // "K" is the escape sequence for clearing the given cursor. 0 (def) for right of the cursor, 1 for the left, and 2 for the entire line
    if (y < E.screenrows - 1){
      abAppend(ab, "\r\n", 2);
    }
  }
}

void editorRefreshScreen() {
  abuf ab = ABUF_INIT;
  
  abAppend(&ab, "\x1b[?25l", 6); //"l" is the escape sequence for reset mode (disable). In this case ?25 is used for disabling the cursor while rendering.
  abAppend(&ab, "\x1b[H", 3); // "H" is the escape sequence for cursor position. Default value is already 1;1.

  editorDrawRows(&ab);

  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy+1, E.cx+1); // Move the cursor to the position specified in the editor config 
  abAppend(&ab, buf, strlen(buf));

  abAppend(&ab, "\x1b[?25h", 6); // 'h' is the escape sequence for set mode. ?25 now turns the cursor back on .
  
  write(STDOUT_FILENO, ab.b, ab.len); //Write the entire buffer to render current screen.
  abFree(&ab);
}

/*** input ***/

void editorMoveCursor(int key){
  switch (key){
    case ARROW_LEFT:
      if (E.cx != 0) E.cx--;
      break;
    case ARROW_RIGHT:
      if (E.cx != E.screencols - 1) E.cx++;
      break;
    case ARROW_DOWN:
      if (E.cy != E.screenrows - 1 ) E.cy++;
      break;
    case ARROW_UP:
      if (E.cy != 0) E.cy--;
      break;
  }
}

void editorProcessKeyPress(){
  int c = editorReadKey();

  switch (c){
    case CTRL_KEY('q'):
      // Clear the screen and position the cursor 
      write(STDOUT_FILENO, "\x1b[2J", 4); 
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(0);
      break;

    case HOME_KEY:
      E.cx = 0; break;
    case END_KEY:
      E.cx = E.screencols-1; break;

    case PAGE_UP:
    case PAGE_DOWN:
      {
        int times = E.screenrows;
        while (times--)
          editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
      }
      break;

    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_RIGHT:
    case ARROW_LEFT:
      editorMoveCursor(c); break;
  }
  
}

/*** init ***/
void initEditor(){
  E.cx = 0;
  E.cy = 0;

  if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
}

int main(void){
  enableRawMode();
  initEditor();
  
  while (1){
    editorRefreshScreen();
    editorProcessKeyPress();
  }

  return 0;
}
