#include <config.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

#include <editorconfig.h>
#include <display.h>

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


void editorScroll(){
  // Vertical Scroll
  if (E.cy < E.rowoff){
    E.rowoff = E.cy;
  }

  else if (E.cy >= E.rowoff + E.screenrows){
    E.rowoff = E.cy - E.screenrows + 1;
  }

  // Horizontal Scroll
  if (E.cx < E.coloff){
    E.coloff = E.cx;
  }

  else if (E.cx >= E.coloff + E.screencols){
    E.coloff = E.cx - E.screencols + 1;
  }
}

void editorDrawRows(abuf *ab) {
  for (int y = 0; y < E.screenrows; y++){
    int filerow = y + E.rowoff;
    if (filerow >= E.numrows){
      if (E.numrows == 0 && y == E.screenrows/3) {
        char welcome[80];
        int welcomelen = snprintf(welcome, sizeof(welcome), "kzt editor --version %s", KZT_VERSION);
        if (welcomelen > E.screencols) welcomelen = E.screencols;

        int padding = (E.screencols - welcomelen)/2;
        if (padding){
          abAppend(ab, "~", 1);
          padding--;
        }
        while (padding--) abAppend(ab, " ", 1);
        abAppend(ab, welcome, welcomelen);
      }
      else{
        abAppend(ab, "~", 1);
      }
    }
    else{
      int len = E.row[filerow].size - E.coloff;
      if (len < 0) len = 0;
      if (len > E.screencols) len = E.screencols;
      abAppend(ab, &E.row[filerow].chars[E.coloff], len);
    }

    abAppend(ab, "\x1b[K", 3); // "K" is the escape sequence for clearing the line for a given cursor. 0 (def) for right of the cursor, 1 for the left, and 2 for the entire line
    abAppend(ab, "\r\n", 2);
    
  }
}

void editorDrawStatusBar(struct abuf *ab) {
  abAppend(ab, "\x1b[7m", 4); // Invert color by turning on 7 argument in Select Graphic Rendition (m)
  char status[80], rstatus[80];

  int len = snprintf(status, sizeof(status), "%.20s - %d lines",
    E.filename ? E.filename : "[No Name]", E.numrows);
  int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d",
    E.cy + 1, E.numrows);

  if (len > E.screencols) len = E.screencols;
  abAppend(ab, status, len);

  while (E.screencols - len > rlen) { // Add space until there is only space for the status message
    abAppend(ab, " ", 1);
    len++;
  }
  abAppend(ab, rstatus, rlen);

  abAppend(ab, "\x1b[m", 3); // Go back to normal graphic rendering
  abAppend(ab, "\r\n", 2); // Line for status message
}

void editorDrawMessageBar(struct abuf *ab){
  abAppend(ab, "\x1b[K", 3);

  int msglen = strlen(E.statusmsg);
  if (msglen > E.screencols) msglen = E.screencols;

  if (msglen && time(NULL) - E.statusmsg_time < 5){
    abAppend(ab, E.statusmsg, msglen);
  }
}

void editorRefreshScreen() {
  editorScroll();

  abuf ab = ABUF_INIT;
  
  abAppend(&ab, "\x1b[?25l", 6); //"l" is the escape sequence for reset mode (disable). In this case ?25 is used for disabling the cursor while rendering.
  abAppend(&ab, "\x1b[H", 3); // "H" is the escape sequence for cursor position. Default value is already 1;1.

  editorDrawRows(&ab);
  editorDrawStatusBar(&ab);
  editorDrawMessageBar(&ab);

  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy-E.rowoff+1, E.cx-E.coloff+1); // Move the cursor to the position specified in the editor config 
  abAppend(&ab, buf, strlen(buf));

  abAppend(&ab, "\x1b[?25h", 6); // 'h' is the escape sequence for set mode. ?25 now turns the cursor back on .
  
  write(STDOUT_FILENO, ab.b, ab.len); //Write the entire buffer to render current screen.
  abFree(&ab);
}

void editorSetStatusMessage(const char* fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
  va_end(ap);
  E.statusmsg_time = time(NULL);
}
