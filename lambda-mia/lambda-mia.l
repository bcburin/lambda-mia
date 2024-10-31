%{
#include "lambda-mia.tab.h"
#include <stdio.h>  // For printf
%}

%%
"if"                  { return IF; }
"then"                { return THEN; }
"else"                { return ELSE; }
"let"                 { return LET; }
"in"                  { return IN; }
"Head"                { return HEAD; }
"Tail"                { return TAIL; }
"Empty"               { return EMPTY; }
[a-z][a-zA-Z0-9_]*    { yylval.sval =strdup(yytext); return ID_VAR; }
[A-Z][a-zA-Z0-9_]*    { yylval.sval =strdup(yytext); return ID_LAMBDA_VAR; }
[0-9]+                { yylval.ival = atoi(yytext); return INTEGER; }
"\\"                  { return '\\'; }
":"                   { return COLON; }
","                   { return COMMA; }
";"                   { return SEMICOLON; }
"="                   { return ASSIGN; }
"("                   { return LPAREN; }
")"                   { return RPAREN; }
"["                   { return LBRACK; }
"]"                   { return RBRACK; }
"--"[^\n]*            { /* Ignore comments until newline */ }
[ \t\n]+              { /* Ignore whitespaces */ }
.                     { return DOT; }  // Moved to the end
%%

int yywrap() {
    return 1;
}
