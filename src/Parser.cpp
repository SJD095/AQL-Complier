#include <iostream>
#include <vector>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Parser.h"

using namespace std;

enum
{
	LeftmostBiased = 0,
	LeftmostLongest = 1,
};

enum
{
	RepeatMinimal = 0,
	RepeatLikePerl = 1,
};

int debug;
int matchtype = LeftmostBiased;
int reptype = RepeatMinimal;

enum
{
	NSUB = 10
};

typedef struct Sub Sub;
struct Sub
{
	const char *sp;
	const char *ep;
};

enum {
	CharIncluded = 0,
	CharExcluded = 1,
};

enum {
	NCHAR = 128,
};
typedef struct Range Range;
struct Range {
	int type;
	char flag[NCHAR];
};

typedef union Data Data;
union Data
{
	int val;
	Range range;
};

enum
{
	Char = 1,
	Any = 2,
	Split = 3,
	LParen = 4,
	RParen = 5,
	Match = 6,
	CharClass = 7,
};
typedef struct State State;
typedef struct Thread Thread;
struct State
{
	int op;
	Data data;
	State *out;
	State *out1;
	int id;
	int lastlist;
	int visits;
	Thread *lastthread;
};

struct Thread
{
	State *state;
	Sub match[NSUB];
};

typedef struct List List;
struct List
{
	Thread *t;
	int n;
};

State matchstate = { Match };
int nstate;
int listid;
List l1, l2;

/* Allocate and initialize State */
State*
state(int op, int val, State *out, State *out1)
{
	State *s;

	nstate++;
	s = (State *)malloc(sizeof *s);
	s->lastlist = 0;
	s->op = op;
	s->data.val = val;
	s->out = out;
	s->out1 = out1;
	s->id = nstate;
	return s;
}

/* Allocate and initialize CharClass State */
State*
ccstate(int op, Range range, State *out, State *out1)
{
	State *s;

	nstate++;
	s = (State *)malloc(sizeof *s);
	s->lastlist = 0;
	s->op = op;
	s->data.range = range;
	s->out = out;
	s->out1 = out1;
	s->id = nstate;
	return s;
}

typedef struct Frag Frag;
typedef union Ptrlist Ptrlist;
struct Frag
{
	State *start;
	Ptrlist *out;
};

/* Initialize Frag struct. */
Frag
frag(State *start, Ptrlist *out)
{
	Frag n = { start, out };
	return n;
}

/*
 * Since the out pointers in the list are always
 * uninitialized, we use the pointers themselves
 * as storage for the Ptrlists.
 */
union Ptrlist
{
	Ptrlist *next;
	State *s;
};

/* Create singleton list containing just outp. */
Ptrlist*
list1(State **outp)
{
	Ptrlist *l;

	l = (Ptrlist*)outp;
	l->next = NULL;
	return l;
}

/* Patch the list of states at out to point to start. */
void
patch(Ptrlist *l, State *s)
{
	Ptrlist *next;

	for(; l; l=next){
		next = l->next;
		l->s = s;
	}
}

/* Join the two lists l1 and l2, returning the combination. */
Ptrlist*
append(Ptrlist *l1, Ptrlist *l2)
{
	Ptrlist *oldl1;

	oldl1 = l1;
	while(l1->next)
		l1 = l1->next;
	l1->next = l2;
	return oldl1;
}

int nparen;
void yyerror(const char*);
int yylex(void);
State *start;

Frag
paren(Frag f, int n)
{
	State *s1, *s2;

	if(n >= NSUB)
		return f;
	s1 = state(LParen, n, f.start, NULL);
	s2 = state(RParen, n, NULL, NULL);
	patch(f.out, s2);
	return frag(s1, list1(&s2->out));
}

typedef union YYSTYPE YYSTYPE;
union YYSTYPE {
	Frag frag;
	int c;
	int nparen;
	Range range;
};
YYSTYPE yylval;

const char *input;
const char *text;
void dumplist(List*);

enum
{
	EOL = 0,
	CHAR = 257,
	CHARCLASS = 258,
};

