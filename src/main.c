#include <config.h>

#include <stddef.h>

#include <editorconfig.h>
#include <terminal.h>
#include <display.h>
#include <input.h>
#include <util.h>
#include <fileio.h>

struct editorConfig E;

void initEditor(){
  E.cx = 0;
  E.cy = 0;
  E.rowoff = 0;
  E.numrows = 0;
  E.row = NULL; 

  if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
}

int main(int argc, char* argv[]){
  enableRawMode();
  initEditor();
  if (argc >= 2){
    editorOpen(argv[1]);
  }
  
  while (1){
    editorRefreshScreen();
    editorProcessKeyPress();
  }

  return 0;
}
