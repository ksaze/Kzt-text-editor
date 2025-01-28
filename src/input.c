#include <config.h>

#include <unistd.h>
#include <stdlib.h>

#include <util.h>
#include <editorconfig.h>
#include <terminal.h>

void editorMoveCursor(int key){
  erow* row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];

  switch (key){
    case ARROW_LEFT:
      if (E.cx != 0){
        E.cx--;
      }
      else if (E.cy > 0){ // Move to the previous line when at the beginning of a line
        E.cy--;
        E.cx = E.row[E.cy].size;
      }
      break;
    case ARROW_RIGHT:
      if (row && E.cx < row->size){
        E.cx++;
      }
      else if (row && E.cx == row->size){ // Move to the next line when at the end of a line
        E.cy++;
        E.cx = 0;
      }
      break;
    case ARROW_DOWN:
      if (E.cy < E.numrows) E.cy++;
      break;
    case ARROW_UP:
      if (E.cy != 0) E.cy--;
      break;
  }

  // Fix cursor when going to a line of shorter length to make sure cursor doesn't go beyond the last char of the line
  if (key == ARROW_UP || key == ARROW_DOWN){
    row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
    int rowlen = row ? row->size : 0;

    if (E.cx > rowlen){
      E.cx = rowlen;
    }
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


