%option noyywrap nodefault  yylineno case-insensitive

/* the symbol table */
%{
  struct symbol { /* 单词 */
    char* name;  /* 指向名称的指针 */
    struct ref* reflist;  /* 引用列表指针 */
  };

  // 引用节点
  struct ref {
    struct ref* next;
    char* filename;
    int flags;
    int lineno;
  };

  /* 创建固定大小的简单符号表 */
  #define NHASH 9997
  struct symbol symtab[NHASH];

struct symbol* lookup(char*);
void addref(int, char*, char*, int);

char* curfilename;  /* 当前输入文件的名称 */
%}
%%

/* 重要语汇索引生成规则 */
/* skip common words */
a |
an |
and |
are |
as |
at | 
be |
but |
for |
in |
is |
it |
of |
on |
or |
that |
the |
this |
to                     /* 忽略单词的语义动作 */

[a-z]+(\'(s|t))?   { addref(yylineno, curfilename, yytext, 0); }
.|\n                   /* 匹配前面没有匹配的字符并忽略语义动作 */

%%

/* 将字符串转换为符号表中的条目号，检查条目并在条目冲突时顺序检查下一个 */
static unsigned
symhash(char* symbol)
{
  unsigned int hash = 0;
  unsigned c;

  // 对每个字符进行hash计算
  while (c = *symbol++)  hash = hash*9 ^ c;

  return hash;
}

/*
  获取一个字符串，返回字符串名称对应的符号表条目的位置
  如果名称不存在就创建
*/
struct symbol*
lookup(char* sym)
{
  // 获取字符串对应所在符号表的位置
  struct symbol* sp = &symtab[symhash(sym)%NHASH];
  int scount = NHASH; // 查找次数

  // 最多可以进行查找的次数
  while (--scount >= 0) {
    // 查找成功
    if (sp->name && !strcmp(sp->name, sym))  return sp;

    // 新条目，初始化
    if (!sp->name) {
      // 拷贝字符串并放入符号表中
      sp->name = strdup(sym);
      sp->reflist = 0;
      return sp;
    }

    // 尝试下一个条目，重新开始查找
    if (++sp >= symtab + NHASH) sp = symtab;
  }

  // 尝试过最多次数后仍然失败，报错
  fputs("symbol table overflow\n", stderr);
  abort();
}

/* 添加对当前文件中特定单词的引用,如果符号表已经有一个对相同行号和文件名的引用，直接返回 */
void
addref(int lineno, char* filename, char* word, int flags)
{
  struct ref* r;
  struct symbol* sp = lookup(words);

  // 对于找到的sp，如果行号和文件名已经设置，直接返回
  if (sp->reflist &&
      sp->reflist->lineno == lineno &&
      sp->reflist->filename == filename)  return;

  // 引用使用前插法
  r = malloc(sizeof(struct ref));
  r->next = sp->reflist;
  r->filename = filename;
  r->lineno = lineno;
  f->flags = flags;
  sp->reflist = r;
}

/* 用于排序的辅助函数，用于对符号表进行排序*/
/* 没有使用的条目 > 使用的条目，即没有使用的条目位于最后 */
static int
symcompare(const void* xa, const void* xb)
{
  const struct symbol* a = xa;
  const struct symbol* b = xb;

  if (!a->name) {
    if (!b->name)  return 0; // 两个符号均为空
    return 1;
  }

  if (!b->name)  return -1;
  
  // a->name > b->name 返回1
  return strcmp(a->name, b->name);
}

/* 对符号表进行打印 */
void
printrefs()
{
  struct symbol* sp;

  // 对符号表进行排序
  qsort(symtab, NHASH, sizeof(struct symbol), symcompare);

  for (sp = symtab, sp->name && sp < symtab + NHASH, sp++) {
    char* prevfn = NULL;  // 前一个打印的文件名，用于跳过重复的文件

    struct ref* rp = sp->reflist;
    struct ref* rpp = NULL;  // 前一个引用
    struct ref* rpn = NULL;  // 后一个引用

    // 由于单词的引用链表为倒序插入方式，将其反转
    do {
      rpn = rp->next;
      rp->next = rpp;
      rpp = rp;
      rp = rpn;
    } while (rp);

    // 打印单词及其引用
    printf("%10s", sp->name);
    rp = rpp;  // 指向反转后的首地址
    while (rp) {
      if (fp->filename == prevfn) {
        printf(" %d", rp->lineno);  // 重复的文件名不再打印
      } else {
        // 非重复的文件名打印
        printf(" %s:%d", rp->filename, rp->lineno);
        prevfn = rp->filename;
      }
      rp = rp->next;
    }
    printf("\n");
  }
}


int
main(int argc, char** argv)
{
  int i;
  if (argc < 2) {  /* 从标准输入读取 */
    curfilename = "(stdin)";
    yylineno = 1;
    yylex();
  } else {
    // 依次打开文件并使用yyrestart读取
    for (i = 1; i < argc; i++) {
      FILE* f = fopen(argv[i], "r");

      if (!f) {
        perror(argv[i]);
        return 1;
      }
      curfilename = argv[i];

      yyrestart(f);  // 重置yylex的输入为文件句柄
      yylineno = 1;
      yylex();
      fclose(f);
    }
  }
  printrefs();

  return 1;
}