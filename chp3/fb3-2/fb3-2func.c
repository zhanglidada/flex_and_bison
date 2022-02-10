/**
 * @file fb3-2func.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-02-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#  include <stdio.h>
#  include <stdlib.h>
#  include <stdarg.h>
#  include <string.h>
#  include <math.h>
#  include "fb3-2.h"

static double callbuiltin(struct fncall*);
static double calluser(struct ufncall*);


/***************************************************************************** 
* purpose: 符号表
* notes: hash一个字符串
******************************************************************************/
static unsigned
symhash(char* sym)
{
  unsigned int hash = 0;
  unsigned c;

  while (c = *sym++)
    hash = (hash * 9) ^ c;

  return hash;
}

struct symbol*
lookup(char* sym)
{
  // 根据字符串的hash值获取对应的符号表中元素地址
  struct symbol* sp = &symtab[symhash(sym) % NHASH];
  int scount = NHASH;

  // 最大匹配次数NHASH - 1次
  while (--scount) {
    if (sp->name && !strcmp(sp->name, sym))  return sp;

    // 新条目
    if (!sp->name) {
      sp->name = strdup(sym);
      sp->value = 0;
      sp->func = NULL;
      sp->syms = NULL;
      return sp;
    }

    if (++sp >= symtab + NHASH) sp = symtab;  // 尝试下一个条目
  }

  yyerror("symbol table overflow\n");
  abort();  // 尝试完所有条目，符号表已满
}

