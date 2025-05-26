#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ki.h"

int main() {
	char *program = "read dup echo; while (dup) do (dup; echo);";
	
	struct LexedBlock L = ki_lex_analyze(program);
	printf("L:\n");
	ki_lex_dump(L, 1);
	
	struct ParsedBlock P1 = ki_parse_analyze(L);
	printf("P1:\n");
	ki_parse_dump(P1, 1);
	
	struct ParsedBlock P2 = ki_parse_analyze_pass2(P1);
	printf("P2:\n");
	ki_parse_dump(P2, 1);
	
	struct Ki K = ki_compile(P2);
	printf("K:\n");
	ki_prg_dump(K);
}