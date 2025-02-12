#ifndef EDITORCONFIG_H 
#define EDITORCONFIG_H

#include <termios.h>
#include <time.h>

typedef struct erow {
  int size;
  int rsize;
  char* chars;
  char* render;
} erow;

struct editorConfig {
  int cx, cy;
  int rx;
  int rowoff;
  int coloff;
  struct termios orig_termios; //stores the original terminal configuration to return to after exiting the program
  int screencols;
  int screenrows;
  int numrows;
  erow* row;
  int dirty;
  char* filename;
  char statusmsg[80];
  time_t statusmsg_time;

};

extern struct editorConfig E;

#endif 
