#include "Lexer_Aql.h"

using namespace std;

Lexer_Aql::Lexer_Aql(char * aql_file)
{
	ifstream get_from_argv(aql_file);
	stringstream input_aql_file;
	input_aql_file << get_from_argv.rdbuf();
	file_buffer = input_aql_file.str();
	get_from_argv.close();
	get_from_argv.clear();
	
	int start_pos = 0, end_pos = 1, line = 1;
	for(; end_pos < file_buffer.size(); end_pos++)
	{
		char i = file_buffer[end_pos];
		char j = file_buffer[end_pos - 1];
		if(((i >= 'a' && i <= 'z')||(i >= 'A' && i <= 'Z') || (i >= '0' && i <= '9')) && ((j >= 'a' && j <= 'z')||(j >= 'A' && j <= 'Z') || (j >= '0' && j <= '9')))
		{
			continue;
		}
		else 
		{
			incert_aql_token(start_pos, end_pos, line);
			start_pos = end_pos;
			if(file_buffer[start_pos] == '/')
			{
				do
				{
					end_pos++;
				}
				while(file_buffer[end_pos] != '/');
				end_pos++;
				
				incert_aql_token(start_pos, end_pos, line);				
				start_pos = end_pos;
			}
		}
	}
	
	if(end_pos > start_pos) incert_aql_token(start_pos, end_pos, line);
}

vector<Aql_Token> Lexer_Aql::get_aql_tokens()
{
	return Lexer_aql_tokens;
}

void Lexer_Aql::incert_aql_token(int start_pos, int end_pos, int &line)
{
	string token = file_buffer.substr(start_pos, end_pos - start_pos);
	if(token == " ") return;
	else if(token == "\t") return;
	else if(token == "\n") line++;
	else if (token == "create")
	{
		Lexer_aql_tokens.push_back(Aql_Token(token, token, line));
	} else if (token == "view")
	{
		Lexer_aql_tokens.push_back(Aql_Token(token, token, line));
	} else if (token == "as")
	{
		Lexer_aql_tokens.push_back(Aql_Token(token, token, line));
	} else if (token == "output")
	{
		Lexer_aql_tokens.push_back(Aql_Token(token, token, line));
	} else if (token == "select")
	{
		Lexer_aql_tokens.push_back(Aql_Token(token, token, line));
	} else if (token == "from")
	{
		Lexer_aql_tokens.push_back(Aql_Token(token, token, line));
	} else if (token == "extract")
	{
		Lexer_aql_tokens.push_back(Aql_Token(token, token, line));
	} else if (token == "regex")
	{
		Lexer_aql_tokens.push_back(Aql_Token(token, token, line));
	} else if (token == "on")
	{
		Lexer_aql_tokens.push_back(Aql_Token(token, token, line));
	} else if (token == "return")
	{
		Lexer_aql_tokens.push_back(Aql_Token(token, token, line));
	} else if (token == "group")
	{
		Lexer_aql_tokens.push_back(Aql_Token(token, token, line));
	} else if (token == "Token")
	{
		Lexer_aql_tokens.push_back(Aql_Token(token, token, line));
	} else if (token == "pattern")
	{
		Lexer_aql_tokens.push_back(Aql_Token(token, token, line));
	} else if (token == ".")
	{
		Lexer_aql_tokens.push_back(Aql_Token(token, token, line));
	} else if (token == ";")
	{
		Lexer_aql_tokens.push_back(Aql_Token(token, token, line));
	} else if (token == "(")
	{
		Lexer_aql_tokens.push_back(Aql_Token(token, token, line));
	} else if (token == ")")
	{
		Lexer_aql_tokens.push_back(Aql_Token(token, token, line));
	} else if (token == "<")
	{
		Lexer_aql_tokens.push_back(Aql_Token(token, token, line));
	} else if (token == ">")
	{
		Lexer_aql_tokens.push_back(Aql_Token(token, token, line));
	} else if (token == "{")
	{
		Lexer_aql_tokens.push_back(Aql_Token(token, token, line));
	} else if (token == "}")
	{
		Lexer_aql_tokens.push_back(Aql_Token(token, token, line));
	} else if (token == ",")
	{
		Lexer_aql_tokens.push_back(Aql_Token(token, token, line));
	} else if (token[0] == '/' && token[token.size() - 1] == '/')
	{
		Lexer_aql_tokens.push_back(Aql_Token("REG", token.substr(1, token.size() - 2), line));
	} else if (number_decide(token))
	{
		Lexer_aql_tokens.push_back(Aql_Token("NUM", token, line));
	} else
	{
		Lexer_aql_tokens.push_back(Aql_Token("ID", token, line));
	}
}

bool Lexer_Aql::number_decide(string test)
{
	for (int i = 0; i < test.size(); i++)
	{
		if (test[i] < '0' || test[i] > '9') return false;
	}
	return true;
}