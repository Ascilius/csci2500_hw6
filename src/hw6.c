#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "equation.h"

// -----------------------------------------------------------------------------------------------------------------------------
// debugging
bool debug = false;
bool verbose = false;

// printing register table
void print_reg_table(char reg_table[][MAX_TOKEN_SIZE]) {
  printf("Debug: Register table:\n");
  for (int i = 0; i < 8; ++i)
    printf("  $s%d: %s\n", i, reg_table[i]);
}

// -----------------------------------------------------------------------------------------------------------------------------
// file manipulation

// open file
FILE* get_file(const char* filename) {
  FILE* file = fopen(filename, "r");
  if (file == NULL) {
    printf("ERROR: Unable to open \"%s\"!\n", filename);
    exit(1);
  }

  if (debug) printf("Debug: Opening \"%s\"...\n", filename);
  return file; // remember to close the file!
}

// read file into string array
void parse_file(FILE* file, char*** lines, int* n_lines) {
  *lines = (char**) malloc(0); // array to store lines

  if (debug) printf("Debug: Parsing:\n");
  char line[MAX_STRING_SIZE];
  while (fgets(line, MAX_STRING_SIZE, file) != NULL) {
    // increasing array size
    (*n_lines)++;
    *lines = realloc(*lines, *n_lines * sizeof(char*));
    (*lines)[*n_lines - 1] = (char*) malloc(MAX_STRING_SIZE * sizeof(char));
    char* curr_line = (*lines)[*n_lines - 1];

    // trimming newline
    line[strcspn(line, "\r\n")] = '\0';

    // saving new line
    strcpy(curr_line, line);
    if (debug) {
      printf("  %p: ", (*lines)[*n_lines - 1]);
      printf("%s (%s)\n", (*lines)[*n_lines - 1], line);
    }
  }
  if (debug) {
    printf("Debug: lines: %p\n", *lines);
    printf("Debug: n_lines: %d\n", *n_lines);
  }
  fclose(file); // done reading file
}

// -----------------------------------------------------------------------------------------------------------------------------
// processing

// find corresponding register for a variable
// if not found add to register table and return newly assigned register
void get_reg(char reg_table[][MAX_TOKEN_SIZE], char* var, char* reg) {
  // register
  strcpy(reg, "$sx"); // x will be replaced by register number

  // finding var
  if (debug) printf("Debug: Finding \"%s\"...\n", var);
  for (int i = 0; i < 8; ++i) {
    if (strcmp(reg_table[i], var) == 0) {
      reg[2] = ('0' + i);
      if (debug) printf("Debug: Returning \"%s\"...\n", reg);
      return;
    } else if (strcmp(reg_table[i], "(empty)") == 0) {
      if (debug && verbose) printf("  %d: Is empty\n", i);
    } else
      if (debug && verbose) printf("  %d: Not found\n", i);
  }

  if (debug) printf("Debug: Adding \"%s\" to register table...\n", var);
  for (int i = 0; i < 8; ++i) {
    if (strcmp(reg_table[i], "(empty)") == 0) {
      strcpy(reg_table[i], var);
      reg[2] = ('0' + i);
      if (debug) printf("Debug: Returning \"%s\"...\n", reg);
      return;
    } else
      if (debug && verbose) printf("  %d: Not empty\n", i);
  }

  printf("ERROR: Register table is full\n");
}

// get next token and print
char* nexttok() {
  char* tok = strtok(NULL, " ;"); // next token
  if (debug)
    printf("\nDebug: tok: %s\n", tok);
  return tok;
}

// check if a token is numeric
bool isnumeric(char* tok) {
  int size = strlen(tok);
  for (int i = 0; i < size; ++i) {
    char c = tok[i];
    if (debug && verbose)
      printf("  %c\n", c);
    
    if (i == 0 && c == '-') // could be a negative number
      continue;

    // checking if character is a number
    if (c < '0' || c > '9') {
      if (debug)
        printf("Debug: %s is not numeric\n", tok);
      return false;
    }
  }
  if (debug)
    printf("Debug: %s is numeric\n", tok);
  return true;
}

