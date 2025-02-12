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
  E.rx = 0;
  E.rowoff = 0;
  E.coloff = 0;
  E.numrows = 0;
  E.row = NULL; 
  E.dirty = 0;
  E.filename = NULL;
  E.statusmsg[0] = '\0';
  E.statusmsg_time = 0;

  if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
  E.screenrows -= 2; // Lines for status bar and status message
}

int main(int argc, char* argv[]){
  enableRawMode();
  initEditor();
  if (argc >= 2){
    editorOpen(argv[1]);
  }

  editorSetStatusMessage("HELP: Ctrl-Q to quit | Ctrl-S to save file");
  
  while (1){
    editorRefreshScreen();
    editorProcessKeyPress();
  }

  return 0;
}
