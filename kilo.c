#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h> 
#include <termios.h>

/*** data ***/

struct termios orig_termios;

/*** terminal ***/

void die(const char *s) {
  perror(s);
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) die("tcsetattr");
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
  atexit(disableRawMode);

  struct termios raw = orig_termios;
  
  tcgetattr(STDIN_FILENO, &raw);
  // Ignores ctrl-s and ctrl-q XOFF and XON stops data and turns back on data transmission
  // Carriage reutrn and new line turned off
  raw.c_iflag &= ~(ICRNL | IXON); // input flags
  // Turns off echo (no more printing characters)
  // Canoncial mode being turned off so line by line instead
  // Ignore ctrl c and ctrl z signals with ISIG being turned off
  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN); // local flags
  // Turn off all output processing features
  raw.c_oflag &= ~(OPOST);

  // Min number of bytes input needed before read() can return
  raw.c_cc[VMIN] = 0;

  // Amount of time to wait before read returns
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) die("tcsetattr");
}

/*** init ***/

int main() {
  enableRawMode();

  while (1)  {
    char c = '\0';
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
    if (iscntrl(c)) {
      printf("%d\r\n", c);
    } else {
      printf("%d ('%c')\r\n", c, c);
    }
    if (c == 'q') break;
  };
  return 0;
}










