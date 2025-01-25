#include <unistd.h>
#include <stdlib.h>

#include <util.h>
#include <editorconfig.h>
#include <terminal.h>
#include <config.h>

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


