#ifndef EDITORCONFIG_H 
#define EDITORCONFIG_H

#include <termios.h>

typedef struct erow {
  int size;
  char* chars;
} erow;

struct editorConfig {
  int cx, cy;
  int rowoff;
  int coloff;
  struct termios orig_termios; //stores the original terminal configuration to return to after exiting the program
  int screencols;
  int screenrows;
  int numrows;
  erow* row;
};

extern struct editorConfig E;

#endif 
