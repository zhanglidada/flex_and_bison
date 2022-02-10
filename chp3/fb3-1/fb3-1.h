/**
 * @file fb3-1.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-02-03
 * 
 * @copyright Copyright (c) 2022
 * 
 * 计算器的声明部分
 */

/* 与词法分析器的接口 */
extern int yylineno;  /* 来自于词法分析器 */
void yyerror(char* s, ...);  /* 允许接收多个参数 */

/* 抽象语法树中的节点 */
struct ast {  /* Abstract Syntax Tree */
  int nodetype;
  struct ast* l;
  struct ast* r;
};

struct numval {
  int nodetype;  /* 类型k 表明为常量 */
  double number;
};

/* 构造抽象语法树
   1.每个节点都有节点类型
   2.不同的节点可以有不同的域 
 */
struct ast* newast(int nodetype, struct ast* l, struct ast* r); /* 指向多个节点的指针类型 */
struct ast* newnum(double d);  /* 包含数值的叶子节点 */

/* 遍历抽象语法树
   返回节点代表的表达式的值 
 */
double eval(struct ast*);

/* 遍历抽象语法树
   删除所有节点
 */
void treefree(struct ast*);

