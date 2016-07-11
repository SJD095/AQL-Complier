/*Zhongyang Sun 2015.10.31*/

%{
   #include "stdio.h"
   #include "stdlib.h"
%}

zhushi ([/][/][^\n]*)|([/][*]([^1]|[\n])*[*][/])|([#][^\n]*)

guanjianci (STRICT)|(strict)|(GRAPH)|(graph)|(DIGRAPH)|(digraph)|(SUBGRAPH)|(subgraph)|(NODE)|(node)|(EDGE)|(edge)

ID [_a-zA-Z][_a-zA-Z0-9]*

STRING ["]([^\n]|[\"])*["]

NUMBER ([-+]?[0-9][0-9]*)|([-+]?[.][0-9]+)|([-+]?[0-9]+[.][0-9]*)

fengefu [;]|[,]|[{]|[}]|[[]|[]]

qitafuhao (--)|(->)|(=)

kongge ([\ ])|(\t)|([\n])

%%

{zhushi} {}

{guanjianci} {printf("%s\n", yytext);}

{kongge} {}

{qitafuhao} {printf("%s\n", yytext);}

{ID} {printf("%s\n", yytext);}

{STRING} {printf("%s\n", yytext);}

{fengefu} {printf("%s\n", yytext);}

{NUMBER} {printf("%d\n", atoi(yytext));}

%%

int main()
{
   yylex();
   printf("\n");
   return 0;
}

int yywrap() // 此函数必须由用户提供, 函数的返回值是1，就表示停止解析
{
   return 1;
}