// tree building
void make_tree(char** lines, const int n_lines, char reg_table[][MAX_TOKEN_SIZE], Equation*** eqs) {
  if (debug) printf("\nDebug: Making array/tree...\n");
  
  // allocating equation array
  *eqs = (Equation**) malloc(n_lines * sizeof(Equation*));
  for (int i = 0; i < n_lines; ++i) {
    (*eqs)[i] = alloc_eq(lines[i]);
    if (debug) printf("Debug: New equation allocated at %p\n", (*eqs)[i]);
  }

  // traversing through lines
  Equation* curr_eq = NULL;    // current equation
  Expression* curr_ex = NULL;  // current expression
  for (int i = 0; i < n_lines; ++i) {
    if (debug) printf("\n\n\nDebug: line %d: %p: %s\n", i, lines[i], lines[i]);
    char* curr_line = lines[i];
    char tok_line[MAX_STRING_SIZE];
    strcpy(tok_line, curr_line);

    // traversing through tokens
    int s = 0; // current state
    char* tok = strtok(tok_line, " ;"); // remove spaces and semicolons
    while (tok != NULL) {
      if (debug) {
        printf("\nDebug: s: %d\n", s);
        printf("Debug: tok: %s\n", tok);
      }

      switch(s) {
        // new equation
        case 0:
          if (debug) printf("Debug: New Equation:\n");

          // getting equation
          curr_eq = (*eqs)[i];

          // getting first operand/register
          get_reg(reg_table, tok, curr_eq->rd);

          s++;
          break;

        // determining li or op
        case 1:
          tok = nexttok();
          // if tok is a number, then this line is a li (constants cannot be on the left side of operations)
          if (isnumeric(tok)) { 
            if (debug) printf("Debug: li operation\n");
            strcpy(curr_eq->im, tok); // saving to equation struct
          }

          // new expression
          else { 
            // allocating new expression
            if (debug) printf("Debug: New Expression:\n");
            curr_ex = alloc_ex(); 
            if (debug) printf("       New expression allocated at %p\n", curr_ex);

            // getting first operand/register
            get_reg(reg_table, tok, curr_ex->rs);

            s = 2;
          } 
          break;

        // complete creating expression
        case 2:
          if (debug) printf("Debug: other operation\n");

          // connecting to equation
          if (curr_eq->ex == NULL) {
            curr_eq->ex = curr_ex;
          }

          // extend expression
          else {
            // allocating new expression
            if (debug) printf("Debug: New Expression:\n");
            Expression* new_ex = alloc_ex(); 
            if (debug) printf("       New expression allocated at %p\n", curr_ex);

            // reshaping equation structure
            new_ex->left_ex = curr_ex;
            curr_ex = new_ex;
            curr_eq->ex = curr_ex;
          }

          // saving operation and second operand
          curr_ex->op = tok[0]; // saving op
          tok = nexttok();
          // second operand (rt)
          if (isnumeric(tok)) { // constant operand
            strcpy(curr_ex->rt, tok); 
            curr_ex->con = true;
            if (atoi(tok) < 0)
              curr_ex->neg = true;
          } else { // only register operands
            get_reg(reg_table, tok, curr_ex->rt);
          }

          break;
      }

      tok = strtok(NULL, " ;"); // next token

      if (debug) {
        printf("\n");
        print_reg_table(reg_table);
        printf("Debug: curr_eq: %p\n", curr_eq);
        print_eq(curr_eq);
        printf("Debug: curr_ex: %p\n", curr_ex);
        print_ex(curr_ex, "");
      }
    }
  }

  // end of function
  if (debug)
    printf("\nDebug: %d lines of C read, tree built!\n", n_lines);
}

// -----------------------------------------------------------------------------------------------------------------------------
// compiling

// increase MIPS_code AND n_MIPS by n lines
char** realloc_MIPS_code(char** MIPS_code, int* n_MIPS, const int n) {
  if (debug) printf("Debug: Increasing MIPS code array by %d lines...\n", n);
  int old_size = *n_MIPS;
  *n_MIPS += n;
  MIPS_code = (char**) realloc(MIPS_code, *n_MIPS * sizeof(char*));
  for (int i = old_size; i < *n_MIPS; ++i)
    MIPS_code[i] = (char*) malloc(MAX_STRING_SIZE * sizeof(char));
  if (debug) printf("       Realloc succesful!\n");
  return MIPS_code;
}

// ---------------------------------------------------------------------------
// addition
void MIPS_add(Equation* curr_eq, Expression* curr_ex, char*** MIPS_code, int* n_MIPS, int* curr_t) {
  if (debug) {
    printf("Debug: Adding:\n");
    printf("  con: %d\n", curr_ex->con);
    printf("  neg: %d\n", curr_ex->neg);
    printf("  curr_t: %d\n", *curr_t);
  }

  // expanding array
  *MIPS_code = realloc_MIPS_code(*MIPS_code, n_MIPS, 1);
  char* curr_line = (*MIPS_code)[*n_MIPS-1];

  // determining rd
  int old_t = *curr_t;
  char rd[] = "$tx";
  if (curr_eq->ex == curr_ex)
    strcpy(rd, curr_eq->rd);
  else
    rd[2] = ('0' + ++(*curr_t));

  // determining rs
  char rs[] = "$tx";
  if (curr_ex->left_ex == NULL)
    strcpy(rs, curr_ex->rs);
  else
    rs[2] = ('0' + old_t);

  // writing the instruction
  if (!(curr_ex->con)) // adding with registers
    strcpy(curr_line, "add ");
  else // adding with constant
    strcpy(curr_line, "addi ");
  // rest of the instruction
  strcat(curr_line, rd);
  strcat(curr_line, ",");
  strcat(curr_line, rs);
  strcat(curr_line, ",");
  strcat(curr_line, curr_ex->rt);
}

