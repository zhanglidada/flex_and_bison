/* 词法分析器，识别计算器的记号 */
%option noyywrap nodefault yylineno
%{
#include "fb3-1.h"
#include "fb3-1.tab.h"  
%}

/* 浮点指数部分 */
EXP ([Ee][-+]?[0-9]+)

%%
"+" |
"-" |
"*" |
"/" |
"(" |
")" { return yytext[0]; /* 将自身作为记号返回 */ }

[0-9]+"."[0-9]*{EXP}? |
"."?[0-9]+{EXP}? { yylval.d = atof(yytext); return NUMBER; }

\n { return EOL; }

"//".*
[ \t] { /*忽略空白字符*/ }

. { yyerror("Mystery character %c\n", *yytext); }
%%