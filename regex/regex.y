%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* BiNode(char* value, char* first, char* second) //具有两个子树
{
      char* result = (char*)malloc((int)strlen(value) + (int)strlen(first) + (int)strlen(second) + 50);
      strcpy(result, value);
      strcat(result, "(");
      strcat(result, first);
      strcat(result, ", ");
      strcat(result, second);
      strcat(result, ")");
      return result;
}

char* SingleNode(char* value, char* str) //拥有一个子树
{
  int len;
  if (str == NULL) len = 0;
  else len = (int)strlen(str);
  char* result = (char*)malloc((int)strlen(value) + len + 50);
  strcpy(result, value);
  if (str == NULL)return result;
  strcat(result, "(");
  strcat(result, str);
  strcat(result, ")");
  return result;
}


%}

%union
{
  char Lit_c;
  char* string_result; //最终用于输出的string
}


%token LIT DOT
%left  '|' '(' ')' wushi
%right '+' '*' '?' NgStar NgPlus NgQuest

%%

input :
      | input line
;

line  : '\n'
      | exp '\n'            { printf("%s\n", $<string_result>$);}
;

exp   :exp '|' term              {$<string_result>$ = BiNode("Alt", $<string_result>1, $<string_result>3);}
        |term                     {$<string_result>$=$<string_result>1;}

term: term expression               {$<string_result>$ = BiNode("Cat", $<string_result>1, $<string_result>2);}
        |expression                       {$<string_result>$=$<string_result>1;}
;
expression:DOT                            {$<string_result>$=SingleNode("Dot", NULL);}
          |LIT                    {$<string_result>$=SingleNode("Lit", &$<Lit_c>1);}
          |expression '+'                     {$<string_result>$=SingleNode("Plus", $<string_result>1); }
          | expression '*'                     {$<string_result>$=SingleNode("Star", $<string_result>1);}
          | expression '?'                     {$<string_result>$=SingleNode("Quest", $<string_result>1);}
          | expression NgStar             {$<string_result>$=SingleNode("NgStar", $<string_result>1);}
          | expression NgPlus             {$<string_result>$=SingleNode("NgPlus", $<string_result>1);}
          | expression NgQuest           {$<string_result>$=SingleNode("NgQuest", $<string_result>1);}
          |wushi exp ')'               {$<string_result>$= $<string_result>2;}
          |'(' exp ')'                     {$<string_result>$=SingleNode("Paren", $<string_result>2);}
;

%%

int main()
{
    return yyparse();
}