int
yylex(void)
{
	int c;

	if(input == NULL || *input == 0)
		return EOL;
	c = *input++;
	/* escape character */
	if (c == '\\') {
		c = *input++;
		switch (c) {
			case '\0':
				yyerror("invalid regexp");
				exit(1);
			case 'r':
				yylval.c = '\r';
				break;
			case 'n':
				yylval.c = '\n';
				break;
			case 't':
				yylval.c = '\t';
				break;
			default:
				yylval.c = c;
				break;
		}
		return CHAR;
	}
	/* character class */
	if (c == '[') {
		int i, nchar = 0, ndash = 0;
		char lastchar;
		yylval.range.type = CharIncluded;
		if (*input == '^') {
			yylval.range.type = CharExcluded;
			input++;
		}
		if (*input == ']') {
			yyerror("invalid regexp");
			exit(1);
		}
		memset(yylval.range.flag, 0, sizeof(yylval.range.flag));
		while (*input != 0) {
			c = *input++;
			if (c == ']') {
				if (nchar > 0)
					yylval.range.flag[lastchar] = 1;
				if (ndash > 0)
					yylval.range.flag['-'] = 1;
				if (yylval.range.type == CharExcluded) {
					for (i=0; i<NCHAR; i++)
						yylval.range.flag[i] = 1-yylval.range.flag[i];
				}
				return CHARCLASS;
			}
			if (c == '-') {
				ndash++;
				continue;
			}
			if (c == '\\') {
				c = *input++;
				switch (c) {
					case '\0':
						yyerror("invalid regexp");
						exit(1);
					case 'r':
						c = '\r';
						break;
					case 'n':
						c = '\n';
						break;
					case 't':
						c = '\t';
						break;
					default:
						break;
				}
			}
			if (nchar > 0 && ndash > 0) {
				nchar = ndash = 0;
				if (lastchar > c) {
					yyerror("invalid regexp");
					exit(1);
				} else {
					for (i=lastchar; i<=c; i++)
						yylval.range.flag[i] = 1;
				}
			} else if (nchar > 0) {
				yylval.range.flag[lastchar] = 1;
				lastchar = c;
			} else if (ndash > 0) {
				ndash = 0;
				yylval.range.flag['-'] = 1;
				nchar++;
				lastchar = c;
			} else {
				nchar++;
				lastchar = c;
			}
		}
		yyerror("invalid regexp");
		exit(1);
	}
	if(strchr("|*+?():.", c))
		return c;
	yylval.c = c;
	return CHAR;
}

int look;

void
move()
{
	look = yylex();
}

int
matchtoken(int t)
{
	if (look == t) {
		move();
		return 1;
	}
	return 0;
}

Frag single();
Frag repeat();
Frag concat();
Frag alt();
void line();

void
line()
{
	Frag alt1 = alt();
	if (!matchtoken(EOL))
		yyerror("expected EOL");
	State *s;
	alt1 = paren(alt1, 0);
	s = state(Match, 0, NULL, NULL);
	patch(alt1.out, s);
	start = alt1.start;
}

Frag
alt()
{
	Frag concat1 = concat();
	while (matchtoken('|')) {
		Frag concat2 = concat();
		State *s = state(Split, 0, concat1.start, concat2.start);
		concat1 = frag(s, append(concat1.out, concat2.out));
	}
	return concat1;
}

Frag
concat()
{
	Frag repeat1 = repeat();
	while (look!=EOL && look!='|' && look!=')') {
		Frag repeat2 = repeat();
		patch(repeat1.out, repeat2.start);
		repeat1 = frag(repeat1.start, repeat2.out);
	}
	return repeat1;
}

Frag
repeat()
{
	Frag single1 = single();
	if (matchtoken('*')) {
		if (matchtoken('?')) {
			State *s = state(Split, 0, NULL, single1.start);
			patch(single1.out, s);
			return frag(s, list1(&s->out));
		} else {
			State *s = state(Split, 0, single1.start, NULL);
			patch(single1.out, s);
			return frag(s, list1(&s->out1));
		}
	} else if (matchtoken('+')) {
		if (matchtoken('?')) {
			State *s = state(Split, 0, NULL, single1.start);
			patch(single1.out, s);
			return frag(single1.start, list1(&s->out));
		} else {
			State *s = state(Split, 0, single1.start, NULL);
			patch(single1.out, s);
			return frag(single1.start, list1(&s->out1));
		}
	} else if (matchtoken('?')) {
		if (matchtoken('?')) {
			State *s = state(Split, 0, NULL, single1.start);
			return frag(s, append(single1.out, list1(&s->out)));
		} else {
			State *s = state(Split, 0, single1.start, NULL);
			return frag(s, append(single1.out, list1(&s->out1)));
		}
	}
	return single1;
}

