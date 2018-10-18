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

enum { ERR_DIV_BY_ZERO, ERR_INVALID_OPERATION, ERR_BAD_NUM };
enum { LVAL_NUM, LVAL_ERR };

// this struct is our representation of all calcuations in our lisp. it can be an error or a valid number
// type is number or error
// err is the kind of error that occured
typedef struct {
  int type;
  union {
    int err;
    long num;
  };
} lval;

lval create_lval_num(num) {
  lval v;
  v.type = LVAL_NUM;
  v.num = num;
  return v;
}

lval create_lval_err(err) {
  lval v;
  v.type = LVAL_ERR;
  v.err = err;
  return v;
}

lval min(lval *args, int args_length) {
  long min = args[0].num;
  for (int i = 0; i < args_length; ++i)
  {
    if (args[i].num < min) {
      min = args[i].num;
    }
  }
  return create_lval_num(min);
}

lval max(lval *args, int args_length) {
  long max = args[0].num;
  for (int i = 0; i < args_length; ++i)
  {
    if (args[i].num > max) {
      max = args[i].num;
    }
  }
  return create_lval_num(max);
}

lval eval_op(lval x, char* op, lval y) {
  /* If either value is an error return it */
  if (x.type == LVAL_ERR) { return x; }
  if (y.type == LVAL_ERR) { return y; }

  if (strcmp(op, "+") == 0) { return create_lval_num(x.num + y.num); }
  if (strcmp(op, "-") == 0) { return create_lval_num(x.num - y.num); }
  if (strcmp(op, "*") == 0) { return create_lval_num(x.num * y.num); }
  if (strcmp(op, "%") == 0) { 
    return (y.num == 0) ?
      create_lval_err(ERR_DIV_BY_ZERO)
      : create_lval_num(x.num % y.num);
  }
  if (strcmp(op, "/") == 0) { 
    return (y.num == 0) ?
      create_lval_err(ERR_DIV_BY_ZERO)
      : create_lval_num(x.num / y.num);
  }

  return create_lval_err(ERR_INVALID_OPERATION);
}

lval eval_func(char *func, lval *args, int args_length) {
  if (strcmp(func, "min") == 0) { return min(args, args_length); }
  if (strcmp(func, "max") == 0) { return max(args, args_length); }

  return create_lval_err(ERR_INVALID_OPERATION);
}


// this evaluates an AST. recursively calling itself to evaluate child ASTs
// this... works... somehow.. hmm.
lval eval (mpc_ast_t* t) {
  if (strstr(t->tag, "number")) {
    // atoi returns 0 on error. boo.
    // return atoi(t->contents);
    // use stol instead
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? create_lval_num(x) : create_lval_err(ERR_BAD_NUM);
  }

  // if function, run eval_func with all other as arguments
  if (strstr(t->children[0]->tag, "function")) {
    lval args[t->children_num - 1];

    for (int i = 0; i < (t->children_num - 1); ++i) {
      args[i] = eval(t->children[i + 1]);
      // if this is an error, fuck it
      if (args[i].type == LVAL_ERR) {
        return create_lval_err(ERR_INVALID_OPERATION);
      }
    }

    return eval_func(t->children[0]->contents, args, (t->children_num - 1));
  }

  lval x = eval(t->children[1]);

  int count = t->children_num;

  for (int i = 2; i < (count - 1); i = i + 2)
  {
    char* op = t->children[i]->contents;
    mpc_ast_t* right = t->children[i + 1];
    x = eval_op(x, op, eval(right));
  }

  return x;
}

void lval_print(lval x) {
  switch(x.type) {
    case LVAL_NUM:
      printf("Calculated value: %li\n", x.num);
      break;

    case LVAL_ERR:
      switch(x.err) {
        case ERR_INVALID_OPERATION:
          puts("Invalid Operation!");
          break;
        case ERR_DIV_BY_ZERO:
          puts("Division by zero!");
          break;
        case ERR_BAD_NUM:
          puts("Invalid Number!");
          break;
      }
      break;

    default:
      puts("Something went wrong.");
  }
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
      operator   : '+' | '-' | '*' | '/' | '%' ;                  \
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
      lval result = eval(r.output);
      lval_print(result);
      // printf("%li\n", result);

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
