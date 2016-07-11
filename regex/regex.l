%{
    #include <stdlib.h>
    #include "regex.tab.h"
%}

%%

"."                             {yylval.Lit_c = *yytext; return DOT;}
"|"                             return '|';
"("                             return '(';
")"                             return ')';
"*"                             return '*';
"+"                             return '+';
"?"                             return '?';
"(?:"                           return wushi;
"*?"                            return NgStar;
"+?"                            return NgPlus;
"??"                            return NgQuest;
"\n"                            return '\n';
(.)                             {yylval.Lit_c = *yytext; return LIT; }

%%

int yywrap(void)
{
    return 1;
}