// ---------------------------------------------------------------------------
// subtraction
void MIPS_sub(Equation* curr_eq, Expression* curr_ex, char*** MIPS_code, int* n_MIPS, int* curr_t) {
  if (debug) {
    printf("Debug: Subtracting:\n");
    printf("  con: %d\n", curr_ex->con);
    printf("  neg: %d\n", curr_ex->neg);
    printf("  curr_t: %d\n", *curr_t);
  }

  // registers only
  if (!(curr_ex->con)) {
    // expanding array
    *MIPS_code = realloc_MIPS_code(*MIPS_code, n_MIPS, 1);
    char* curr_line = (*MIPS_code)[*n_MIPS-1];

    // determining rd
    int old_t = *curr_t;
    char rd[] = "$tx";
    if (curr_eq->ex == curr_ex)
      strcpy(rd, curr_eq->rd);
    else
      rd[2] = ('0' + ++(*curr_t));

    // determining rs
    char rs[] = "$tx";
    if (curr_ex->left_ex == NULL)
      strcpy(rs, curr_ex->rs);
    else
      rs[2] = ('0' + old_t);

    strcpy(curr_line, "sub ");
    strcat(curr_line, rd);
    strcat(curr_line, ",");
    strcat(curr_line, rs);
    strcat(curr_line, ",");
    strcat(curr_line, curr_ex->rt);
  }

  // with constant
  else {
    // negate
    if (curr_ex->rt[0] == '-') { // already negative
      char* tok = strtok(curr_ex->rt, "-");
      strcpy(curr_ex->rt, tok);
    } else {
      char temp[MAX_TOKEN_SIZE];
      strcpy(temp, curr_ex->rt);
      strcpy(curr_ex->rt, "-");
      strcat(curr_ex->rt, temp);
    }

    // send to add
    curr_ex->op = '+';
    MIPS_add(curr_eq, curr_ex, MIPS_code, n_MIPS, curr_t);
  }
}

// ---------------------------------------------------------------------------
// multiplication

// prepping for multiplication by a constant rt by first calculating the bit shifts needed
int MIPS_mul_prep(const int rt, bool* shifts) {
  if (debug) printf("Debug: Multiplying by constant %d:\n", rt);

  int n_shifts = 0; // number of shift operations needed
  int rem = abs(rt);
  for (int i = 31; i >= 1; --i) { // multiply by 1 is not necessary
    int new_rem = rem - pow(2,i);
    if (new_rem >= 0) {
      n_shifts++;
      shifts[i] = true;
      rem = new_rem;
    }
  }

  if (debug) {
    printf("       Shifts needed:");
    for (int i = 31; i >= 0; --i) {
      if (shifts[i])
        printf(" %d", i);
    }
    printf("\n");
  }
  return n_shifts;
}

