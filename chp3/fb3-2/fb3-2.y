/* 基于抽象语法树的计算器 */

%{
#include <stdio.h>
#include <stdlib.h>
#include "fb3-2.h" 
%}

 /***************************************************************************** 
* purpose: 使用union声明语法分析器中符号值的类型
* notes: 类似c语言的union类型
******************************************************************************/
%union {
  struct ast* a;
  double d;
  struct symbol* s;  /* 指定符号 */
  struct symlist* sl;  /* 指定类型为符号链表 */
  int fn;  // 指定函数
}

/***************************************************************************** 
* purpose: token是终结符,是词法分析器产生并交由语法分析器处理的词法单元
* notes:
  1.记号由记号名以及记号的属性值两部分组成
  2.token用于声明记号
  3.<...> 尖括号内部的类型为union中定义的数据类型
  4.bison中将token和union中的一个变量类型进行绑定
******************************************************************************/
%token <d> NUMBER
%token <s> NAME
/* FUNC的值确定了具体的函数 */
%token <fn> FUNC
%token EOL
/* 保留字 */
%token IF THEN ELSE WHILE DO LET


/***************************************************************************** 
* purpose: 运算符的优先级排列
* notes: 
  1.left表示左结合,right表示右结合,从上到下优先级依次递增
  2.nonassoc表示不可结合的
******************************************************************************/
%nonassoc <fn> CMP
%right '='
%left '+' '-'
%left '*' '/'
%nonassoc '|' UMINUS

/***************************************************************************** 
* purpose: 非终结符, 处理文法中各种非终结符的属性
* notes: 
  1.文法中的终结符拥有属性值, 同时非终结符也有属性值
  2.type用于给文法中的各种非终结符定义属性值的类型
******************************************************************************/
%type <a> exp stmt list explist
%type <sl> symlist

/***************************************************************************** 
* purpose: 指定文法的开始符号(非终结符)
* notes: 
  1.如果不使用%start定义文法开始符号,
  则默认在第二部分规则段中定义的第一条产生式规则的左部非终结符为开始符号。
******************************************************************************/
%start calclist
%%

/* node types 创建节点的类型
 *  + - * / |
 *  0-7 comparison ops, bit coded 04 equal, 02 less, 01 greater
 *  M unary minus
 *  L statement list
 *  I IF statement
 *  W WHILE statement
 *  N symbol ref
 *  = assignment
 *  S list of symbols
 *  F built in function call
 *  C user function call
 */

/***************************************************************************** 
* purpose: 计算器语句的语法
* notes: 
  1.语法区分语句(stmt)以及表达式(exp)
  2.语句为控制流（if/then/else或者while/do）或者一个表达式
******************************************************************************/
stmt: IF exp THEN list { 
      /* 创建流类型的数据节点，if和then都有一连串语句，以分号结尾；
      每当规则匹配到一条语句就用调用例程创建抽象语法树节点 */
      $$ = newflow('I', $2, $4, NULL);
    }
  | IF exp THEN list ELSE list { $$ = newflow('I', $2, $4, $6); }
  | WHILE exp DO list { $$ = newflow('W', $2, $4, NULL); }
;

/***************************************************************************** 
* purpose: list采用右递归的方式
* notes: 
  1.右递归即每次使用最右边的非终结符进行表达式的替换
******************************************************************************/
list: /* 空 */ { $$ = NULL; }
  | stmt ';' list {
    /* 流类型数据节点，list为一连串语句，以分号结尾 */
    if ($3 == NULL)
      $$ = $1;
    else
      $$ = newast('L', $1, $3);
  }
;

/***************************************************************************** 
* purpose: 表达式的语法(explanation)
* notes: 
  1. CMP是词法分析中的比较操作符，为终结符
******************************************************************************/
exp: exp CMP exp          { $$ = newcmp($2, $1, $3); }
   | exp '+' exp          { $$ = newast('+', $1, $3); }
   | exp '-' exp          { $$ = newast('-', $1, $3);}
   | exp '*' exp          { $$ = newast('*', $1, $3); }
   | exp '/' exp          { $$ = newast('/', $1, $3); }
   | '|' exp              { $$ = newast('|', $2, NULL); }
   | '(' exp ')'          { $$ = $2; }
   | '-' exp %prec UMINUS { $$ = newast('M', $2, NULL); }
   | NUMBER               { $$ = newnum($1); }
   | FUNC '(' explist ')' { $$ = newfunc($1, $3); }
   | NAME                 { $$ = newref($1); }
   | NAME '=' exp         { $$ = newasgn($1, $3); }
   | NAME '(' explist ')' { $$ = newcall($1, $3); }
;

/***************************************************************************** 
* purpose: explist定义表达式列表
* notes: 
  1.采用右递归的方式
******************************************************************************/
explist: exp
  | exp ',' explist { $$ = newast('L', $1, $3); }
;

/***************************************************************************** 
* purpose: 计算器的顶层规则(起始规则)
* notes: 
  1.识别语句列表
  2.识别函数声明
******************************************************************************/
calclist: /* 空 */
  | calclist stmt EOL {
      printf("= %4.4g\n> ", eval($2));
      treefree($2);  // 释放抽象语法树
    }
  | calclist LET NAME '(' symlist ')' '=' list EOL {
      dodef($3, $5, $8);
      printf("DEfined %s\n> ", $3->name);
    }
  | calclist error EOL {
      yyerrok; printf("> ");
    }
%%