Frag
single()
{
	if (matchtoken('(')) {
		if (matchtoken('?')) {
			if (matchtoken(':')) {
				Frag alt1 = alt();
				matchtoken(')');
				return alt1;
			}
		} else {
			int n = ++nparen;
			Frag alt1 = alt();
			matchtoken(')');
			return paren(alt1, n);
		}
	} else if (matchtoken('.')) {
		State *s = state(Any, 0, NULL, NULL);
		return frag(s, list1(&s->out));
	} else if (look == CHAR) {
		State *s = state(Char, yylval.c, NULL, NULL);
		move();
		return frag(s, list1(&s->out));
	} else if (look == CHARCLASS) {
		State *s = ccstate(CharClass, yylval.range, NULL, NULL);
		move();
		return frag(s, list1(&s->out));
	} else {
		yyerror("single");
	}
}

void
yyparse()
{
	move();
	line();
}

void
yyerror(const char *s)
{
	fprintf(stderr, "parse error: %s\n", s);
	exit(1);
}

void
printmatch(Sub *m, int n)
{
	int i;

	for(i=0; i<n; i++){
		if(m[i].sp && m[i].ep)
			printf("(%d,%d)", (int)(m[i].sp - text), (int)(m[i].ep - text));
		else if(m[i].sp)
			printf("(%d,?)", (int)(m[i].sp - text));
		else
			printf("(?,?)");
	}
}

void
dumplist(List *l)
{
	int i;
	Thread *t;

	for(i=0; i<l->n; i++){
		t = &l->t[i];
		if(t->state->op != Char && t->state->op != CharClass && t->state->op != Any && t->state->op != Match)
			continue;
		printf("  ");
		printf("%d ", t->state->id);
		printmatch(t->match, nparen+1);
		printf("\n");
	}
}

/*
 * Is match a longer than match b?
 * If so, return 1; if not, 0.
 */
int
longer(Sub *a, Sub *b)
{
	if(a[0].sp == NULL)
		return 0;
	if(b[0].sp == NULL || a[0].sp < b[0].sp)
		return 1;
	if(a[0].sp == b[0].sp && a[0].ep > b[0].ep)
		return 1;
	return 0;
}

/*
 * Add s to l, following unlabeled arrows.
 * Next character to read is p.
 */
void
addstate(List *l, State *s, Sub *m, const char *p)
{
	Sub save;

	if(s == NULL)
		return;

	if(s->lastlist == listid){
		switch(matchtype){
			case LeftmostBiased:
				if(reptype == RepeatMinimal || ++s->visits > 2)
					return;
				break;
			case LeftmostLongest:
				if(!longer(m, s->lastthread->match))
					return;
				break;
		}
	}else{
		s->lastlist = listid;
		s->lastthread = &l->t[l->n++];
		s->visits = 1;
	}
	if(s->visits == 1){
		s->lastthread->state = s;
		memmove(s->lastthread->match, m, NSUB*sizeof m[0]);
	}

	switch(s->op){
		case Split:
			/* follow unlabeled arrows */
			addstate(l, s->out, m, p);
			addstate(l, s->out1, m, p);
			break;

		case LParen:
			/* record left paren location and keep going */
			save = m[s->data.val];
			m[s->data.val].sp = p;
			m[s->data.val].ep = NULL;
			addstate(l, s->out, m, p);
			/* restore old information before returning. */
			m[s->data.val] = save;
			break;

		case RParen:
			/* record right paren location and keep going */
			save = m[s->data.val];
			m[s->data.val].ep = p;
			addstate(l, s->out, m, p);
			/* restore old information before returning. */
			m[s->data.val] = save;
			break;
	}
}

/*
 * Step the NFA from the states in clist
 * past the character c,
 * to create next NFA state set nlist.
 * Record best match so far in match.
 */
void
step(List *clist, int c, const char *p, List *nlist, Sub *match)
{
	int i;
	Thread *t;
	static Sub m[NSUB];

	if(debug){
		dumplist(clist);
		printf("%c (%d)\n", c, c);
	}

	listid++;
	nlist->n = 0;

	for(i=0; i<clist->n; i++){
		t = &clist->t[i];
		if(matchtype == LeftmostLongest){
			/*
			 * stop any threads that are worse than the
			 * leftmost longest found so far.  the threads
			 * will end up ordered on the list by start point,
			 * so if this one is too far right, all the rest are too.
			 */
			if(match[0].sp && match[0].sp < t->match[0].sp)
				break;
		}
		switch(t->state->op){
			case Char:
				if(c == t->state->data.val)
					addstate(nlist, t->state->out, t->match, p);
				break;

			case CharClass:
				if(t->state->data.range.flag[c])
					addstate(nlist, t->state->out, t->match, p);
				break;

			case Any:
				addstate(nlist, t->state->out, t->match, p);
				break;

			case Match:
				switch(matchtype){
					case LeftmostBiased:
						/* best so far ... */
						memmove(match, t->match, NSUB*sizeof match[0]);
						/* ... because we cut off the worse ones right now! */
						return;
					case LeftmostLongest:
						if(longer(t->match, match))
							memmove(match, t->match, NSUB*sizeof match[0]);
						break;
				}
				break;
		}
	}

	/* start a new thread if no match yet */
	if(match == NULL || match[0].sp == NULL)
		addstate(nlist, start, m, p);
}

