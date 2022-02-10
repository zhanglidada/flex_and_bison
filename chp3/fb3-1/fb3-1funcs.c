/*
 * helper functions for fb3-1
 */

#  include <stdio.h>
#  include <stdlib.h>
#  include <stdarg.h>
#  include "fb3-1.h"

struct ast*
newast(int nodetype, struct ast*l, struct ast* r)
{
  struct ast* a = malloc(sizeof(struct ast));

  // 创建失败
  if (!a) {
    yyerror("out of space!");
    exit(0);
  }
  a->nodetype = nodetype;
  a->l = l;
  a->r = r;
  return (struct ast*) a;
}

struct ast*
newnum(double d)
{
  struct numval* a = malloc(sizeof(struct numval));
    // 创建失败
  if (!a) {
    yyerror("out of space!");
    exit(0);
  }

  a->nodetype = 'K';
  a->number = d;
  return (struct ast*) a;
}

double
eval(struct ast* a)
{
  double v;  // 子树的计算结果

  switch(a->nodetype) {
    case 'K':
      v = ((struct numval*)a)->number;
      break;
    case '+':
      v = eval(a->l) + eval(a->r);
      break;
    case '-':
      v = eval(a->l) - eval(a->r);
      break;
    case '*':
      v = eval(a->l) * eval(a->r);
      break;
    case '/':
      v = eval(a->l) / eval(a->r);
      break;
    case '|':
      v = eval(a->l);
      if (v < 0) v = -v;
      break;
    case 'M':
      v = -eval(a->l);
      break;
  }
  return v;
}

void
treefree(struct ast* a)
{
  switch(a->nodetype) {
    
    /* 两棵子树 */
    case '+':
    case '-':
    case '*':
    case '/':
      // 释放右子树
      treefree(a->r);

    /* 一棵子树 */
    case '|':
    case 'M':
      // 释放左子树
      treefree(a->l);
      break;

    /* 没有子树，为叶子节点 */
    case 'K':
      free(a);
      break;
    default:
      printf("internal error: free bad node %c\n", a->nodetype);
  }
}

void
yyerror(char* s, ...)
{
  va_list ap;
  va_start(ap, s);

  fprintf(stderr, "%d: error: ", yylineno);
  vfprintf(stderr, s, ap);
}

int
main()
{
  printf("> ");
  return yyparse();
}