void MIPS_mul(Equation* curr_eq, Expression* curr_ex, char*** MIPS_code, int* n_MIPS, int* curr_t) {
  if (debug) {
    printf("Debug: Multiplying:\n");
    printf("  con: %d\n", curr_ex->con);
    printf("  neg: %d\n", curr_ex->neg);
    printf("  curr_t: %d\n", *curr_t);
  }

  // registers only
  if (!(curr_ex->con)) {
    // expanding array
    *MIPS_code = realloc_MIPS_code(*MIPS_code, n_MIPS, 2);
    char* line1 = (*MIPS_code)[*n_MIPS-2];
    char* line2 = (*MIPS_code)[*n_MIPS-1];

    // determining rd
    int old_t = *curr_t;
    char rd[] = "$tx";
    if (curr_eq->ex == curr_ex)
      strcpy(rd, curr_eq->rd);
    else
      rd[2] = ('0' + ++(*curr_t));

    // determining rs
    char rs[] = "$tx";
    if (curr_ex->left_ex == NULL)
      strcpy(rs, curr_ex->rs);
    else
      rs[2] = ('0' + old_t);

    // line 1
    strcpy(line1, "mult ");
    strcat(line1, rs);
    strcat(line1, ",");
    strcat(line1, curr_ex->rt);

    // line 2
    strcpy(line2, "mflo ");
    strcat(line2, rd);
  }

  // with constant
  else {
    // 0
    if (strcmp(curr_ex->rt, "0") == 0) {
      // expanding array
      *MIPS_code = realloc_MIPS_code(*MIPS_code, n_MIPS, 1);
      char* curr_line = (*MIPS_code)[*n_MIPS-1];

      // determining rd
      char rd[] = "$tx";
      if (curr_eq->ex == curr_ex)
        strcpy(rd, curr_eq->rd);
      else
        rd[2] = ('0' + ++(*curr_t));

      // writing instruction
      strcpy(curr_line, "li ");
      strcat(curr_line, rd);
      strcat(curr_line, ",0");
    }

    // 1
    else if (strcmp(curr_ex->rt, "1") == 0) {
      // expanding array
      *MIPS_code = realloc_MIPS_code(*MIPS_code, n_MIPS, 2);
      char* line1 = (*MIPS_code)[*n_MIPS-2];
      char* line2 = (*MIPS_code)[*n_MIPS-1];

      // determining rds
      int old_t = *curr_t;
      char rd1[] = "$tx";
      rd1[2] = ('0' + ++(*curr_t));
      char rd2[] = "$tx";
      if (curr_eq->ex == curr_ex)
        strcpy(rd2, curr_eq->rd);
      else
        rd2[2] = ('0' + ++(*curr_t));

      // determining rs
      char rs[] = "$tx";
      if (curr_ex->left_ex == NULL)
        strcpy(rs, curr_ex->rs);
      else
        rs[2] = ('0' + old_t);

      // line 1
      strcpy(line1, "move ");
      strcat(line1, rd1);
      strcat(line1, ",");
      strcat(line1, rs);

      // line 2
      strcpy(line2, "move ");
      strcat(line2, rd2);
      strcat(line2, ",");
      strcat(line2, rd1);
    }

    // -1
    else if (strcmp(curr_ex->rt, "-1") == 0) {
      // expanding array
      *MIPS_code = realloc_MIPS_code(*MIPS_code, n_MIPS, 2);
      char* line1 = (*MIPS_code)[*n_MIPS-2];
      char* line2 = (*MIPS_code)[*n_MIPS-1];

      // determining rds
      int old_t = *curr_t;
      char rd1[] = "$tx";
      rd1[2] = ('0' + ++(*curr_t));
      char rd2[] = "$tx";
      if (curr_eq->ex == curr_ex)
        strcpy(rd2, curr_eq->rd);
      else
        rd2[2] = ('0' + ++(*curr_t));

      // determining rs
      char rs[] = "$tx";
      if (curr_ex->left_ex == NULL)
        strcpy(rs, curr_ex->rs);
      else
        rs[2] = ('0' + old_t);

      // line 1
      strcpy(line1, "move ");
      strcat(line1, rd1);
      strcat(line1, ",");
      strcat(line1, rs);

      // line 2
      strcpy(line2, "sub ");
      strcat(line2, rd2);
      strcat(line2, ",$zero,");
      strcat(line2, rd1);
    }

    // other constants
    else {
      // calculating # of shift operations
      bool shifts[32];
      for (int i = 0; i < 32; ++i)
        shifts[i] = false;
      int n_shifts = MIPS_mul_prep(atoi(curr_ex->rt), shifts);

      // expanding array
      int d_size = n_shifts * 2 + 2; // change in code size (2 for each shift/add and another 2 for the last add and move)
      *MIPS_code = realloc_MIPS_code(*MIPS_code, n_MIPS, d_size);
      int i_line = *n_MIPS - d_size; // current line index

      // determining rds
      int old_t = *curr_t;
      char rd1[] = "$tx";
      rd1[2] = ('0' + ++(*curr_t));
      char rd2[] = "$tx";
      rd2[2] = ('0' + ++(*curr_t));
      char rd3[] = "$tx";
      if (curr_eq->ex == curr_ex)
        strcpy(rd3, curr_eq->rd);
      else
        rd3[2] = ('0' + ++(*curr_t));

      // determining rs
      char rs[] = "$tx";
      if (curr_ex->left_ex == NULL)
        strcpy(rs, curr_ex->rs);
      else
        rs[2] = ('0' + old_t);

      // debug: checking
      if (debug && verbose) {
        printf("Debug: curr_t: %d\n", *curr_t);
        printf("Debug: rd1: %s\n", rd1);
        printf("       rd2: %s\n", rd2);
        printf("       rd3: %s\n", rd3);
        printf("        rs: %s\n", rs);
      }

      // generating instructions
      char* curr_line = NULL;
      bool first = true;
      for (int i = 31; i >= 1; --i) {
        if (debug) printf("Debug: %d: %d\n", i, shifts[i]);
        if (shifts[i]) {
          // sll
          curr_line = (*MIPS_code)[i_line];
          strcpy(curr_line, "sll ");
          strcat(curr_line, rd1);
          strcat(curr_line, ",");
          strcat(curr_line, rs);
          strcat(curr_line, ",");
          char num[3];
          sprintf(num, "%d", i);
          strcat(curr_line, num);

          // add
          curr_line = (*MIPS_code)[++i_line];
          if (first) { // move if first
            strcpy(curr_line, "move ");
            strcat(curr_line, rd2);
            strcat(curr_line, ",");
            strcat(curr_line, rd1);
            first = false;
          }
          else {
            strcpy(curr_line, "add ");
            strcat(curr_line, rd2);
            strcat(curr_line, ",");
            strcat(curr_line, rd2);
            strcat(curr_line, ",");
            strcat(curr_line, rd1);
          }

          i_line++;
        }
      }
      // last two lines
      curr_line = (*MIPS_code)[i_line];
      strcpy(curr_line, "add ");
      strcat(curr_line, rd2);
      strcat(curr_line, ",");
      strcat(curr_line, rd2);
      strcat(curr_line, ",");
      strcat(curr_line, rs);
      curr_line = (*MIPS_code)[++i_line];
      if (!(curr_ex->neg)) { // positive constant
        strcpy(curr_line, "move ");
        strcat(curr_line, rd3);
        strcat(curr_line, ",");
        strcat(curr_line, rd2);
      } else {               // negative constant
        strcpy(curr_line, "sub ");
        strcat(curr_line, rd3);
        strcat(curr_line, ",$zero,");
        strcat(curr_line, rd2);
      }
    }
  }
}

