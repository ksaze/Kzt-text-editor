/*** includes ***/
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

/*** data ***/
struct termios orig_termios; //stores the original terminal configuration to return to after exiting the program


/*** terminal config ***/
void die(const char *s){
  perror(s);
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) die("tcsetattr");
}

void enableRawMode(void){
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
  atexit(disableRawMode);

  struct termios raw = orig_termios;

  // c_lflag (local flags) is a bitflag for storing miscellaneous flags. We need to disable some flags like:
  // ICANON: For canonoical mode
  // ECHO: For printing input text
  // ISIG: For disabling C-c & C-z
  // IEXTEN: For disabling C-v 
  // The constant for these flags represent the bitflag with only their respective as 1 and all other as 0. These can be used to disable them in the current config using bit manipulation
  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
  
  // c_iflag is for input flags. Similar to the previous line, we use:
  // IXON to disable C-s and C-q which are used for software flow control 
  // ICRNL [Carriage Return New Line] to disable C-m returning /n instead of /r
  // BRKINT: Turning on allows the break condition to result in SIGINT signal [C-c]
  // INPCK: Enables parity checking which doesn't seem to apply for modern terminals 
  // ISTRIP: Causes the 8th bit of the each input byte to be stripped (set to 0). Already off, most probaby
  raw.c_iflag &= ~(BRKINT | IXON | ICRNL | ISTRIP | INPCK);

  // Output flags:
  // OPOST: Converts /n to /r/n 
  raw.c_oflag &= ~(OPOST);

  // Control flags: CS8 is a bit mask used in conjuction with the bitwise-or for setting the character size (CS) to 8 bits per byte  
  raw.c_cflag |= (CS8);

  // CC is an array of control characters. VMIN sets the min number of bytes before read() can return and VTIME sets the maximum time (in 1/10s units) to wait before read() returns
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}


/*** init ***/
int main(void){
  enableRawMode();

  while (1){
    char c = '\0';
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");

    if (iscntrl(c)){
      printf("%d\r\n", c);
    }
    else{
      printf("%d ('%c')\r\n", c, c);
    }
    if (c == 'q') break;
  }

  return 0;
}
