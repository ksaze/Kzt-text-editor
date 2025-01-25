#ifndef TERMINAL_H
#define TERMINAL_H

void disableRawMode();
void enableRawMode();
int editorReadKey();
int getCursorPosition(int *, int *);
int getWindowSize(int *, int *);

#endif