// ---------------------------------------------------------------------------
// division

// checks if a 32 bit number is a power of 2
bool power_of_2(int n, int* n_bit) {
  n = abs(n);
  for (int i = 0; i < 32; ++i) {
    if (n - pow(2,i) == 0) {
      *n_bit = i;
      return true;
    }
  }
  return false;
}

void MIPS_div(Equation* curr_eq, Expression* curr_ex, char*** MIPS_code, int* n_MIPS, int* curr_t, int* curr_L) {
  if (debug) {
    printf("Debug: Dividing:\n");
    printf("  con: %d\n", curr_ex->con);
    printf("  neg: %d\n", curr_ex->neg);
    printf("  curr_t: %d\n", *curr_t);
  }

  // registers only
  if (!(curr_ex->con)) {
    // expanding array
    *MIPS_code = realloc_MIPS_code(*MIPS_code, n_MIPS, 2);
    char* line1 = (*MIPS_code)[*n_MIPS-2];
    char* line2 = (*MIPS_code)[*n_MIPS-1];

    // determining rd
    int old_t = *curr_t;
    char rd[] = "$tx";
    if (curr_eq->ex == curr_ex)
      strcpy(rd, curr_eq->rd);
    else
      rd[2] = ('0' + ++(*curr_t));

    // determining rs
    char rs[] = "$tx";
    if (curr_ex->left_ex == NULL)
      strcpy(rs, curr_ex->rs);
    else
      rs[2] = ('0' + old_t);

    // line 1
    strcpy(line1, "div ");
    strcat(line1, rs);
    strcat(line1, ",");
    strcat(line1, curr_ex->rt);

    // line 2
    strcpy(line2, "mflo ");
    strcat(line2, rd);
  }

  // with constant
  else {
    // 1
    if (strcmp(curr_ex->rt, "1") == 0) {
      // expanding array
      *MIPS_code = realloc_MIPS_code(*MIPS_code, n_MIPS, 1);
      char* curr_line = (*MIPS_code)[*n_MIPS-1];

      // determining rd
      int old_t = *curr_t;
      char rd[] = "$tx";
      if (curr_eq->ex == curr_ex)
        strcpy(rd, curr_eq->rd);
      else
        rd[2] = ('0' + ++(*curr_t));

      // determining rs
      char rs[] = "$tx";
      if (curr_ex->left_ex == NULL)
        strcpy(rs, curr_ex->rs);
      else
        rs[2] = ('0' + old_t);

      // writing instruction
      strcpy(curr_line, "move ");
      strcat(curr_line, rd);
      strcat(curr_line, ",");
      strcat(curr_line, rs);
    }

    // -1
    else if (strcmp(curr_ex->rt, "-1") == 0) {
      // expanding array
      *MIPS_code = realloc_MIPS_code(*MIPS_code, n_MIPS, 1);
      char* curr_line = (*MIPS_code)[*n_MIPS-1];

      // determining rd
      int old_t = *curr_t;
      char rd[] = "$tx";
      if (curr_eq->ex == curr_ex)
        strcpy(rd, curr_eq->rd);
      else
        rd[2] = ('0' + ++(*curr_t));

      // determining rs
      char rs[] = "$tx";
      if (curr_ex->left_ex == NULL)
        strcpy(rs, curr_ex->rs);
      else
        rs[2] = ('0' + old_t);

      // writing instruction
      strcpy(curr_line, "sub ");
      strcat(curr_line, rd);
      strcat(curr_line, ",$zero,");
      strcat(curr_line, rs);
    }

    // other constants
    else {
      // checking if rt is a power of 2
      int i_bit = -1;
      if (power_of_2(atoi(curr_ex->rt), &i_bit)) {
        // check whether first operand is negative
        int d_size = 8;
        if (curr_ex->rt[0] == '-')
          d_size++; // additional sub instruction

        // expanding array
        *MIPS_code = realloc_MIPS_code(*MIPS_code, n_MIPS, d_size);
        
        // determining rd
        int old_t = *curr_t;
        char rd1[] = "$tx";
        if (curr_eq->ex == curr_ex) // this expression is at the top, use equation rd
          strcpy(rd1, curr_eq->rd);
        else
          rd1[2] = ('0' + ++(*curr_t));
        char rd2[] = "$tx";
        rd2[2] = ('0' + ++(*curr_t));

        // determining rs
        char rs[] = "$tx";
        if (curr_ex->left_ex == NULL)
          strcpy(rs, curr_ex->rs);
        else
          rs[2] = ('0' + old_t);

        // determining labels
        char Lx[] = "Lx";
        Lx[1] = ('0' + ++(*curr_L));
        char Ly[] = "Ly";
        Ly[1] = ('0' + ++(*curr_L));

        // writing instructions
        int i_line = *n_MIPS-d_size;
        
        // bltz rs,Lx
        char* curr_line = (*MIPS_code)[i_line];
        strcpy(curr_line, "bltz ");
        strcat(curr_line, rs);
        strcat(curr_line, ",");
        strcat(curr_line, Lx);

        // srl rd1,rs,i_bit
        curr_line = (*MIPS_code)[++i_line];
        strcpy(curr_line, "srl ");
        strcat(curr_line, rd1);
        strcat(curr_line, ",");
        strcat(curr_line, rs);
        strcat(curr_line, ",");
        char num[3];
        sprintf(num, "%d", i_bit);
        strcat(curr_line, num);

        // sub rd1,$zero,rd1 (if constant is negative)
        if (curr_ex->neg) {
          curr_line = (*MIPS_code)[++i_line];
          strcpy(curr_line, "sub ");
          strcat(curr_line, rd1);
          strcat(curr_line, ",$zero,");
          strcat(curr_line, rd1);
        }

        // j Ly
        curr_line = (*MIPS_code)[++i_line];
        strcpy(curr_line, "j ");
        strcat(curr_line, Ly);

        // Lx:
        curr_line = (*MIPS_code)[++i_line];
        strcpy(curr_line, Lx);
        strcat(curr_line, ":");

        // li rd2,rt
        curr_line = (*MIPS_code)[++i_line];
        strcpy(curr_line, "li ");
        strcat(curr_line, rd2);
        strcat(curr_line, ",");
        strcat(curr_line, curr_ex->rt);

        // div rs,rd2
        curr_line = (*MIPS_code)[++i_line];
        strcpy(curr_line, "div ");
        strcat(curr_line, rs);
        strcat(curr_line, ",");
        strcat(curr_line, rd2);

        // mflo rd1
        curr_line = (*MIPS_code)[++i_line];
        strcpy(curr_line, "mflo ");
        strcat(curr_line, rd1);

        // Ly:
        curr_line = (*MIPS_code)[++i_line];
        strcpy(curr_line, Ly);
        strcat(curr_line, ":");
      }

      // not a power of 2
      else {
        // expanding array
        *MIPS_code = realloc_MIPS_code(*MIPS_code, n_MIPS, 3);
        char* line1 = (*MIPS_code)[*n_MIPS-3];
        char* line2 = (*MIPS_code)[*n_MIPS-2];
        char* line3 = (*MIPS_code)[*n_MIPS-1];

        // determining rd
        int old_t = *curr_t;
        char rd1[] = "$tx";
        rd1[2] = ('0' + ++(*curr_t));
        char rd2[] = "$tx";
        if (curr_eq->ex == curr_ex)
          strcpy(rd2, curr_eq->rd);
        else
          rd2[2] = ('0' + ++(*curr_t));

        // determining rs
        char rs[] = "$tx";
        if (curr_ex->left_ex == NULL)
          strcpy(rs, curr_ex->rs);
        else
          rs[2] = ('0' + old_t);

        // line 1
        strcpy(line1, "li ");
        strcat(line1, rd1);
        strcat(line1, ",");
        strcat(line1, curr_ex->rt);

        // line 2
        strcpy(line2, "div ");
        strcat(line2, rs);
        strcat(line2, ",");
        strcat(line2, rd1);

        // line 3
        strcpy(line3, "mflo ");
        strcat(line3, rd2);
      }
    }
  }
}

