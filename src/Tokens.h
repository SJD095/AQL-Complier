#ifndef TOKENS_H
#define TOKENS_H

#include <iostream>
#include <string>

using namespace std;

class Aql_Token
{
public:
	Aql_Token(string value, string kind, int line)
	{
		this->true_value = value;
		this->true_kind = kind;
		this->line = line; 	
	}

	string true_value;
	string true_kind;
	int line;
};

class Input_Token
{
public:
	Input_Token(int start_pos, int end_pos, string value)
	{
		this->startpos = start_pos;
		this->endpos = end_pos;
		this->true_value = value;
	}

	int startpos;
	int endpos;
	string true_value;
};

#endif