#ifndef DISPLAY_H
#define DISPLAY_H

typedef struct abuf {
  char *b;
  int len;
} abuf;

#define ABUF_INIT {NULL, 0}

void abAppend(abuf* ab, const char* s, int len);
void abFree(abuf* ab);
void editorDrawRows(abuf* ab);
void editorRefreshScreen();

#endif