// ---------------------------------------------------------------------------
// modulo
void MIPS_mod(Equation* curr_eq, Expression* curr_ex, char*** MIPS_code, int* n_MIPS, int* curr_t) {
  if (debug) {
    printf("Debug: Modulo:\n");
    printf("  con: %d\n", curr_ex->con);
    printf("  neg: %d\n", curr_ex->neg);
    printf("  curr_t: %d\n", *curr_t);
  }

  // registers only
  if (!(curr_ex->con)) {
    // expanding array
    *MIPS_code = realloc_MIPS_code(*MIPS_code, n_MIPS, 2);
    char* line1 = (*MIPS_code)[*n_MIPS-2];
    char* line2 = (*MIPS_code)[*n_MIPS-1];

    // determining rd
    int old_t = *curr_t;
    char rd[] = "$tx";
    if (curr_eq->ex == curr_ex)
      strcpy(rd, curr_eq->rd);
    else
      rd[2] = ('0' + ++(*curr_t));

    // determining rs
    char rs[] = "$tx";
    if (curr_ex->left_ex == NULL)
      strcpy(rs, curr_ex->rs);
    else
      rs[2] = ('0' + old_t);

    // line 1
    strcpy(line1, "div ");
    strcat(line1, rs);
    strcat(line1, ",");
    strcat(line1, curr_ex->rt);

    // line 2
    strcpy(line2, "mfhi ");
    strcat(line2, rd);
  }

  // with constant
  else {
    // expanding array
    *MIPS_code = realloc_MIPS_code(*MIPS_code, n_MIPS, 3);
    char* line1 = (*MIPS_code)[*n_MIPS-3];
    char* line2 = (*MIPS_code)[*n_MIPS-2];
    char* line3 = (*MIPS_code)[*n_MIPS-1];

    // extra t register for storing constant
    int old_t = *curr_t;
    char rd1[] = "$tx";
    rd1[2] = ('0' + ++(*curr_t));

    // line 1
    strcpy(line1, "li ");
    strcat(line1, rd1);
    strcat(line1, ",");
    strcat(line1, curr_ex->rt);

    // determining rd
    char rd2[] = "$tx";
    if (curr_eq->ex == curr_ex)
      strcpy(rd2, curr_eq->rd);
    else
      rd2[2] = ('0' + ++(*curr_t));

    // determining rs
    char rs[] = "$tx";
    if (curr_ex->left_ex == NULL)
      strcpy(rs, curr_ex->rs);
    else
      rs[2] = ('0' + old_t);

    // line 2
    strcpy(line2, "div ");
    strcat(line2, rs);
    strcat(line2, ",");
    strcat(line2, rd1);

    // line 3
    strcpy(line3, "mfhi ");
    strcat(line3, rd2);
  }
}