/***************************************************************************** 
* purpose: 构造语法树的节点
* notes: 
  分配节点，根据节点类型填充域
******************************************************************************/
struct ast*
newast(int nodetype, struct ast* l, struct ast* r)
{
  struct ast* a = malloc(sizeof(struct ast));

  if (!a) {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = nodetype;
  a->l = l;
  a->r = r;
  return a;
}

/***************************************************************************** 
* purpose: 构造语法树的节点
* notes: 
  创建的节点类型为叶子节点
******************************************************************************/
struct ast*
newnum(double d)
{
  struct numval* a = malloc(sizeof(struct numval));

  if (!a) {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = 'K';
  a->number = d;
  // 将numval类型的节点强转成ast类型并返回
  return (struct ast*)a;
}

/***************************************************************************** 
* purpose: 构造语法树的节点
* notes: 
  创建的节点类型为比较类型
******************************************************************************/
struct ast*
newcmp(int cmptype, struct ast* l, struct ast* r)
{
  struct ast* a = malloc(sizeof(struct ast));

  if (!a) {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = '0' + cmptype;
  a->l = l;
  a->r = r;
  return a;
}

/***************************************************************************** 
* purpose: 构造语法树的节点
* notes: 
  创建的节点类型为内置函数类型
******************************************************************************/
struct ast*
newfunc(int functype, struct ast *l)
{
  struct fncall *a = malloc(sizeof(struct fncall));
  
  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'F';
  a->l = l;
  a->functype = functype;
  return (struct ast *)a;
}

/***************************************************************************** 
* purpose: 构造语法树的节点
* notes: 
  创建的节点类型为用户定义函数类型
******************************************************************************/
struct ast*
newcall(struct symbol *s, struct ast *l)
{
  struct ufncall *a = malloc(sizeof(struct ufncall));
  
  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'C';
  a->l = l;
  a->s = s;
  return (struct ast *)a;
}

/***************************************************************************** 
* purpose: 构造语法树的节点
* notes: 
  创建的节点类型为符号的引用类型
******************************************************************************/
struct ast*
newref(struct symbol *s)
{
  struct symref *a = malloc(sizeof(struct symref));
  
  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'N';
  a->s = s;
  return (struct ast *)a;
}

/***************************************************************************** 
* purpose: 构造语法树的节点
* notes: 
  创建的节点类型赋值类型
******************************************************************************/
struct ast*
newasgn(struct symbol *s, struct ast *v)
{
  struct symasgn *a = malloc(sizeof(struct symasgn));
  
  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = '=';
  a->s = s;
  a->v = v;
  return (struct ast *)a;
}

/***************************************************************************** 
* purpose: 创建新的流类型节点
* notes: 
******************************************************************************/
struct ast*
newflow(int nodetype, struct ast* cond, struct ast* tl, struct ast* el)
{
  struct flow* a = malloc(sizeof(struct flow));

    if(!a) {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = nodetype;
  a->cond = cond;  // 节点的判断条件
  a->tl = tl;
  a->el = el;
  return (struct ast*)a;
}

struct symlist*
newsymlist(struct symbol* sym, struct symlist* next)
{
  struct symlist* sl = malloc(sizeof(struct symlist));

  if (!sl) {
    yyerror("out of space");
    exit(0);
  }

  sl->sym = sym;
  sl->next = next;

  return sl;
}

/***************************************************************************** 
* purpose: 释放符号表的列表
* notes: 
******************************************************************************/
void
symlistfree(struct symlist* sl)
{
  struct symlist* nsl;

  while (sl) {
    nsl = sl->next;
    free(sl);
    sl = nsl;
  }
}



/***************************************************************************** 
* purpose: 计算机的核心例程
* notes: 
  1.计算语法分析器中构造的抽象语法树
  2.基于c语言的准则，比较表达式的值是否成功，返回0/1
  3.if/then/else while/do中认为非0值均为真
******************************************************************************/
double
eval(struct ast* a)
{
  double v;

  if (!a) {
    yyerror("internel error, null eval");
    return 0.0;
  }

  switch(a->nodetype) {
    
    // 常量
    case 'K': v = ((struct numval*)a)->number; break;
    
    // 名字引用
    case 'N': v = ((struct symref*)a)->s->value; break;

    // 赋值操作符
    case '=': v = ((struct symasgn*)a)->s->value = eval(((struct symasgn*)a)->v); break;

    // 表达式
    case '+': v = eval(a->l) + eval(a->r); break;
    case '-': v = eval(a->l) - eval(a->r); break;
    case '*': v = eval(a->l) * eval(a->r); break;
    case '/': v = eval(a->l) / eval(a->r); break;
    case '|': v = fabs(eval(a->l)); break;
    case 'M': v = -eval(a->l); break;

    // 比较操作符
    case '1': v = (eval(a->l) > eval(a->r)) ? 1 : 0; break;
    case '2': v = (eval(a->l) < eval(a->r)) ? 1 : 0; break;
    case '3': v = (eval(a->l) != eval(a->r)) ? 1 : 0; break;
    case '4': v = (eval(a->l) == eval(a->r)) ? 1 : 0; break;
    case '5': v = (eval(a->l) >= eval(a->r)) ? 1 : 0; break;
    case '6': v = (eval(a->l) <= eval(a->r)) ? 1 : 0; break;

    /* 控制流 */
    /* 由于控制流中允许空表达式，所以需要检查这种可能性 */

    /* if/then/else */
    case 'I':
      // if条件判断
      if (eval(((struct flow *)a)->cond) != 0) {
        // then分支处理
        if (((struct flow*)a)->tl) {
          v = eval(((struct flow*)a)->tl);
        } else {
          v = 0.0;  // 默认值，处理空语句
        }
      } else {  // else分支处理
        if (((struct flow*)a)->el) {
          v = eval(((struct flow*)a)->el);
        } else {
          v = 0.0;  // 默认值，处理空语句
        }
      }
      break;
    /*while/do */
    case 'W':
      v = 0.0;
      // do分支处理
      if (((struct flow*)a)->tl) {
        // 计算条件
        while (eval(((struct flow*)a)->cond != 0))
          v = eval(((struct flow*)a)->tl);
      }
      break;

    // 处理语句列表
    case 'L': eval(a->l); v = eval(a->r); break;

    // 创建内置函数的值计算
    case 'F': v = callbuiltin((struct fncall*)a); break;

    // 创建用户自定义函数的值计算
    case 'C': v = calluser((struct ufncall*)a); break;

    default: printf("internel error: bad node %c\n", a->nodetype);
  }
  return v;
}

/***************************************************************************** 
* purpose: 释放一棵抽象语法树
* notes: 
  传入的是抽象语法树的根节点
******************************************************************************/
void
treefree(struct ast* a)
{
  switch (a->nodetype)
  {
    // 两棵子树时
    case '+':
    case '-':
    case '*':
    case '/':
    case '1': case '2': case '3': case '4': case '5': case '6':
    case 'L':  // L为explist类型，即statement list
      treefree(a->r);

    // 一棵子树
    case '|':
    case 'M': case 'C': case 'F':
      treefree(a->l);

    // 没有子树,K为数值类型，叶子节点
    case 'K': case 'N':
      break;

    // 最多三棵子树
    case 'I': case 'W':
      free(((struct flow*)a)->cond);
      // 释放then分支或者do语句
      if (((struct flow*)a)->tl) treefree(((struct flow*)a)->tl);
      // 释放可选的else分支
      if (((struct flow*)a)->el) treefree(((struct flow*)a)->el);
      break;
    
    default:
      printf("internal error: free bad node %c\n", a->nodetype);
  }

  free(a);  // 释放自身节点
}


/***************************************************************************** 
* purpose: 处理函数，即内置处理函数
* notes: 
  确定具体函数，调用相应代码执行
******************************************************************************/
static double
callbuiltin(struct fncall* f)
{
  enum bifs functype = f->functype;
  double v = eval(f->l);  // 计算调用节点的值

  switch(functype) {
    case B_sqrt:
      return sqrt(v);
    case B_exp:
      return exp(v);
    case B_log:
      return log(v);
    case B_print:
      printf("= %4.4g\n", v);
      return v;
    default:
      yyerror("Unknown built-in function %d", functype);
      return 0.0;
  }
}

/***************************************************************************** 
* purpose: 用户自定义函数
* notes: 
  1.包含函数名，虚拟参数(函数定义时的参数名)，代表函数体的抽象语法树
  2.函数定义时，参数列表以及抽象语法树简单保存到符号表中函数名对应的条目中
  3.替换可能的旧版本
******************************************************************************/
void
dodef(struct symbol* name, struct symlist* syms, struct ast* func)
{
  if (name->syms) symlistfree(name->syms);
  if (name->func) treefree(name->func);
  name->syms = syms;  // 函数的虚拟参数列表
  name->func = func;  // 定义的函数代码，即函数体
}

/***************************************************************************** 
* purpose: 具体的用户定义函数调用
* notes: 调用流程
  1. 计算实际参数值
  2. 保存虚拟参数当前值，并将实际参数赋值给虚拟参数
  3. 执行函数体，在虚拟参数使用处用实际参数替换
  4. 虚拟参数恢复原值
  5. 返回函数体表达式的值
******************************************************************************/
static double
calluser(struct ufncall* f)
{
  struct symbol* fn = f->s;  // 指向具体函数的指针
  struct symlist* sl = NULL;  // 虚拟参数
  struct ast* args = f->l;  // 实际参数列表,起始为一个抽象语法树节点
  double* oldval = NULL, *newval = NULL;  // 保存的参数值
  double v = 0.0;
  int nargs = 0;  // 参数个数
  int i = 0;

  if (!fn->func) {
    yyerror("call to undefined function", fn->name);
    return 0;
  }

  // 计算参数个数
  sl = fn->syms;  // 自定函数的虚拟参数列表
  while (sl) {
    nargs++;
    sl = sl->next;
  }

  // 为保存参数值做准备
  oldval = (double*)malloc(nargs * sizeof(double));
  newval = (double*)malloc(nargs * sizeof(double));

  // 每个虚拟参数对应的实际参数
  for (int i = 0; i < nargs; i++) {
    // 实际参数不存在
    if (!args) {
      yyerror("too few args in call to %s", fn->name);
      free(oldval);
      free(newval);
      return 0.0;
    }

    // 判断是否为节点链表
    if (args->nodetype == 'L') {
      newval[i] = eval(args->l);
      args = args->r;  // 参数列表中的下一个元素
    } else {
      // 列表末尾
      newval[i] = eval(args);
      args = NULL;
    }
  }

  // 保存虚拟参数的旧值，赋予新值
  sl = fn->syms;
  for (i = 0; i < nargs; i++) {
    struct symbol *s = sl->sym;

    oldval[i] = s->value;
    s->value = newval[i];
    sl = sl->next;
  }
  free(newval);

  // 计算函数,使用实际参数的值替换虚拟参数的值
  v = eval(fn->func);

  // 恢复虚拟参数的值
  sl = fn->syms;
  for (i = 0; i < nargs; i++) {
    struct symbol *s = sl->sym;

    s->value = oldval[i];
    sl = sl->next;
  }
  free(oldval);
  return v;
}