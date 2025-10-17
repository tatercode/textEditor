#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h> 
#include <termios.h>

/*** defines ***/
#define CTRL_KEY(k) ((k) & 0x1f)

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

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

/*** output ***/
void editorDrawRows() {
  int y;
  for (y = 0; y < 24; y++) {
    write(STDOUT_FILENO, "~\r\n", 3);
  }
}

/*** input ***/
char editorReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  return c;
}

void editorProcessKeyPress() {
  char c = editorReadKey();
  switch (c) {
    case CTRL_KEY('q'):
      exit(0);
      break;
  }
}

void editorRefreshScreen() {
  // Clear entire screen
  write(STDOUT_FILENO, "\x1b[2J", 4);

  // Place cursor at top left
  write(STDOUT_FILENO, "\x1b[H", 3);

  editorDrawRows();
  write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** init ***/

int main() {
  enableRawMode();

  while (1)  {
    editorRefreshScreen();
    editorProcessKeyPress();
  };
  return 0;
}










