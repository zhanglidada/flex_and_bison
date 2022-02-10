/**
 * @file fb3-2.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-02-05
 * 
 * @copyright Copyright (c) 2022
 * 
 * 计算器的声明部分
 */

/* 与词法分析器的接口 */
extern int yylineno;  // 词法分析器已经定义
void yyerror(char* s, ...);

/***************************************************************************** 
* purpose: 符号表，存放语句的信息
* notes: 
  1.每个符号都可以有一个变量以及一个用户自定义函数
  2.可以指向抽象语法树的指针或者一个具体的数值
******************************************************************************/
struct symbol { /* 变量名 */
  char*           name;
  double          value;  // 保存符号的值
  struct ast*     func;  // 代表函数体的抽象语法树，指向抽象语法树表述的该函数的用户代码
  struct symlist* syms;  // 指向任意多个虚拟参数列表
};

/* 固定大小的简单符号表 */
#define NHASH 9997
struct symbol symtab[NHASH];

struct symbol* lookup(char*);

/***************************************************************************** 
* purpose: 符号列表
* notes: 作为参数列表
******************************************************************************/
struct symlist {
  struct symbol* sym;  // 当前符号表
  struct symlist* next;  // 下一个符号表
};

// 创建符号链表
struct symlist* newsymlist(struct symbol *sym, struct symlist *next);
// 释放符号
void symlistfree(struct symlist* sl);

/* node types
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

enum bifs {  // 内置函数
  B_sqrt = 1,
  B_exp,
  B_log,
  B_print
};

/***************************************************************************** 
* purpose: 抽象语法树节点(Abstract Syntax Tree)
* notes: 所有节点都有公共的初始nodetype
******************************************************************************/
struct ast {
  int nodetype;
  struct ast* l;
  struct ast* r;
};

/***************************************************************************** 
* purpose: 内置函数的节点类型
* notes: 
******************************************************************************/
struct fncall {
  int nodetype;  // 类型F
  struct ast* l;  // 抽象语法树节点
  enum bifs functype;  // 内置函数的枚举类型
};

/***************************************************************************** 
* purpose: 用户自定义函数
* notes: 在交互界面用户可以自定义函数功能，有点类似于c的宏替换
******************************************************************************/
struct ufncall {
  int nodetype;  // 类型C
  struct ast* l;  // 参数列表的抽象语法树节点，自定义函数的实际参数
  struct symbol* s;  // 指向自定义函数的指针
};

/***************************************************************************** 
* purpose: 流语句节点，即条件判断节点类型
* notes: 
******************************************************************************/
struct flow {
  int nodetype;  // 类型I或者类型W
  struct ast* cond;  // if条件
  struct ast* tl;  // then分支或者do语句
  struct ast* el;  // 可选的else分支
};

/***************************************************************************** 
* purpose: 常量类型， 为叶子节点类型
* notes: 
******************************************************************************/
struct numval {
  int nodetype;  // 类型K
  double number;
};

/***************************************************************************** 
* purpose:符号引用类型
* notes: 
******************************************************************************/
struct symref {
  int nodetype;  // 类型N
  struct symbol* s;  // 指向符号表中特定符号的指针
};

struct symasgn {
  int nodetype; // 类型 =
  struct symbol* s;  // 指向被赋值符号的指针
  struct ast* v;  // 使用抽象语法树表示的值
};

// 构造抽象语法树
struct ast *newast(int nodetype, struct ast *l, struct ast *r);
struct ast *newcmp(int cmptype, struct ast *l, struct ast *r);
struct ast *newfunc(int functype, struct ast *l);
struct ast *newcall(struct symbol *s, struct ast *l);
struct ast *newref(struct symbol *s);
struct ast *newasgn(struct symbol *s, struct ast *v);
struct ast *newnum(double d);
struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *tr);

// 定义函数
void dodef(struct symbol* name, struct symlist* syms, struct ast* stmts);

// 计算抽象语法树
double eval(struct ast*);

// 删除和释放抽象语法树
void treefree(struct ast*);

// 与词法分析器对应的接口
extern int yylineno;  // 来自词法分析器
void yyerror(char* s, ...);

extern int debug;
void dumpast(struct ast* a, int level);