/* Compute initial thread list */
List*
startlist(State *start, const char *p, List *l)
{
	List empty = {NULL, 0};
	step(&empty, 0, p, l, NULL);
	return l;
}

int
match(State *start, const char *p, Sub *m)
{
	int c;
	List *clist, *nlist, *t;

	clist = startlist(start, p, &l1);
	nlist = &l2;
	memset(m, 0, NSUB*sizeof m[0]);
	for(; *p && clist->n > 0; p++){
		c = *p & 0xFF;
		step(clist, c, p+1, nlist, m);
		t = clist; clist = nlist; nlist = t;
	}
	step(clist, 0, p, nlist, m);
	return m[0].sp != NULL;
}

void
dump(State *s)
{
	char nc;
	if(s == NULL || s->lastlist == listid)
		return;
	s->lastlist = listid;
	printf("%d| ", s->id);
	switch(s->op){
		case Char:
			printf("'%c' -> %d\n", s->data.val, s->out->id);
			break;

		case CharClass:
			nc = (s->data.range.type == CharExcluded) ? '^' : ' ';
			printf("[%c] -> %d\n", nc, s->out->id);
			break;

		case Any:
			printf(". -> %d\n", s->out->id);
			break;

		case Split:
			printf("| -> %d, %d\n", s->out->id, s->out1->id);
			break;

		case LParen:
			printf("( %d -> %d\n", s->data.val, s->out->id);
			break;

		case RParen:
			printf(") %d -> %d\n", s->data.val, s->out->id);
			break;

		case Match:
			printf("match\n");
			break;

		default:
			printf("??? %d\n", s->op);
			break;
	}

	dump(s->out);
	dump(s->out1);
}

static set<State *> freenodes;
void freenfa(State *state) {
	if (state == NULL)
		return;

	if (freenodes.count(state) == 0) {
		freenodes.insert(state);
		freenfa(state->out);
		freenfa(state->out1);
		free(state);
	}
}

vector<vector<int> >
findall(const char *regex, const char *content) {
	Sub m[NSUB];
	vector<vector<int> > result;

	input = regex;
	nparen = 0;
	yyparse();

	listid = 0;
	if(debug)
		dump(start);

	l1.t = (Thread *)malloc(nstate*sizeof l1.t[0]);
	l2.t = (Thread *)malloc(nstate*sizeof l2.t[0]);

	text = content; /* used by printmatch */
	const char *pos = content;
	while (*pos) {
		if(match(start, pos, m)){
			if (m[0].ep == m[0].sp) {
				pos++;
				continue;
			}

			vector<int> onematch;
			for (int i=0; i<=nparen; i++) {
				onematch.push_back((int)(m[i].sp-text));
				onematch.push_back((int)(m[i].ep-text));
			}
			result.push_back(onematch);

			pos = m[0].ep;
		} else{
			break;
		}
	}

	free(l1.t);
	free(l2.t);
	freenodes.clear();
	freenfa(start);

	return result;
}



//------------------------------------------------------------------

Parser::Parser(Lexer_Aql l_a_input, Lexer_Input l_i_input)
{
	this->l_a = l_a_input;
	this->l_i = l_i_input;
	vector<Aql_Token> aql_tokens_from_lexer_aql = l_a.get_aql_tokens();
	vector<Aql_Token> aql_stmt;
	input_tokens = l_i.get_lexer_input_tokens();

	for (int i = 0; i < aql_tokens_from_lexer_aql.size(); i++)
	{
		aql_stmt.push_back(aql_tokens_from_lexer_aql[i]);
		if (aql_tokens_from_lexer_aql[i].true_value == ";")
		{
			aql_stmts.push_back(aql_stmt);
			aql_stmt = vector<Aql_Token>();
		}
	}
}

