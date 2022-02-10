/* 不使用yywrap */
%option noyywrap
%{
int chars = 0;
int words = 0;
int lines = 0;

int totalchars = 0;
int totalwords = 0;
int totallines = 0;
%}

%%
[a-zA-Z]  { words++; chars += strlen(yytext); }
\n        { chars++; lines++; }
.         { chars++; }

%%

int main(int argc, char** argv)
{
  int i;

  if (argc < 2) { /* 读取标准输入 */
    yylex();  /* 词法分析 */
    printf("%8d%8d%8d", lines, words, chars);
    return 0;
  }
  
  /* 循环遍历所有文件 */
  for (i = 1; i < argc; i++) {
    FILE* f = fopen(argv[i], "r");

    if (!f) {
      perror(argv[i]);
      return 1;
    }

    yyrestart(f);  /* 将打开的文件作为词法分析器的输入 */
    yylex();  /* 词法分析 */
    fclose(f);
    printf("%8d%8d%8d %s\n", lines, words, chars, argv[i]);
    totalchars += chars; chars = 0;
    totallines += lines; lines = 0;
    totalwords += words; words = 0;
  }

  if (argc > 1) {
    printf("%8d%8d%8d total\n", totallines, totalwords, totalchars);
  }

  return 0;
}