// ---------------------------------------------------------------------------
// tree part of compiling
void exs_to_MIPS(Equation* curr_eq, Expression* curr_ex, char*** MIPS_code, int* n_MIPS, int* curr_t, int* curr_L) {
  // reach bottom of tree first
  if (curr_ex->left_ex != NULL) {
    exs_to_MIPS(curr_eq, curr_ex->left_ex, MIPS_code, n_MIPS, curr_t, curr_L);
  }

  // bottom of tree / back up
  switch (curr_ex->op){
    case '+':
      MIPS_add(curr_eq, curr_ex, MIPS_code, n_MIPS, curr_t);
      break;
  
    case '-':
      MIPS_sub(curr_eq, curr_ex, MIPS_code, n_MIPS, curr_t);
      break;

    case '*':
      MIPS_mul(curr_eq, curr_ex, MIPS_code, n_MIPS, curr_t);
      break;

    case '/':
      MIPS_div(curr_eq, curr_ex, MIPS_code, n_MIPS, curr_t, curr_L);
      break;

    case '%':
      MIPS_mod(curr_eq, curr_ex, MIPS_code, n_MIPS, curr_t);
      break;
  }
}

// converting data struct into lines of MIPS code  
void eqs_to_MIPS(Equation** eqs, const int n_eqs, char*** MIPS_code, int* n_MIPS) {
  if (debug)
    printf("\nDebug: Compiling MIPS code into array at %p...\n", MIPS_code);

  *MIPS_code = (char**) malloc(0); // empty array to start
  int curr_t = -1; // counter for t registers (not reset for every line of C code?)
  int curr_L = -1; // counter for labels
  for (int i = 0; i < n_eqs; ++i) {
    // getting equation and line
    Equation* curr_eq = eqs[i];
    if (debug) printf("\n\n\nDebug: curr_eq: %p: %s\n", curr_eq, curr_eq->og);

    // ---------------------------------------------------------------------------
    // expanding code line array
    *MIPS_code = realloc_MIPS_code(*MIPS_code, n_MIPS, 1);
    char* curr_line = (*MIPS_code)[*n_MIPS-1];

    // comment original C code
    strcpy(curr_line, "# ");
    strcat(curr_line, curr_eq->og);

    // ---------------------------------------------------------------------------
    // simple li
    if (curr_eq->ex == NULL) {
      if (debug) printf("Debug: li operation\n");
      
      // expanding MIPS code array
      *MIPS_code = realloc_MIPS_code(*MIPS_code, n_MIPS, 1);
      curr_line = (*MIPS_code)[*n_MIPS-1];

      // generating MIPS code
      strcpy(curr_line, "li ");
      strcat(curr_line, curr_eq->rd);
      strcat(curr_line, ",");
      strcat(curr_line, curr_eq->im);
    }
  
    // ---------------------------------------------------------------------------
    // more complicated op
    else
      exs_to_MIPS(curr_eq, curr_eq->ex, MIPS_code, n_MIPS, &curr_t, &curr_L); // creating intermediate instructions 

    // ---------------------------------------------------------------------------
    // debugging
    if (debug) {
      printf("\nDebug: MIPS code (including comments):\n");
      for (int i = 0; i < *n_MIPS; ++i)
        printf("  %d:\t%s\n", i, (*MIPS_code)[i]);
    }
  }
  if (debug)
    printf("\nDebug: Compiling completed!\n");
}

