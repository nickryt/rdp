/*
     CFG for tinyL LANGUAGE

     PROGRAM ::= STMTLIST .
     STMTLIST ::= STMT MORESTMTS
     MORESTMTS ::= ; STMTLIST | epsilon
     STMT ::= ASSIGN | READ | PRINT
     ASSIGN ::= VARIABLE = EXPR
     READ ::= ! VARIABLE
     PRINT ::= # VARIABLE
     EXPR ::= + EXPR EXPR |
              - EXPR EXPR |
              * EXPR EXPR |
              & EXPR EXPR |
              ^ EXPR EXPR |
              VARIABLE |
              DIGIT
     VARIABLE ::= a | b | c | d | e
     DIGIT ::= 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9

     NOTE: tokens are exactly a single character long

     Example expressions:
           +12.
           &1b.
           +*34-78.
           -*+1^2a58.

     Example programs;
         !a;!b;c=+3*ab;d=+c1;#d.
         b=-*+1&2a58;#b.
*/

// -------------------------------------------- | 

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "Instr.h"
#include "InstrUtils.h"
#include "Utils.h"

#define MAX_BUFFER_SIZE 500
#define EMPTY_FIELD 0xFFFFF
#define token *buffer

// -------------------------------------------- |  GLOBAL VARS

static char *buffer = NULL;  	/* Read buffer */
static int regnum = 1;			  /* For next free virtual register number */
static FILE *outfile = NULL;	/* Output of code generation */

// -------------------------------------------- | UTILITIES

static void CodeGen(OpCode opcode, int field1, int field2, int field3);
static inline void next_token();
static inline int next_register();
static inline int is_digit(char c);
static inline int to_digit(char c);
static inline int is_identifier(char c);
static char *read_input(FILE * f);

// -------------------------------------------- | ROUTINES FOR RECURSIVE DESCENDING PARSER LL(1)

static void program();
static void stmtlist();
static void morestmts();
static void stmt();
static void assign();
static void read();
static void print();
static int expr();
static int variable();
static int digit();

// .*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*. | ROUTINES

static int digit(){
	int reg;
	if (!is_digit(token)) {
		ERROR("Expected digit\n");
		exit(EXIT_FAILURE);
	}
	reg = next_register();
	CodeGen(LOADI, reg, to_digit(token), EMPTY_FIELD);
	next_token();
	return reg;
}

static int variable(){
	int reg;
	if (!is_identifier(token)) {
		ERROR("Expected identifier\n");
		exit(EXIT_FAILURE);
	}
	reg = next_register();
	CodeGen(LOAD, reg, token, EMPTY_FIELD);
	next_token();
	return reg;
}

static int expr(){
	int reg, left_reg, right_reg;

	// + <expr> <expr>
	if(token == '+'){
		next_token(); // Skip Token '+'
		left_reg = expr();
		right_reg = expr();
		reg = next_register();
		CodeGen(ADD, reg, left_reg, right_reg);
		return reg;
	}

	// - <expr> <expr>
	if(token == '-'){
		next_token(); // Skip Token '-'
		left_reg = expr();
		right_reg = expr();
		reg = next_register();
		CodeGen(SUB, reg, left_reg, right_reg);
		return reg;
	}

	// * <expr> <expr>
	if(token == '*'){
		next_token(); // Skip Token '*'
		left_reg = expr();
		right_reg = expr();
		reg = next_register();
		CodeGen(MUL, reg, left_reg, right_reg);
		return reg;
	}

	// & <expr> <expr>
	if(token == '&'){
		next_token(); // Skip Token '&'
		left_reg = expr();
		right_reg = expr();
		reg = next_register();
		CodeGen(AND, reg, left_reg, right_reg);
		return reg;
	}

	// ^ <expr> <expr>
	if(token == '^'){
		next_token(); // Skip Token '^'
		left_reg = expr();
		right_reg = expr();
		reg = next_register();
		CodeGen(XOR, reg, left_reg, right_reg);
		return reg;
	}

	// <variable>
	if(token == 'a' || token == 'b' || token == 'c' || token == 'd' || token == 'e'){
		return variable();
	}

	// <digit>
	if(token == '0' || token == '1' || token == '2' || token == '3' || token == '4' ||
		token == '5' || token == '6' || token == '7' || token == '8' || token == '9'){
		return digit();
	}

	else{
		ERROR("Symbol '%c' Unknown - expr()\n", token);
		exit(EXIT_FAILURE);
	}
}

static void assign(){
	int right_reg;
	char lefttoken;

	// <variable> = <expr>
	if(token == 'a' || token == 'b' || token == 'c' || token == 'd' || token == 'e'){
		lefttoken = token;
		next_token(); // Skip Character Token (Current Token Is =)
		next_token(); // Skip Token '='
		right_reg = expr();
		CodeGen(STORE, lefttoken, right_reg, EMPTY_FIELD);
		return;
	}

	else{
		ERROR("Symbol '%c' Unknown - assign()\n", token);
		exit(EXIT_FAILURE);
	}
}

