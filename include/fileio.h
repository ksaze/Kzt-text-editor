#ifndef FILEIO_H
#define FILEIO_H

void editorSave();
void editorOpen(char* filename);
void editorAppendRow(char* s, size_t len);
void editorRowInsertChar(erow* row, int at, int c);

#endif
