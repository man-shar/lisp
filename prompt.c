#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

/* If we are compiling on Windows compile these functions */
#ifdef _WIN32
#include <string.h>

static char buffer[2048]

// fake readline function
char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer) + 1);
  cpy[strlen(cpy) - 1] '\0';
  return cpy;
}

// fake add_history function
void add_history(char* unused) {}

// otherwise, on mac use editline headers
#else
#include <editline/readline.h>
#endif

int main(int argc, char const *argv[]) {
  // print version etc
  puts("LispLove Version 0.0.0.0.1");
  puts("Press Ctrl+c to exit\n");

  while(1) {
    // fputs("LispLove> ", stdout);

    // fgets(input, 2048, stdin);

    char* input = readline("LispLove> ");

    add_history(input);

    printf("No you're a %s\n", input);

    free(input);
  }

  return 0;
}
