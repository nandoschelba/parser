#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NONTERMINALS 28
#define MAX_TERMINALS 27
#define MAX_STACK 500
#define MAX_INPUT 8192

extern const char* nonTerminals[];
extern const char* terminals[];
extern int num_non_terminals;
extern int num_terminals;
extern const char* table[MAX_NONTERMINALS][MAX_TERMINALS];

int getNonTerminalIndex(const char* symbol);
int getTerminalIndex(const char* symbol);
int tokenize_production(const char *production, char tokens[][MAX_INPUT], int max_tokens);
void initialize_table();

#endif