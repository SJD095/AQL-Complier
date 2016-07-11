#ifndef LEXER_INPUT
#define LEXER_INPUT

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include "Tokens.h"

using namespace std;

class Lexer_Input
{
public:
	Lexer_Input() {}
	Lexer_Input(char * input_file);
	string get_file_buffer();
	vector<Input_Token> get_lexer_input_tokens();
private:
	vector<Input_Token> Lexer_input_tokens;
	string file_buffer;
	bool char_or_number(char test_char);
};

#endif