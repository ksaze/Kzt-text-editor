#include <config.h>

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <editorconfig.h>
#include <util.h>

void editorRowRenderTab(erow* row){
  int tabs = 0;
  for (int i = 0; i < row->size; i++){
    if (row->chars[i] == '\t') tabs++;
  }

  free(row->render);
  row->render = malloc(row->size + tabs*(TAB_STOP-1) + 1);

  int idx = 0;
  for (int i = 0; i < row->size; i++){
    if (row->chars[i] == '\t') {
      row->render[idx++] = ' ';
      while (idx % TAB_STOP != 0) row->render[idx++] = ' ';
    }
    else{
      row->render[idx++] = row->chars[i];
    }
  }

  row->render[idx] = '\0';
  row->rsize = idx;
}

void editorAppendRow(char *s, size_t len){
  E.row = realloc(E.row, sizeof(erow)*(E.numrows + 1));

  int at = E.numrows;
  E.row[at].size = len;
  E.row[at].chars = malloc(len+1);
  memcpy(E.row[at].chars, s, len);
  E.row[at].chars[len] = '\0';

  E.row[at].rsize = 0;
  E.row[at].render = NULL;
  editorRowRenderTab(&E.row[at]);

  E.numrows++;
}

void editorRowInsertChar(erow* row, int at, int c){
  if (at < 0 || at > row->size) at = row->size;
  row->chars = realloc(row->chars, row->size + 2);
  memmove(&row->chars[at+1], &row->chars[at], row->size - at + 1); // memmove is like memcpy but used when source and destination overlap
  row->size++;
  row->chars[at] = c;
  editorRowRenderTab(row);
}

void editorOpen(char* filename){
  free(E.filename);
  E.filename = strdup(filename);

  FILE* fp = fopen(filename, "r");
  if (!fp) die("fopen");

  char *line = NULL;
  size_t linecap = 0;
  ssize_t linelen; 
  while((linelen = getline(&line, &linecap, fp)) != -1) {
    while (linelen > 0 && (line[linelen-1] == '\n' || 
                           line[linelen-1] == '\r'))
      linelen--;
    editorAppendRow(line, linelen);
  } 

  free(line);
  fclose(fp);
}
