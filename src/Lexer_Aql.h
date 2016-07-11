#ifndef AQL_H
#define AQL_H

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

#include "Tokens.h"

using namespace std;

class Lexer_Aql
{
public:
	Lexer_Aql() {}
	Lexer_Aql(char * aql_file);
	vector<Aql_Token> get_aql_tokens();
	
private:
	vector<Aql_Token> Lexer_aql_tokens;
	string file_buffer;
	void incert_aql_token(int start_pos, int end_pos, int &line);
	bool number_decide(string test_string);
};

#endif