void Parser::execute()
{
	for (int i = 0; i < aql_stmts.size(); i++)
	{
		execute_stmt = aql_stmts[i];
		aql_stmt(0);
	}
}

void Parser::aql_stmt(int pos)
{
	if (execute_stmt[pos].true_value == "create")
	{
		create_stmt(pos+1);
	}
	else if (execute_stmt[pos].true_value == "output") output_stmt(pos+1);
}

void Parser::output_stmt(int pos)
{
	string table_name_output = execute_stmt[pos + 1].true_kind;

	map<string, vector<Input_Token> > tables_output = tables[table_name_output];
	cout << "View: " << table_name_output << endl;


	vector<string> sub_table_names;
	for (map<string, vector<Input_Token> >::iterator it = tables_output.begin(); it != tables_output.end(); ++it)
	{
		sub_table_names.push_back(it->first);
	}
	
	int rows = tables_output[sub_table_names[0]].size();

	vector<int> tables_broder;
	for (int i = 0; i < sub_table_names.size(); i++)
	{
		int max_token = 0;
		for (int j = 0; j < rows; j++)
		{
			int tem_max_token;
			tem_max_token = tables_output[sub_table_names[i]][j].true_value.size();
			tem_max_token += number_to_string(tables_output[sub_table_names[i]][j].startpos).size();
			tem_max_token += number_to_string(tables_output[sub_table_names[i]][j].endpos).size();
			tem_max_token += 3;
			if (tem_max_token > max_token)
			{
				max_token = tem_max_token;
			}
		}
		tables_broder.push_back(max_token);
	}

	string broder;
	for (int i = 0; i < tables_broder.size(); i++)
	{
		broder += "+";
		for (int j = 0; j < tables_broder[i] + 2; j++) broder += "-";
	}
	broder += "+";

	cout << broder << endl;
	for (int i = 0; i < tables_broder.size(); i++)
	{
		cout << "| ";
		cout << sub_table_names[i];
		for (int j = sub_table_names[i].size() + 1; j < tables_broder[i] + 2; j++) cout << " ";
	}
	cout << "|" << endl;

	cout << broder << endl;
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < sub_table_names.size(); j++)
		{
			cout << "| ";
			cout << tables_output[sub_table_names[j]][i].true_value << '(' << tables_output[sub_table_names[j]][i].startpos << ',' << tables_output[sub_table_names[j]][i].endpos << ')';
			
			int tem_max_token;
			tem_max_token = tables_output[sub_table_names[j]][i].true_value.size();
			tem_max_token += number_to_string(tables_output[sub_table_names[j]][i].startpos).size();
			tem_max_token += number_to_string(tables_output[sub_table_names[j]][i].endpos).size();
			for (int k = tem_max_token + 4; k < tables_broder[j] + 2; k++) cout << " ";
		}
		cout << "|" << endl;
	}
	cout << broder << endl;
	cout << rows << " rows in set" << endl << endl;
}

void Parser::create_stmt(int pos)
{
	string table_name = execute_stmt[pos + 1].true_kind;

	map<string, string> table_map;
	int table_pos = pos;
	while (execute_stmt[table_pos].true_kind != "from")
	{
		table_pos++;
	}
	
	while (execute_stmt[table_pos].true_kind != ";" && table_pos < execute_stmt.size())
	{
		table_map[execute_stmt[table_pos + 2].true_kind] = execute_stmt[table_pos + 1].true_kind;
		table_pos += 3;
	}
	
	pos += 3;
	if (execute_stmt[pos].true_kind == "select")
	{
		tables[table_name] = select_stmt(pos + 1, table_map);
	} else
if (execute_stmt[pos].true_kind == "extract")
{
		tables[table_name] = extract_stmt(pos + 1, table_map);
	}
}

map<string, vector<Input_Token> > Parser::extract_stmt(int pos, map<string, string> table_map)
{

	if (execute_stmt[pos].true_kind == "regex") return regex_spec(pos + 1);
	else if (execute_stmt[pos].true_kind == "pattern") return pattern_spec(pos + 1, table_map);
}

map<string, vector<Input_Token> > Parser::regex_spec(int pos)
{
	map<string, vector<Input_Token> > one_table;
	string regular_expression = execute_stmt[pos].true_kind;
	
	vector<Input_Token> table_items = regex_output_tokens(regular_expression);
	
	int table_name_pos = pos;
	while (execute_stmt[table_name_pos].true_kind != "return" && execute_stmt[table_name_pos].true_kind != "as")
	{
		table_name_pos++;
	}

	vector<string> table_name;
	for (int i = table_name_pos; i < execute_stmt.size(); i++)
	{
		if (execute_stmt[i].true_kind == "as") table_name.push_back(execute_stmt[i + 1].true_kind);
	}

	one_table[table_name[0]] = table_items;

	return one_table;
}

