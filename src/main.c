#include <editorconfig.h>
#include <terminal.h>
#include <display.h>
#include <config.h>
#include <input.h>
#include <util.h>

struct editorConfig E;

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
