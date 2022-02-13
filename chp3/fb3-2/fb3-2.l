/* 高级计算器的词法分析器 */

/*******************************************第一部分****************************************************************/

/* 识别计算器的记号,是词法分析器的选项，用于影响生成的词法分析器的属性和行为 */
%option noyywrap nodefault yylineno

/* 此部分为c语言声明部分，用于声明全局变量和函数 */
%{
#include "fb3-2.h"
#include "fb3-2.tab.h"  
%}

/* 此部分定义正则表达式的名字，可以在规则段(第二部分)使用 */
/* 浮点指数部分 */
EXP ([Ee][-+]?[0-9]+)  

/*******************************************第二部分****************************************************************/
/* 第二部分为规则段，每一行都是一条规则，每一条规则由匹配模式和事件组成。每当一个模式被匹配到，后面的事件被执行 */
/* 规则部分也有优先级，上面的规则优先于下面的规则进行匹配
/* 优先匹配操作符以及关键字  */
%%
 /* single character ops */
"+" |
"-" |
"*" |
"/" |
"=" |
"|" |
"," |
";" |
"(" |
")"     { return yytext[0]; /* 返回匹配到的第一个字符 */ }

 /* 比较操作符,设置yylval的值，所有的返回值都是CMP记号 */
 /* yylval中包含的类型都在.y文件中的union类型中定义了，返回值是bison中的一个token */
">"     { yylval.fn = 1; return CMP; }
"<"     { yylval.fn = 2; return CMP; }
"<>"    { yylval.fn = 3; return CMP; }
"=="    { yylval.fn = 4; return CMP; }
">="    { yylval.fn = 5; return CMP; }
"<="    { yylval.fn = 6; return CMP; }

 /* 关键字 */
"if"    { return IF; }
"then"  { return THEN; }
"else"  { return ELSE; }
"while" { return WHILE; }
"do"    { return DO; }
"let"   { return LET;}

 /* 内置函数，设置yylval的值，所有的返回值都是FUNC记号 */
"sqrt" { yylval.fn = B_sqrt; return FUNC; }
"exp"  { yylval.fn = B_exp; return FUNC; }
"log"   { yylval.fn = B_log; return FUNC; }
"print" { yylval.fn = B_print; return FUNC; }

 /* 之后开始匹配名字 */
[a-zA-Z][a-zA-Z0-9]* { yylval.s = lookup(yytext); return NAME; /* 返回token */ }

 /* 匹配数字,EXP为第一部分定义的正则表达式 */
[0-9]+"."[0-9]*{EXP}? |
"."?[0-9]+{EXP}? {
  yylval.d = atof(yytext);
  return NUMBER;
}
 
"//".*  /* 忽略注释 */
[ \t]  /* 忽略空白字符 */

 /* 忽略续航符，为一个 \ 和一个 \n */
\\\n { printf("c> "); }

\n { return EOL; }

. {yyerror("Mystery character %c\n", *yytext); }

%%
/*******************************************第三部分****************************************************************/
/* 第三部分为用户自定义的过程 */

