#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void die(const char *s){
  // Clear the screen and position the cursor 
  write(STDOUT_FILENO, "\x1b[2J", 4); 
  write(STDOUT_FILENO, "\x1b[H", 3);

  perror(s);
  exit(1);
}
