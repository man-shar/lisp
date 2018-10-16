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

long min(long *args, int args_length) {
  long min = args[0];
  for (int i = 0; i < args_length; ++i)
  {
    if (args[i] < min) {
      min = args[i];
    }
  }
  return min;
}

long max(long *args, int args_length) {
  long max = args[0];
  for (int i = 0; i < args_length; ++i)
  {
    if (args[i] > max) {
      max = args[i];
    }
  }
  return max;
}

long eval_op(long x, char* op, long y) {
  if (strcmp(op, "+") == 0) { return x + y; }
  if (strcmp(op, "-") == 0) { return x - y; }
  if (strcmp(op, "*") == 0) { return x * y; }
  if (strcmp(op, "/") == 0) { return x / y; }
  return 0;
}

long eval_func(char *func, long *args, int args_length) {
  if (strcmp(func, "min") == 0) { return min(args, args_length); }
  if (strcmp(func, "max") == 0) { return max(args, args_length); }

  return 0;
}


// this evaluates an AST. recursively calling itself to evaluate child ASTs
// this... works... somehow.. hmm.
long eval (mpc_ast_t* t) {
  if (strstr(t->tag, "number")) {
    return atoi(t->contents);
  }

  // if function, run eval_func with all other as arguments
  if (strstr(t->children[0]->tag, "function")) {
    long args[t->children_num - 1];

    for (int i = 0; i < (t->children_num - 1); ++i) {
      args[i] = eval(t->children[i + 1]);
    }

    return eval_func(t->children[0]->contents, args, (t->children_num - 1));
  }

  long x = eval(t->children[1]);

  int count = t->children_num;

  for (int i = 2; i < (count - 1); i = i + 2)
  {
    char* op = t->children[i]->contents;
    mpc_ast_t* right = t->children[i + 1];
    x = eval_op(x, op, eval(right));
  }

  return x;
}

int main(int argc, char const *argv[]) {
  // Create parsers
  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Function = mpc_new("function");
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* Lispy = mpc_new("lispy");

  // define grammar
  mpca_lang(MPCA_LANG_DEFAULT, 
    "                                                       \
      number     : /-?[0-9.]+/ ;                             \
      operator   : '+' | '-' | '*' | '/' ;                  \
      function   : \"min\" | \"max\" ;                  \
      expr       : <number> | '(' <expr>+ (<operator> <expr>+)* ')' | <function> <expr>+;  \
      lispy      : /^/ <expr>+ (<operator> <expr>+)* /$/ ;             \
    ",
    Number, Operator, Function, Expr, Lispy, NULL);


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
      long result = eval(r.output);
      printf("%li\n", result);

      mpc_ast_delete(r.output);
    } else {
      // print error
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);
  }

  mpc_cleanup(5, Number, Operator, Function, Expr, Lispy);

  return 0;
}
