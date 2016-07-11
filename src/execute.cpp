#include "Lexer_Aql.h"
#include "Lexer_Input.h"
#include "Parser.h"

using namespace std;

int main(int argc, char *argv[2])
{
	
	Lexer_Aql l_a = Lexer_Aql(argv[1]);    //argv[1]为AQL文件
	Lexer_Input l_i = Lexer_Input(argv[2]);    //argv[2]为input文件
	Parser p = Parser(l_a, l_i);
	p.execute();
	
	return 0;
}