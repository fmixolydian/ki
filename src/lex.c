#include "ki.h"

struct LexedBlock ki_lex_newblock() {
	struct LexedBlock prg = {0};
	
	return prg;
}

struct LexedBlock ki_lex_cutblock(struct LexedBlock src, int start, int end) {
	struct LexedBlock dest = ki_lex_newblock();
	struct LexedNode *P = src.tokens;
	
	for (int i=0; P; i++) {
		if (i >= start && i <= end) {
			struct LexedNode N = *P;
			N.next = NULL;
			ki_lex_push(&dest, N);
		}
		
		P = P->next;
	}
	
	return dest;
}


struct LexedNode *ki_lex_tail(struct LexedBlock *prg) {
	// grab linkedlist's tail
	struct LexedNode *P = prg->tokens;
	while (P->next) {
		P = P->next;
	}
	
	return P;
}

void ki_lex_push(struct LexedBlock *prg, struct LexedNode node) {
	struct LexedNode *P = malloc(sizeof node);
	if (!P) {perror("malloc"); abort();}
	*P = node;
	P->next = NULL;
	
	if (prg->tokens) {
		ki_lex_tail(prg)->next = P;
	} else {
		prg->tokens = P;
	}
}

int ki_strtok(char *dest, size_t len, char *src, char *delim) {
	int i;
	for (i=0; src[i] && i < (len - 1); i++) {
		if (strchr(delim, src[i])) break;
		dest[i] = src[i];
	}
	dest[i] = 0;
	
	return i;
}

void ki_strrev(char *dest, char *src, size_t n) {
	for (int i=0; i<n; i++) {
		dest[i] = src[(n - 1) - i];
	}
}

void ki_lex_error(char *prg, char *P, char *s) {
	fprintf(stderr, "\033[31mLEXER ERROR at %ld: %s\033[0m\n", P - prg, s);
	exit(1);
}

struct LexedBlock ki_lex_analyze(char *program) {
	struct LexedBlock prg = ki_lex_newblock();
	
	enum {MODE_LITERAL, MODE_STRING, MODE_COMMENT} mode = MODE_LITERAL;
	char *P = program;
	char buffer[256];
	
	while (*P) {
		int N;
		switch (mode) {
			case MODE_STRING:
				N = ki_strtok(buffer, sizeof buffer, P, "\"'");
				break;
			case MODE_LITERAL:
				N = ki_strtok(buffer, sizeof buffer, P, " \t\r\n'\",;()[]{}");
				break;
		}
		
		P += N;
		
		if (N > 0) {
			printf("%sread %d: '%s', DELIM is '%c'\n",
			        mode == MODE_LITERAL ? "" :
			        mode == MODE_STRING  ? "STR " :
			        mode == MODE_COMMENT ? "REM " : "", N, buffer, *P);
			
			// parse number
			
			// parse string
			if (mode == MODE_STRING) {
				
				if (*P == '\'') {
					if (N > 4) ki_lex_error(program, P, "multibyte char constant too long");
					uint32_t n = 0;
					uint32_t f = 0;
					strncpy(&n, buffer, 4);
					
					if (IS_LITTLE_ENDIAN) ki_strrev(&f, &n, N);
					
					printf("\tLITERAL_CHAR %x\n", f);
					ki_lex_push(&prg, (struct LexedNode) {
						.type = NODE_LITERAL_INTEGER,
						.val.i = f
					});
					
				} else {
					
					printf("\tLITERAL_STRING '%s'\n", buffer);
					ki_lex_push(&prg, (struct LexedNode) {
						.type = NODE_LITERAL_STRING,
						.val.s = strdup(buffer)
					});
					
				}
				
			} else if (mode == MODE_LITERAL) {
				
				if (isdigit(buffer[0]) || (buffer[0] == '.')) {
					if (strchr(buffer, '.')) {
						
						printf("\tLITERAL_FLOAT %f\n", atof(buffer));
						ki_lex_push(&prg, (struct LexedNode) {
							.type = NODE_LITERAL_FLOAT,
							.val.f = atof(buffer)
						});
						
					} else {
						
						printf("\tLITERAL_INT %d\n", atoi(buffer));
						ki_lex_push(&prg, (struct LexedNode) {
							.type = NODE_LITERAL_INTEGER,
							.val.i = atoi(buffer)
						});
						
					}
				} else if (buffer[0] == '$') {
					if (N < 2) ki_lex_error(program, P, "empty variable name");
					
					printf("\tVARPUSH '%s'\n", buffer + 1);
					ki_lex_push(&prg, (struct LexedNode) {
						.type = NODE_VARPUSH,
						.val.s = strdup(buffer + 1)
					});
					
				} else {
					
					printf("\tIDENTIFIER '%s'\n", buffer);
					ki_lex_push(&prg, (struct LexedNode) {
						.type = NODE_IDENTIFIER,
						.val.s = strdup(buffer)
					});
					
				}
				
			}
			
		}
		
		switch (*P) {
			case '\'':
			case '"':
				// if mode == MODE_COMMENT, we wouldn't've reached this code
				// and yes, that IS valid english grammar
				if (mode == MODE_STRING) mode = MODE_LITERAL;
				else                     mode = MODE_STRING;
				break;
			case ',':
			case ';':
				printf("\tSTATEMENT_SEPARATOR\n");
				ki_lex_push(&prg, (struct LexedNode) { .type = NODE_STATEMENT_SEP });
				break;
			case '(':
				printf("\tBRACKETS_START\n");
				ki_lex_push(&prg, (struct LexedNode) { .type = NODE_BLOCK_START });
				break;
			case ')':
				printf("\tBRACKETS_END\n");
				ki_lex_push(&prg, (struct LexedNode) { .type = NODE_BLOCK_END });
				break;
			case '\t':
			case '\r':
			case '\n':
			case ' ':
				break;
			
			case '\0':
				goto end_parse;
				
			default:
				printf("\tIGNORED (DELIM: '%c')\n", *P);
				break;
		}
		
		P++;
	}
	
	end_parse:
	printf("finished.\n");
	
	return prg;
}

const char *ki_lex_nodenames[] = {
	[NODE_NULL]             = "NULL",
	[NODE_IDENTIFIER]       = "ID",
	[NODE_VARPUSH]          = "PUSH VAR",
	[NODE_LITERAL_INTEGER]  = "PUSH",
	[NODE_LITERAL_FLOAT]    = "PUSH",
	[NODE_LITERAL_STRING]   = "PUSH",
	[NODE_BLOCK_START]      = "BLOCK START",
	[NODE_BLOCK_END]        = "BLOCK END",
	[NODE_STATEMENT_SEP]    = "STMT SEP",
};

void ki_lex_dump(struct LexedBlock prg) {
	struct LexedNode *P = prg.tokens;
	
	for (int i=0; P; i++) {
		printf("\t%3d NODE %-12s ", i, ki_lex_nodenames[P->type]);
		switch (P->type) {
			case NODE_NULL:
			case NODE_BLOCK_START:
			case NODE_BLOCK_END:
			case NODE_STATEMENT_SEP:
				break;
			
			case NODE_IDENTIFIER:
			case NODE_VARPUSH:
				printf("%s", P->val.s);
				break;
				
			case NODE_LITERAL_STRING:
				printf("'%s'", P->val.s);
				break;
				
			case NODE_LITERAL_INTEGER:
				printf("%d", P->val.i);
				break;
				
			case NODE_LITERAL_FLOAT:
				printf("%f", P->val.i);
				break;
			
			default:
				printf("???");
				break;
		}
		
		printf("\n");
		
		P = P->next;
	}
}
