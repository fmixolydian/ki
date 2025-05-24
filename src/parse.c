#include "ki.h"

// grabs last child of 2d linked list
struct ParsedStmt *ki_parse_block_htail(struct ParsedBlock *B) {
	struct ParsedStmt *P = B->child;
	
	if (!P) return NULL;
	while (P->next) {
		P = P->next;
	}
	
	return P;
}

// push a lexed statement to the end of a block (2d linked list)
void ki_parse_block_pushstmt(struct ParsedBlock *B, struct ParsedStmt S) {
	struct ParsedStmt *P = malloc(sizeof S);
	if (!P) {perror("malloc"); abort();}
	*P = S;
	P->next = NULL;
	P->type = 0;
	
	if (B->child) {
		ki_parse_block_htail(B)->next = P;
	} else {
		B->child = P;
	}
}

// push a lexed statement to the end of a block (2d linked list)
void ki_parse_block_pushlexstmt(struct ParsedBlock *B, struct ParsedLexedStmt LS) {
	struct ParsedLexedStmt *P = malloc(sizeof LS);
	if (!P) {perror("malloc"); abort();}
	*P = LS;
	P->next = NULL;
	P->type = 1;
	
	if (B->child) {
		ki_parse_block_htail(B)->next = P;
	} else {
		B->child = P;
	}
}

// push a lexed statement to the end of a block (2d linked list)
void ki_parse_block_pushblkstmt(struct ParsedBlock *B, struct ParsedBlockStmt BS) {
	struct ParsedBlockStmt *P = malloc(sizeof BS);
	if (!P) {perror("malloc"); abort();}
	*P = BS;
	P->next = NULL;
	P->type = 2;
	
	if (B->child) {
		ki_parse_block_htail(B)->next = P;
	} else {
		B->child = P;
	}
}

void ki_parse_error(char *s) {
	printf("PARSE ERROR: %s\n", s);
	exit(1);
}

struct ParsedWord *ki_parse_stmt_htail(struct ParsedStmt *S) {
	struct ParsedWord *P;
	
	P = S->child;
	if (!P) return NULL;
	
	while (P->next) {
		P = P->next;
	}
	
	return P;
}

void ki_parse_stmt_pushwint(struct ParsedStmt *S, struct ParsedWordInt W) {
	struct ParsedWord *P = malloc(sizeof W);
	if (!P) {perror("malloc"); abort();}
	memcpy(P, &W, sizeof W);
	P->next = NULL;
	
	if (S->child) {
		ki_parse_stmt_htail(S)->next = P;
	} else {
		S->child = P;
	}
}

void ki_parse_stmt_pushwstr(struct ParsedStmt *S, struct ParsedWordStr W) {
	struct ParsedWord *P = malloc(sizeof W);
	if (!P) {perror("malloc"); abort();}
	memcpy(P, &W, sizeof W);
	P->next = NULL;
	
	if (S->child) {
		ki_parse_stmt_htail(S)->next = P;
	} else {
		S->child = P;
	}
}

void ki_parse_stmt_pushwflt(struct ParsedStmt *S, struct ParsedWordFloat W) {
	struct ParsedWord *P = malloc(sizeof W);
	if (!P) {perror("malloc"); abort();}
	memcpy(P, &W, sizeof W);
	P->next = NULL;
	
	if (S->child) {
		ki_parse_stmt_htail(S)->next = P;
	} else {
		S->child = P;
	}
}

void ki_parse_stmt_pushwblk(struct ParsedStmt *S, struct ParsedWordBlockSingle W) {
	struct ParsedWord *P = malloc(sizeof W);
	if (!P) {perror("malloc"); abort();}
	memcpy(P, &W, sizeof W);
	P->next = NULL;
	
	if (S->child) {
		ki_parse_stmt_htail(S)->next = P;
	} else {
		S->child = P;
	}
}

void ki_parse_stmt_pushwblks(struct ParsedStmt *S, struct ParsedWordBlocks W) {
	struct ParsedWord *P = malloc(sizeof W);
	if (!P) {perror("malloc"); abort();}
	memcpy(P, &W, sizeof W);
	P->next = NULL;
	
	if (S->child) {
		ki_parse_stmt_htail(S)->next = P;
	} else {
		S->child = P;
	}
}

const char *ki_parse_word_names[] = {
	[WORD_IDENTIFIER]      = "ID",
	[WORD_VARPUSH]         = "PUSH VAR",
	[WORD_LITERAL_INTEGER] = "PUSH",
	[WORD_LITERAL_FLOAT]   = "PUSH",
	[WORD_LITERAL_STRING]  = "PUSH",
	[WORD_BLOCK]           = "BLOCK",
	[WORD_VARPOP]          = "POP VAR",
	[WORD_CALL]            = "CALL",
	[WORD_IF]              = "BLOCK IF",
	[WORD_WHILE]           = "BLOCK WHILE",
	[WORD_FOR]             = "BLOCK FOR"
};

