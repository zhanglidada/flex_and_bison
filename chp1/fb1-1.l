/* 第一个flex此法分析程序，用于统计文件中单词的个数 */

/* 第一部分：声明和选项设置,原样照抄到生成的c文件中 */
%{
    int chars = 0;
    int words = 0;
    int lines = 0;
%}

/* 第二部分：模式 + 动作 */

%%
[a-zA-Z]+   {words ++; chars += strlen(yytext);}
\n          {chars ++; lines ++;}
.           {chars ++;}
%%

/* 第三部分：拷贝到生成的词法分析器中的代码，动作代码相关的例程 */
int main(int argc, char** argv)
{
    yylex();
    printf("%8d%8d%8d\n", lines, words, chars);
    return 0;
}