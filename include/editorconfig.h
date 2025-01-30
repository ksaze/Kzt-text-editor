#ifndef EDITORCONFIG_H 
#define EDITORCONFIG_H

#include <termios.h>
#include <time.h>

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
  char* filename;
  char statusmsg[80];
  time_t statusmsg_time;

};

extern struct editorConfig E;

#endif 
