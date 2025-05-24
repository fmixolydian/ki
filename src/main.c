#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ki.h"

int main() {
	char *program = "read dup echo; foo bar; one two three;";
	
	struct LexedBlock L = ki_lex_analyze(program);
	printf("L:\n");
	ki_lex_dump(L);
	
	struct ParsedBlock P = ki_parse_analyze(L);
	printf("P:\n");
	ki_parse_dump(P);
	
}