void ki_parse_dump_stmt(struct ParsedStmt S) {
	struct ParsedWord *P = S.child;
	if (P) {
		while (P) {
			printf("\tWORD %s ", ki_parse_word_names[P->type]);
			switch (P->type) {
				case WORD_IDENTIFIER:
				case WORD_VARPUSH:
				case WORD_VARPOP:
				case WORD_CALL:
					printf("%s ", ((struct ParsedWordStr*)P)->value);
					break;
				
				case WORD_LITERAL_STRING:
					printf("\"%s\" ", ((struct ParsedWordStr*)P)->value);
					break;
				
				case WORD_LITERAL_FLOAT:
					printf("%f ", ((struct ParsedWordFloat*)P)->value);
					break;
					
				case WORD_LITERAL_INTEGER:
					printf("%d ", ((struct ParsedWordInt*)P)->value);
					break;
				
				default:
					printf("???? ");
					break;
			}
			printf("\n");
			P = P->next;
		}
	} else {
		printf("\t(no children)\n");
	}
}

struct ParsedBlock ki_parse_analyze(struct LexedBlock L) {
	struct LexedNode *T;
	struct ParsedStmt       S = {0};
	struct ParsedBlock      B = {0};
	struct ParsedLexedStmt LS = {0};
	
	struct LexedBlock      SubL = {0};
	struct ParsedBlock     SubB = {0};
	
	struct ParsedWordStr         Ws = {0};
	struct ParsedWordInt         Wi = {0};
	struct ParsedWordFloat       Wf = {0};
	struct ParsedWordBlockSingle Wb = {0};
	struct ParsedWordBlocks      Wbk = {0};
	
	int SubStart, SubEnd;
	
	T = L.tokens;
	for (int i=0; T; i++) {
		switch (T->type) {
			case NODE_BLOCK_START:
				// parse sub block
				T = T->next; i++;
				if (!T) ki_parse_error("expected content in block");
				SubStart = i;
				
				// find next NODE_BLOCK_END
				while (T->type != NODE_BLOCK_END) {
					T = T->next;
					i++;
					if (!T) ki_parse_error("expected `)` to close block");
				}
				SubEnd = i - 1;
				printf("cut %d %d\n", SubStart, SubEnd);
				
				// extarct lexedblock of code in staement
				printf("lexed:\n");
				SubL = ki_lex_cutblock(L, SubStart, SubEnd);
				ki_lex_dump(SubL);
				
				// parse lexedblock (this is a recursive function btw)
				printf("parsed:\n");
				SubB = ki_parse_analyze(SubL);
				ki_parse_dump(SubB);
				
				// insert SubB into statement
				Wb.type = WORD_BLOCK;
				Wb.B = SubB;
				ki_parse_stmt_pushwblk(&S, Wb);
				
				break;
			
			case NODE_IDENTIFIER:
				Ws.type = WORD_IDENTIFIER;
				Ws.value = T->val.s;
				ki_parse_stmt_pushwstr(&S, Ws);
				break;
			
			case NODE_VARPUSH:
				Ws.type = WORD_VARPUSH;
				Ws.value = T->val.s;
				ki_parse_stmt_pushwstr(&S, Ws);
				break;
			
			case NODE_LITERAL_INTEGER:
				Wi.type = WORD_LITERAL_INTEGER;
				Wi.value = T->val.i;
				ki_parse_stmt_pushwint(&S, Wi);
				break;
			
			case NODE_LITERAL_FLOAT:
				Wf.type = WORD_LITERAL_FLOAT;
				Wf.value = T->val.f;
				ki_parse_stmt_pushwflt(&S, Wf);
				break;
			
			case NODE_STATEMENT_SEP:
				// push statement
				printf("STMT:\n");
				ki_parse_dump_stmt(S);
				ki_parse_block_pushstmt(&B, S);
				// clear stmt
				S.child = NULL;
				break;
		}
		
		T = T->next;
	}
	
	ki_parse_dump_stmt(S);
	ki_parse_block_pushstmt(&B, S);
	// clear stmt
	S.child = NULL;
}

void ki_parse_dump_r(struct ParsedBlock B, int level) {
	struct ParsedStmt *S = B.child;
	
	for (int i=0; S; i++) {
		switch (S->type) {
			case 0:
				printf("\tSTMT:\n");
				ki_parse_dump_stmt(*S);
				break;
			
			case 1:
				printf("\tLEXSTMT:\n");
				ki_lex_dump(((struct ParsedLexedStmt*)S)->B);
				break;
			
			case 2:
				printf("\tBLKSTMT:\n");
				break;
			
			default:
				printf("\t??????\n");
				break;
		}
		
		S = S->next;
	}
}

void ki_parse_dump(struct ParsedBlock B) {
	ki_parse_dump_r(B, 1);
}