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
  // Create parsers
  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* Lispy = mpc_new("lispy");

  // define grammar
  mpca_lang(MPCA_LANG_DEFAULT, 
    "                                                       \
      number     : /-?[0-9.]+/ ;                             \
      operator   : '+' | '-' | '*' | '/' ;                  \
      expr       : (<number> (<operator> <number>?)* <number>?)+ | '(' <expr>+ ')';  \
      lispy      : /^/ <expr>+ (<operator> <expr>+)* /$/ ;             \
    ",
    Number, Operator, Expr, Lispy);


  // print version etc
  puts("LispLove Version 0.0.0.0.1");
  puts("Press Ctrl+c to exit\n");

  while(1) {
    // fputs("LispLove> ", stdout);

    // fgets(input, 2048, stdin);

    char* input = readline("LispLove> ");

    add_history(input);

    // printf("No you're a %s\n", input);
    // try to parse the input

    mpc_result_t r;

    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      mpc_ast_print(r.output);
      mpc_ast_delete(r.output);
    } else {
      // print error
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);
  }

  mpc_cleanup(4, Number, Operator, Expr, Lispy);

  return 0;
}
