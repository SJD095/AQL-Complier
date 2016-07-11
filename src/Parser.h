#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "Lexer_Aql.h"
#include "Lexer_input.h"

using namespace std;

class Parser
{
public:
	Parser(Lexer_Aql l_a_input, Lexer_Input l_i_input);
	void execute();
	
private:
	Lexer_Aql l_a;
	Lexer_Input l_i;
	vector<vector<Aql_Token> > aql_stmts;
	vector<Input_Token> input_tokens;
	map<string, map<string, vector<Input_Token> > > tables;
	vector<Aql_Token> execute_stmt;

	void aql_stmt(int pos);
	void create_stmt(int pos);
	void output_stmt(int pos);
	map<string, vector<Input_Token> > select_stmt(int pos, map<string, string> table_map);
	map<string, vector<Input_Token> > extract_stmt(int pos, map<string, string> table_map);
	map<string, vector<Input_Token> > regex_spec(int pos);
	
	map<string, vector<Input_Token> > pattern_spec(int pos, map<string, string> table_map);
	vector<vector<Input_Token> > match(vector<vector<Input_Token> > result_tokens);
	void pattern_execute(int pos, map<string, string> table_map, vector<vector<Input_Token> > &result_tokens);
	vector<vector<Input_Token> > deal_with_regex(vector<vector<Input_Token> > result_tokens,int pos, string group_tag);
	vector<vector<Input_Token> > deal_with_token(vector<vector<Input_Token> > result_tokens,int pos, string group_tag);
	vector<vector<Input_Token> > deal_with_ID(vector<vector<Input_Token> > result_tokens,int pos, string group_tag, map<string,string> table_map);
	vector<vector<Input_Token> > link(vector<vector<Input_Token> > result_tokens, vector<Input_Token> vector_items, string group_tag);
	vector<vector<Input_Token> > chushihua(string group_tag, vector<Input_Token> vector_items);
	
	vector<Input_Token> regex_output_tokens(string reg);
	string number_to_string(int number_for_string);
	int string_to_number(string string_for_number);
};

#endif
