#include "Lexer_Input.h"

using namespace std;

Lexer_Input::Lexer_Input(char *input_text_file)
{
	ifstream get_from_argv(input_text_file);
	stringstream input_file;
	input_file << get_from_argv.rdbuf();
	file_buffer = input_file.str();
	get_from_argv.close();
	get_from_argv.clear();

	int start_pos = 0, end_pos = 1;
	
	for (; end_pos < file_buffer.size(); end_pos++)
	{
		if (char_or_number(file_buffer[end_pos]) && char_or_number(file_buffer[end_pos - 1]))
		{
			continue;
		}
		
		else
		{
			string token_in = file_buffer.substr(start_pos, end_pos - start_pos);

			if (token_in != "\t" && token_in != " " && token_in != "\n")
			{
				Lexer_input_tokens.push_back(Input_Token(start_pos, end_pos, token_in));
			}
			start_pos = end_pos;
		}
	}

	if (end_pos > start_pos)
	{
		string token_in = file_buffer.substr(start_pos, end_pos - start_pos);
		if (token_in != "\t" && token_in != " " && token_in != "\n")
		{
			Lexer_input_tokens.push_back(Input_Token(start_pos, end_pos, token_in));
		}
	}
}

vector<Input_Token> Lexer_Input::get_lexer_input_tokens()
{
	return Lexer_input_tokens;
}

string Lexer_Input::get_file_buffer()
{
	return file_buffer;
}

bool Lexer_Input::char_or_number(char test_char)
{
	if((test_char >= 'a' && test_char <= 'z')||(test_char >= 'A' && test_char <= 'Z') || (test_char >= '0' && test_char <= '9')) return true;
	else
	{
		return false;
	}
}