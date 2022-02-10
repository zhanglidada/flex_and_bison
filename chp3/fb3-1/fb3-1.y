/* 基于抽象语法树的计算器 */
%{
#include <stdio.h>
#include <stdlib.h>
#include "fb3-1.h"

%}

/* 
  使用union声明语法分析器中符号值的类型
  类似c语言的union类型
 */
%union {
  struct ast* a;
  double d;
}

/* 声明记号,并告诉每种语法符号使用的值类型 */
%token <d> NUMBER
%token EOL

/* 赋值操作 */
%type <a> exp factor term


%%
/* 语法树的起始 */
calclist: /* do nothing */
| calclist exp EOL {
    printf("= %4.4g\n", eval($2));
    treefree($2);
    printf("> ");
  }
| calclist EOL { 
    printf("> "); /* blank line or a comment*/ 
  }
;

/* 语法树中的表达式，中间节点 */
exp: factor
| exp '+' factor { $$ = newast('+', $1, $3); }
| exp '-' factor { $$ = newast('-', $1, $3); }
;

/* 表达式的因子，中间节点 */
factor: term
| factor '*' term { $$ = newast('*', $1, $3); }
| factor '/' term { $$ = newast('/', $1, $3); }
;

/* 语法树的叶子节点，单个的词语 */
term: NUMBER { $$ = newnum($1);}
| '|' term { $$ = newast('|', $2, NULL); /* 创建新的指针节点 */ }
| '(' exp ')' { $$ = $2; /* 括号中的是一个表达式 */ }
| '-' term { $$ = newast('M', $2, NULL); /* 单目负号操作符创建类型为M的节点 */}
;
%%