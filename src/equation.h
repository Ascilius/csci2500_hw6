#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_STRING_SIZE 128
#define MAX_TOKEN_SIZE 8

// ---------------------------------------------------------------------------

typedef struct Expression {
  char rs[4];
  char op; // single char (+, -, *, /, %)
  char rt[MAX_TOKEN_SIZE];

  bool con; // second operand is a constant
  bool neg; // second constant operand is negative

  struct Expression* left_ex; // pointer to left expression (if applicable)
} Expression;

Expression* alloc_ex() {
  Expression* new_ex = (Expression*) malloc(sizeof(Expression));
  
  strcpy(new_ex->rs, "$sx");
  new_ex->op = '?';
  strcpy(new_ex->rt, "$sx");
  new_ex->con = false;
  new_ex->neg = false;
  new_ex->left_ex = NULL;
  
  return new_ex;
}

void free_ex(Expression* ex) {
  if (ex->left_ex != NULL)
    free_ex(ex->left_ex);
  free(ex);
}

void print_ex(Expression* ex, char* buf) {
  if (ex != NULL) {
    printf("%s  rs: %s\n", buf, ex->rs);
    printf("%s  op: %c\n", buf, ex->op);
    printf("%s  rt: %s\n", buf, ex->rt);
    printf("%s  con: %d\n", buf, ex->con);
    printf("%s  neg: %d\n", buf, ex->neg);
    printf("%s  left_ex: %p\n", buf, ex->left_ex);
  }
}

// ---------------------------------------------------------------------------

typedef struct Equation {
  char og[MAX_STRING_SIZE]; // original operation in string
  char rd[4];
  char im[MAX_TOKEN_SIZE]; // load immediate
  Expression* ex; // operation
} Equation;

Equation* alloc_eq(char* line) {
  Equation* new_eq = (Equation*) malloc(sizeof(Equation));
  
  strcpy(new_eq->og, line);
  strcpy(new_eq->rd, "$sx");
  strcpy(new_eq->im, "?");
  
  new_eq->ex = NULL;
  
  return new_eq;
}

void free_eq(Equation* eq) {
  if (eq->ex != NULL)
    free_ex(eq->ex);
  free(eq);
}

void print_eq(Equation* eq) {
  if (eq != NULL) {
    printf("  og: %s\n", eq->og);
    printf("  rd: %s\n", eq->rd);
    printf("  im: %s\n", eq->im);
    printf("  ex: %p\n", eq->ex);
  }
}

// ---------------------------------------------------------------------------

void print_ex_tree(Expression* curr_ex) {
  // go as far down first
  if (curr_ex->left_ex != NULL)
    print_ex_tree(curr_ex->left_ex);

  // printing expression
  printf("  %p:\n", curr_ex);
  print_ex(curr_ex, "  ");
}

void print_tree(Equation** eqs, const int eq_size) {
  for (int i = 0; i < eq_size; ++i) {
    Equation* curr_eq = eqs[i];
    printf("%p:\n", curr_eq);
    print_eq(curr_eq);
    
    // operation instruction
    if (curr_eq->ex != NULL)
      print_ex_tree(curr_eq->ex);
  }
}