vector<Input_Token> Parser::regex_output_tokens(string regular_expression)
{
	vector<Input_Token> tokens;
	vector<vector<int> > from_regex_engine;
	string input_file_buffer = l_i.get_file_buffer();
	from_regex_engine = findall(regular_expression.c_str(), input_file_buffer.c_str());
	string true_value;

	for (int i = 0; i < from_regex_engine.size(); i++)
	{
		true_value = input_file_buffer.substr(from_regex_engine[i][0], from_regex_engine[i][1] - from_regex_engine[i][0]);
		tokens.push_back(Input_Token(from_regex_engine[i][0], from_regex_engine[i][1], true_value));
	}
	return tokens;
}

map<string, vector<Input_Token> > Parser::select_stmt(int pos, map<string, string> table_map)
{
	map<string, vector<Input_Token> > new_tables;
	while(execute_stmt[pos].true_value != "from")
	{
		string super_table_name = table_map[execute_stmt[pos].true_kind];  //大表代号
		string sub_table_of_super_table_name = execute_stmt[pos + 2].true_kind;  //大表子项
		pos += 3;

		string new_sub_table_name;
		if (execute_stmt[pos].true_kind == "as") {
			new_sub_table_name = execute_stmt[pos + 1].true_kind; //新表子项
			pos += 2;
		} else
		{
		   new_sub_table_name = sub_table_of_super_table_name;
		}
		new_tables[new_sub_table_name] = tables[super_table_name][sub_table_of_super_table_name];
	}
	return new_tables;
}

map<string, vector<Input_Token> > Parser::pattern_spec(int pos, map<string, string> table_map)
{
	map<string, vector<Input_Token> > result_table;
	vector<vector<Input_Token> > result_tokens;
	vector<vector<Input_Token> > match_result;

	pattern_execute(pos, table_map, result_tokens);

	match_result = match(result_tokens);
	while(execute_stmt[pos].true_kind != "return")
	{
		pos++;
	}

	while(execute_stmt[pos].true_kind != "from")
	{
		string sub_table_name;

		int number = execute_stmt[pos + 2].true_kind[0] - '0';

		sub_table_name = execute_stmt[pos + 4].true_kind;
		result_table[sub_table_name] = match_result[number];
		pos++;
		
		while(execute_stmt[pos].true_kind != "and" && execute_stmt[pos].true_kind != "from")
		{
			pos++;
		}
	}

	return result_table;
}

vector<vector<Input_Token> > Parser::match(vector<vector<Input_Token> > result_tokens)
{
	vector<vector<Input_Token> > match_result;
	int group_numbers = 0;
	vector<Input_Token> links_tem;

	for(int j = 0; j < result_tokens[0].size(); j++)
	{
		if(result_tokens[0][j].true_value == "group_start")
		{
			group_numbers++;
		}
	}

	vector<vector<Input_Token> > groups_tem(group_numbers);

	for(int i = 0; i < result_tokens.size(); i++)
	{
		string result_tokens_value;
		int startpos;

		for(int j = 0; j < result_tokens[i].size(); j++)
		{
			if(result_tokens[i][j].true_value == "group_start" || result_tokens[i][j].true_value == "group_end")
			{
				continue;
			}
			result_tokens_value += result_tokens[i][j].true_value;
		}

		if(result_tokens[i][0].startpos == -1)
		{
			startpos = result_tokens[i][1].startpos;
		}
		else
		{
			startpos = result_tokens[i][0].startpos;
		}

		Input_Token link_result_tokens = Input_Token(startpos, result_tokens[i].size() - 1, result_tokens_value);
		links_tem.push_back(link_result_tokens);
	}

	match_result.push_back(links_tem);

	for(int i = 0; i < result_tokens.size(); i++)
	{
		int group_count = 0;
		for(int j = 0; j < result_tokens[i].size(); j++)
		{
			if(result_tokens[i][j].true_value == "group_start")
			{
				string true_value;
				int tem = j + 1;
				while(result_tokens[i][tem].true_value != "group_end")
				{
					true_value += result_tokens[i][tem].true_value;
					tem++;
				}
				groups_tem[group_count].push_back(Input_Token(result_tokens[i][j+1].startpos, result_tokens[i][tem-1].endpos, true_value));
				group_count++;
			}
		}
	}

	for(int i = 0; i < group_numbers; i++)
	{
		match_result.push_back(groups_tem[i]);
	}
	return match_result;
}

