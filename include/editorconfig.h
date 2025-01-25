#ifndef EDITORCONFIG_H 
#define EDITORCONFIG_H

#include <termios.h>

struct editorConfig {
  int cx, cy;
  struct termios orig_termios; //stores the original terminal configuration to return to after exiting the program
  int screencols;
  int screenrows;

};

extern struct editorConfig E;

#endif 
