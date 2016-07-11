/*Zhongyang Sun 2015.10.31*/

%{
   #include "stdio.h"
   #include "stdlib.h"
%}

Valid   ([0-9]|([1-9][0-9])|([1][0-9][0-9])|([2][0-4][0-9])|([2][5][0-5]))

inValid   ([0-9]*[a-zA-Z]+[0-9]*)|([2][5][6-9])|([2][6-9][0-9])|([3-9][0-9][0-9])|([0-9]{4}[0-9]*)|([0][0]+)|([+-]*[0-9]*)

konghang [\ ]{4}[\ ]*

A   [0\ ]*([0-9])|([1-9][0-9])|([1][0-1][0-9])|([1][2][0-7])

B   [0]*|[\ ]*([1][2][8-9])|([1][3-8][0-9])|([1][9][0-1])

C   [0]*|[\ ]*([1][9][2-9])|([2][0-1][0-9])|([2][2][0-3])

D   [0]*|[\ ]*([2][2][4-9])|([2][3][0-9])

E   [0]*|[\ ]*([2][4][0-9])|([2][5][0-5])

%%

{A}[.]{Valid}[.]{Valid}[.]{Valid} {printf("A");}

{B}[.]{Valid}[.]{Valid}[.]{Valid} {printf("B");}

{C}[.]{Valid}[.]{Valid}[.]{Valid} {printf("C");}

{D}[.]{Valid}[.]{Valid}[.]{Valid} {printf("D");}

{E}[.]{Valid}[.]{Valid}[.]{Valid} {printf("E");}

({Valid}|{inValid})(([.]{Valid})|([.]{inValid})){4}(([.]{Valid})|([.]{inValid}))* {printf("InValid");}

{inValid}{1}(([.]{Valid})|([.]{inValid})){3} {printf("InValid");}

(([.]{Valid})|([.]{inValid})){1}{inValid}{1}(([.]{Valid})|([.]{inValid})){2} {printf("InValid");}

(([.]{Valid})|([.]{inValid})){2}{inValid}{1}(([.]{Valid})|([.]{inValid})){1} {printf("InValid");}

(([.]{Valid})|([.]{inValid})){3}{inValid}{1} {printf("InValid");}

({Valid}|{inValid})(([.]{Valid})|([.]{inValid})){0,2} {printf("InValid");}

{konghang} {printf("InValid");}

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
