/* 计算器的最简单版本 */
%{
#include <stdio.h>
%}

/* declare tokens */
%token NUMBER
%token ADD SUB MUL DIV ABS
%token EOL

%%
/* 从输入的开头进行匹配，EOL代表一个表达式的结束 */
calclist:  /* 空规则 */
  | calclist exp EOL { printf("= %d\n", $2); }
  ;

exp: factor default $$ = $1
  | exp ADD factor { $$ = $1 + $3; }
  | exp SUB factor { $$ = $1 - $3; }
  ;

factor: term default $$ = $1
  | factor MUL term { $$ = $1 * $3; }
  | factor DIV term { $$ = $1 / $3; }
  ;

term: NUMBER default $$ = $1
  | ABS term { $$ = $2 >= 0? $2 : -$2; }
  ;
%%


int main(int argc, char** argv)
  yyparse();
}
void yyerror(char* s)
{
  fprintf(stderr, "error: %s\n", s);
}