// -----------------------------------------------------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
  // getting debug value
  if (argc >= 3) {
    int cmp = strcmp(argv[2], "1");
    // printf("%d\n", cmp); // TOREMOVE
    if (cmp == 0)
      debug = true;
    if (argc >= 4) {
      if (strcmp(argv[3], "1") == 0)
        verbose = true;
    }
  }
  // checking inputs
  if (debug) {
    printf("Debug: argc: %d\n", argc);
    for (int i = 0; i < argc; ++i)
      printf("Debug: argv[%d]: %s\n", i, argv[i]);
    printf("\n");
  }

  // file reading
  FILE* file = get_file(argv[1]);
  char** lines = NULL; // string array for storing lines
  int n_lines = 0;
  parse_file(file, &lines, &n_lines); // parse file into lines stored in string array
  if (debug) printf("\nDebug: lines: %p\n", lines);

  // parsing and tree making
  char reg_table[][MAX_TOKEN_SIZE] = {"(empty)", "(empty)", "(empty)", "(empty)", "(empty)", "(empty)", "(empty)", "(empty)"}; // register table for storing variable names
  Equation** eqs = NULL; // equation array for storing equations and expressions
  make_tree(lines, n_lines, reg_table, &eqs); // convert lines into array-tree hybrid structure
  // freeing lines array
  for (int i = 0; i < n_lines; ++i) free(lines[i]);
  free(lines);
  
  if (debug) {
    printf("\nDebug: eqs: %p\n", eqs);
    print_tree(eqs, n_lines);
  }

  // code compiling
  char** MIPS_code = NULL;
  int n_MIPS = 0;  // lines of MIPS code (including comments)
  eqs_to_MIPS(eqs, n_lines, &MIPS_code, &n_MIPS); // compiling function
  // freeing equation/expression array/tree
  for (int i = 0; i < n_lines; ++i) free_eq(eqs[i]);
  free(eqs);
  
  // outputting
  for (int i = 0; i < n_MIPS; ++i)
    printf("%s\n", MIPS_code[i]);
  
  // memory management / cleaning up
  if (debug)
    printf("\nDebug: Freeing memory, cleaning up...\n");
  for (int i = 0; i < n_MIPS; ++i)
    free(MIPS_code[i]);
  free(MIPS_code);
  if (debug) printf("Debug: Process completed!\n");
  return 0;	// successful process
}