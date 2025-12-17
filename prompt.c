#include <stdio.h>
#include "shell.h"

void print_prompt1(void) {
  fprintf(stderr, "$ "); // PS1 First prompt string when shell is waiting for you to enter command
}

void print_prompt2(void) {
  fprintf(stderr, "> "); // PS2 when you enter multi-line command.
}