void Parser::pattern_execute(int pos, map<string, string> table_map, vector<vector<Input_Token> > &result_tokens)
{
	string group_tag = "no_tag";
	if(execute_stmt[pos].true_kind == "(")
	{
		group_tag = "(";
		pos++;
	}

	if(execute_stmt[pos].true_kind == ")")
	{
		group_tag = ")";
		pos++;
		if(execute_stmt[pos].true_kind == "(")
		{
			for(int i = 0; i < result_tokens.size(); i++)
			{
				Input_Token tem = Input_Token(-1, -1, "group_end");
				tem.startpos = result_tokens[i][result_tokens[i].size() - 1].startpos;
				tem.endpos = result_tokens[i][result_tokens[i].size() - 1].endpos;
				result_tokens[i].push_back(tem);
			}		
		}
	}

	if(execute_stmt[pos].true_value == "REG")
	{
		result_tokens = deal_with_regex(result_tokens, pos, group_tag);
		pos ++;
	}

	if(execute_stmt[pos].true_kind == "<")
	{
		pos++;
		if(execute_stmt[pos].true_kind == "Token")
		{
			result_tokens = deal_with_token(result_tokens, pos, group_tag);
		}

		else if(execute_stmt[pos].true_value == "ID")
		{
			result_tokens = deal_with_ID(result_tokens, pos, group_tag, table_map);
		}

		while(execute_stmt[pos].true_kind != "<" && execute_stmt[pos].true_kind != "(" && execute_stmt[pos].true_kind != "return" && execute_stmt[pos].true_value != "REG" && execute_stmt[pos].true_kind != ")")
			{
				pos++;
			}
	}
	
	if(execute_stmt[pos].true_kind == "return" && group_tag == ")")
	{
		for(int i = 0; i < result_tokens.size(); i++)
		{
			Input_Token tem = Input_Token(-1, -1, "group_end");
			tem.startpos = result_tokens[i][result_tokens[i].size() - 1].startpos;
			tem.endpos = result_tokens[i][result_tokens[i].size() - 1].endpos;
			result_tokens[i].push_back(tem);
		}
	}

	if(execute_stmt[pos].true_kind != "return")
	{
		pattern_execute(pos, table_map, result_tokens);
	}

}

vector<vector<Input_Token> > Parser::deal_with_regex(vector<vector<Input_Token> > result_tokens,int pos, string group_tag)
{
	vector<vector<Input_Token> > new_result_tokens;
	string regular_expression = execute_stmt[pos].true_kind;
	vector<Input_Token> vector_items = regex_output_tokens(regular_expression);

	if(result_tokens.size() == 0)
	{
		new_result_tokens = chushihua(group_tag, vector_items);
	}
	else
	{
		new_result_tokens = link(result_tokens, vector_items, group_tag);
	}

	return new_result_tokens;
}

vector<vector<Input_Token> > Parser::deal_with_token(vector<vector<Input_Token> > result_tokens,int pos, string group_tag)
{
	vector<vector<Input_Token> > new_result_tokens;
	int min, max;

	if(group_tag == "(")
	{
		for(int i = 0; i < result_tokens.size(); i++)
		{
			Input_Token tem = Input_Token(-1, -1, "group_start");
			tem.startpos = result_tokens[i][result_tokens[i].size() - 1].startpos;
			tem.endpos = result_tokens[i][result_tokens[i].size() - 1].endpos;
			result_tokens[i].push_back(tem);
		}
	}
	if(group_tag == ")")
	{
		for(int i = 0; i < result_tokens.size(); i++)
		{
			Input_Token tem = Input_Token(-1, -1, "group_end");
			tem.startpos = result_tokens[i][result_tokens[i].size() - 1].startpos;
			tem.endpos = result_tokens[i][result_tokens[i].size() - 1].endpos;
			result_tokens[i].push_back(tem);
		}
	}
	if(execute_stmt[pos + 2].true_kind == "{")
	{
		min = string_to_number(execute_stmt[pos + 3].true_kind);
		max = string_to_number(execute_stmt[pos + 5].true_kind);
	}
	else
	{
		min = 1;
		max = 1;
	}

	for(int i = min; i <= max; i++)
	{
		for(int j = 0; j < result_tokens.size(); j++)
		{
			int k = result_tokens[j].size() - 1;
			for(int m = 0; m < input_tokens.size() - i; m++)
			{
				if(input_tokens[m].startpos == result_tokens[j][k].startpos)
				{
					vector<Input_Token> temp = result_tokens[j];
					for(int count = 1; count <= i; count++)
					{
						Input_Token tem = input_tokens[m + count];
						temp.push_back(tem);
					}
					new_result_tokens.push_back(temp);
				}
			}
		}
	}
	return new_result_tokens;
}

