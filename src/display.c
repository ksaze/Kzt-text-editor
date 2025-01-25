#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <editorconfig.h>
#include <display.h>
#include <config.h>



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
