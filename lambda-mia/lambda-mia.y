%{
#include <stdio.h>
#include <stdlib.h>
#include "lambda_ast.h"  // Assuming this header defines lambda structures and creation functions.


// Global program instance to collect statements
struct program *current_program;

void yyerror(const char *s);
int yylex(void);
extern int yylineno;  // Use this to access the current line number
extern char *yytext;  // The text of the current token
%}

%union {
    int ival;
    float fval;
    char cval;
    char *sval;
    struct lambda *lval;
    struct binding *bval;
    struct statement *stmval;
}

%token <ival> INTEGER
%token <fval> FLOAT
%token <cval> CHARACTER
%token <sval> ID_VAR ID_LAMBDA_VAR
%token IF THEN ELSE LET IN
%token COLON COMMA SEMICOLON ASSIGN DOT LPAREN RPAREN LBRACK RBRACK
%token HEAD TAIL EMPTY

%type <lval> lambda_expression lambda_term 
%type <lval> if_expression let_expression list_construction
%type <lval> literal empty_list list_operation
%type <bval> lambda_declaration
%type <stmval> statement list_statement prog

%define parse.error verbose

%%

prog:
    list_statement SEMICOLON {
        generate_assembly(current_program, "output.asm");
    }
    ;
;

list_statement:
      list_statement SEMICOLON statement {
          add_statement_to_program(current_program, $3);
      }
    | statement {
          add_statement_to_program(current_program, $1);
      }
    ;

statement
    : lambda_declaration  { $$ = create_binding_statement($1); }
    | lambda_expression   { $$ = create_lambda_statement($1); }
    ;

lambda_declaration
    :  ID_LAMBDA_VAR ASSIGN lambda_expression { $$ = create_binding($1, $3); }
    ;

lambda_expression
    : lambda_term
    ;

lambda_term
    : '\\' ID_VAR DOT lambda_term { $$ = create_lambda($2, $4); }
    | LPAREN lambda_term lambda_term RPAREN { $$ = create_application($2, $3); }
    | if_expression               
    | let_expression              
    | list_construction  
    | empty_list         
    | list_operation
    | ID_LAMBDA_VAR               { $$ = create_lambda_var($1); }   
    | ID_VAR                      { $$ = create_var($1); }   
    | literal                     { $$ = $1; }              
    | LPAREN lambda_expression RPAREN { $$ = $2; }
    ;

if_expression
    : IF lambda_expression THEN lambda_expression ELSE lambda_expression { $$ = create_if($2, $4, $6); }
    ;

let_expression
    : LET lambda_declaration IN lambda_expression { $$ = create_let($2, $4); }
    ;

literal
    : INTEGER   { $$ = create_literal_int($1); }
    ;

list_construction
    : LBRACK lambda_expression COLON lambda_expression RBRACK { $$ = create_list_construction($2, $4); }
    ;

empty_list
    : LBRACK RBRACK { $$ = create_empty_list(); }
    ;

list_operation
    : HEAD lambda_expression   { $$ = create_list_head_op($2); }
    | TAIL lambda_expression   { $$ = create_list_tail_op($2); }
    | EMPTY lambda_expression  { $$ = create_list_empty_op($2); }
    ;


%%

void yyerror(const char *s) {
    fprintf(stderr, "Syntax error at line %d near '%s': %s\n", yylineno, yytext, s);
}

int main() {
    current_program = create_program();
    if (yyparse() == 0) {
        printf("Assembly code successfully generated in 'output.asm'.\n");
    } else {
        printf("Parsing failed.\n");
    }
}