vector<vector<Input_Token> > Parser::deal_with_ID(vector<vector<Input_Token> > result_tokens,int pos, string group_tag, map<string,string> table_map)
{
	vector<vector<Input_Token> > new_result_tokens;
	vector<Input_Token> vector_items = tables[table_map[execute_stmt[pos].true_kind]][execute_stmt[pos + 2].true_kind];

	if(result_tokens.size() == 0)
	{
		new_result_tokens = chushihua(group_tag, vector_items);
	}

	else
	{
		new_result_tokens = link(result_tokens, vector_items, group_tag);
	}
	return new_result_tokens;
}

vector<vector<Input_Token> > Parser::link(vector<vector<Input_Token> > result_tokens, vector<Input_Token> vector_items, string group_tag)
{
	vector<vector<Input_Token> > new_result_tokens;

	if(group_tag == "(")
	{
		for(int i = 0; i < result_tokens.size(); i++)
		{
			Input_Token tem = Input_Token(-1, -1, "group_start");
			tem.startpos = result_tokens[i][result_tokens[i].size() - 1].startpos;
			tem.endpos = result_tokens[i][result_tokens[i].size() - 1].endpos;
			result_tokens[i].push_back(tem);
		}
	}
	if(group_tag == ")")
	{
		for(int i = 0; i < result_tokens.size(); i++)
		{
			Input_Token tem = Input_Token(-1, -1, "group_end");
			tem.startpos = result_tokens[i][result_tokens[i].size() - 1].startpos;
			tem.endpos = result_tokens[i][result_tokens[i].size() - 1].endpos;
			result_tokens[i].push_back(tem);
		}
	}
	for(int i = 0; i < result_tokens.size(); i++)
	{
		int k = result_tokens[i].size() - 1;
		for(int j = 0; j < vector_items.size(); j++)
		{
			for(int m = 0; m < input_tokens.size(); m++)
			{
				if(input_tokens[m].endpos == result_tokens[i][k].endpos && input_tokens[m + 1].startpos == vector_items[j].startpos)
				{
					vector<Input_Token> tem;
					for(int count = 0; count <= k; count++)
					{
						tem.push_back(result_tokens[i][count]);
					}
					tem.push_back(vector_items[j]);
					new_result_tokens.push_back(tem);
				}
			}
		}
	}
	

	return new_result_tokens;
}

vector<vector<Input_Token> > Parser::chushihua(string group_tag, vector<Input_Token> vector_items)
{
	vector<vector<Input_Token> > new_result_tokens;
	if(group_tag == "(")
		{
			for(int i = 0; i < vector_items.size(); i++)
			{
				vector<Input_Token> tem;
				tem.push_back(Input_Token(-1, -1, "group_start"));
				tem.push_back(vector_items[i]);
				new_result_tokens.push_back(tem);
			}
		}
	else
	{
		for(int i = 0; i < vector_items.size(); i++)
		{
			vector<Input_Token> tem;
			tem.push_back(vector_items[i]);
			new_result_tokens.push_back(tem);
		}
	}
	return new_result_tokens;
}


string Parser::number_to_string(int number_for_string)
{
	string result;
	string execute_string;
	
	if(number_for_string == 0)
	{
		result = "0";
	}
	
	else
	{
		while(number_for_string != 0)
		{
			char tem;
			int digit;
			digit = number_for_string % 10;
			number_for_string /= 10;
			tem = digit + '0';
			execute_string += tem;
		}
		
		for(int i = 0; i < execute_string.size(); i++)
		{
			result += execute_string[execute_string.size() - i - 1];
		}
	}
	return result;
}

int Parser::string_to_number(string string_for_number)
{
	int result_number = 0;
	for(int i = 0; i < string_for_number.size(); i++)
	{
		int tem;
		tem = string_for_number[i] - '0';
		result_number = result_number * 10 +tem;
	}
	return result_number;
}