static void read(){
	// ! <variable>
	if(token == 'a' || token == 'b' || token == 'c' || token == 'd' || token == 'e'){
		CodeGen(READ, token, EMPTY_FIELD, EMPTY_FIELD);
		next_token();
		return;
	}

	else{
		ERROR("Symbol '%c' Unknown - read()\n", token);
		exit(EXIT_FAILURE);
	}
}

static void print(){
	// # <variable>
	if(token == 'a' || token == 'b' || token == 'c' || token == 'd' || token == 'e'){
		CodeGen(WRITE, token, EMPTY_FIELD, EMPTY_FIELD);
		next_token();
		return;
	}

	else{
		ERROR("Symbol '%c' Unknown - read()\n", token);
		exit(EXIT_FAILURE);
	}
}

static void stmt(){
	// <assign>
	if(token == 'a' || token == 'b' || token == 'c' || token == 'd' || token == 'e'){
		assign();
		return;
	}

	// ! <read>
	if(token == '!'){
		next_token(); //Skip Token '!'
		read();
		return;
	}

	// <print>
	if(token == '#'){
		next_token(); //Skip Token '#'
		print();
		return;
	}

	else{
		ERROR("Symbol '%c' Unknown - stmt()\n", token);
		exit(EXIT_FAILURE);
	}
}

static void morestmts(){

	// ; <stmt_list>
	if(token == ';'){
		next_token(); //Skip Token ';'
		stmtlist();
	}

	// Epsillon
	else if(token != '.'){
		ERROR("Symbol '%c' Unknown - morestmts()\n(Expected '.')", token);
		exit(EXIT_FAILURE);
	}
}

static void stmtlist(){
	stmt();
	morestmts();
}

static void program(){
	stmtlist();
	if (token != '.') {
		ERROR("Program Error. Current Input Symbol Is %c\n", token);
		exit(EXIT_FAILURE);
	}
}

// .*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*. | UTILITIES



static void CodeGen(OpCode opcode, int field1, int field2, int field3){
	Instruction instr;

	if (!outfile) {
		ERROR("File Error\n");
		exit(EXIT_FAILURE);
	}
	instr.opcode = opcode;
	instr.field1 = field1;
	instr.field2 = field2;
	instr.field3 = field3;
	PrintInstruction(outfile, &instr);
}

static inline void next_token(){
	if (*buffer == '\0') {
		ERROR("End of program input\n");
		exit(EXIT_FAILURE);
	}
	printf("%c ", *buffer);
	if (*buffer == ';')
		printf("\n");
	buffer++;
	if (*buffer == '\0') {
		ERROR("End of program input\n");
		exit(EXIT_FAILURE);
	}
	if (*buffer == '.')
		printf(".\n");
}

static inline int next_register(){
	return regnum++;
}

static inline int is_digit(char c){
	if (c >= '0' && c <= '9')
		return 1;
	return 0;
}

static inline int to_digit(char c){
	if (is_digit(c))
		return c - '0';
	WARNING("Non-digit passed to %s, returning zero\n", __func__);
	return 0;
}

static inline int is_identifier(char c){
	if (c >= 'a' && c <= 'e')
		return 1;
	return 0;
}

static char *read_input(FILE * f){
	size_t size, i;
	char *b;
	int c;

	for (b = NULL, size = 0, i = 0;;) {
		if (i >= size) {
			size = (size == 0) ? MAX_BUFFER_SIZE : size * 2;
			b = (char *)realloc(b, size * sizeof(char));
			if (!b) {
						ERROR("Realloc failed\n");
						exit(EXIT_FAILURE);
			}
		}
		c = fgetc(f);
		if (EOF == c) {
					b[i] = '\0';
					break;
		}
		if (isspace(c))
					continue;
		b[i] = c;
		i++;
	}
	return b;
}

// .*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.*. | MAIN FUNCTION

int main(int argc, char *argv[]){

	//Create an output file for the code we will generate
	const char *outfilename = "tinyL.out";
	char *input;
	FILE *infile;

	printf("------------------------------------------------\n");
	printf("Compiler For TinyL\n");
	printf("------------------------------------------------\n");

	if (argc != 2) {
		ERROR("Use of command:\n  compile <tinyL file>\n");
		exit(EXIT_FAILURE);
	}

	infile = fopen(argv[1], "r");

	if (!infile) {
		ERROR("Cannot open input file \"%s\"\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	outfile = fopen(outfilename, "w");

	if (!outfile) {
		ERROR("Cannot open output file \"%s\"\n", outfilename);
		exit(EXIT_FAILURE);
	}

	//Pointer to a character in the infile
	input = read_input(infile);
	buffer = input;

	program();
	printf("\nCode written to file \"%s\".\n\n", outfilename);

	free(input);
	fclose(infile);
	fclose(outfile);
	return EXIT_SUCCESS;
}