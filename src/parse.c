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
	struct ParsedStmt *P = ki_memdup(&S, sizeof S);
	P->next = NULL;
	
	if (B->child) {
		ki_parse_block_htail(B)->next = P;
	} else {
		B->child = P;
	}
}

void ki_parse_error(char *s) {
	printf("\033[31mPARSE ERROR: %s\n", s);
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

void ki_parse_stmt_pushword(struct ParsedStmt *S, struct ParsedWord W) {
	struct ParsedWord *P = ki_memdup(&W, sizeof W);
	P->next = NULL;
	
	if (S->child) {
		ki_parse_stmt_htail(S)->next = P;
	} else {
		S->child = P;
	}
}

const char *ki_parse_word_names[] = {
	[WORD_VARPUSH]         = "PUSH VAR",
	[WORD_VARPOP]          = "POP VAR",
	[WORD_KEYWORD]         = "KEYWORD",
	[WORD_CALL]            = "CALL",
	[WORD_LITERAL_INTEGER] = "PUSH",
	[WORD_LITERAL_FLOAT]   = "PUSH",
	[WORD_LITERAL_STRING]  = "PUSH",
	[WORD_BLOCK]           = "BLOCK",
	[WORD_VARPOP]          = "POP VAR",
	[WORD_CALL]            = "CALL",
	[WORD_IF]              = "IF (A) THEN (B) [ELSE (C)]",
	[WORD_WHILE]           = "WHILE (A) DO (B)",
	[WORD_FOR]             = "FOR (A) TO (B) [STEP (C)] DO (C / D)"
};

void ki_parse_dump_stmt(struct ParsedStmt S, int indent) {
	struct ParsedWord *P = S.child;
	if (P) {
		while (P) {
			INDENT(indent);
			printf("WORD %s", ki_parse_word_names[P->type]);
			switch (P->type) {
				case WORD_VARPUSH:
				case WORD_VARPOP:
					printf(" %s\n", P->value.s);
					break;
					
				case WORD_CALL:
					printf(" %s", P->value.s);
					if (P->value.Bc) {
						printf(" ARGV:\n");
						ki_parse_dump(*(P->value.B), indent + 1);
					} else {
						printf("\n");
					}
					break;
				
				case WORD_LITERAL_STRING:
					printf(" \"%s\"\n", P->value.s);
					break;
				
				case WORD_LITERAL_FLOAT:
					printf(" %f\n", P->value.f);
					break;
					
				case WORD_LITERAL_INTEGER:
					printf(" %d\n", P->value.i);
					break;
				
				case WORD_BLOCK:
					printf(":\n");
					for (int i=0; i<P->value.Bc; i++) {
						ki_parse_dump(P->value.B[i], 3);
						INDENT(indent + 1);
						printf("----\n");
					}
					break;
					
				case WORD_IF:
				case WORD_WHILE:
				case WORD_FOR:
					printf(": \n");
					
					for (int i=0; i<P->value.Bc; i++) {
						INDENT(indent + 1);
						printf("%c:\n", 'A' + i);
						ki_parse_dump(P->value.B[i], indent + 2);
					}
					
					break;
				
				case WORD_KEYWORD:
					printf(" %s (%d)\n", ki_keyword_names[P->value.k], P->value.k);
					break;
				
				default:
					printf(" ????\n");
					break;
			}
			P = P->next;
		}
	} else {
		printf("\t(no children)\n");
	}
}

struct ParsedBlock ki_parse_analyze_pass1(struct LexedBlock L, int nest_level) {
	struct LexedNode *T;
	struct ParsedStmt       S = {0};
	struct ParsedBlock      B = {0};
	
	struct LexedBlock      SubL = {0};
	struct ParsedBlock     SubB = {0};
	
	struct ParsedWord         W = {0};
	
	int SubStart, SubEnd;
	int SubNL;
	
	T = L.tokens;
	for (int i=0; T; i++) {
		int nest_level = 0;
		switch (T->type) {
			case NODE_BLOCK_START:
				// parse sub block
				T = T->next; i++;
				if (!T) ki_parse_error("expected content in block");
				SubStart = i;
				
				// find next NODE_BLOCK_END
				SubNL = nest_level;
				while (!(T->type == NODE_BLOCK_END && SubNL == nest_level)) {
					switch (T->type) {
						case NODE_BLOCK_START: SubNL++; break;
						case NODE_BLOCK_END:   SubNL--; break;
					}
					T = T->next;
					i++;
					if (!T) ki_parse_error("expected `)` to close block");
				}
				SubEnd = i - 1;
				printf("cut %d %d\n", SubStart, SubEnd);
				
				// extarct lexedblock of code in staement
				printf("lexed:\n");
				SubL = ki_lex_cutblock(L, SubStart, SubEnd);
				ki_lex_dump(SubL, 1);
				
				// parse lexedblock (this is a recursive function btw)
				printf("parsed:\n");
				SubB = ki_parse_analyze(SubL);
				ki_parse_dump(SubB, 1);
				
				// insert SubB into statement
				memset(&W, 0, sizeof W);
				W.type = WORD_BLOCK;
				W.value.Bc = 1;
				W.value.B  = ki_memdup(&SubB, sizeof SubB);
				ki_parse_stmt_pushword(&S, W);
				break;
			
			case NODE_BLOCK_END:
				ki_parse_error("expected `(` to open block");
				break;
			
			case NODE_VARPOP:
				memset(&W, 0, sizeof W);
				W.type = WORD_VARPOP;
				W.value.s = T->val.s;
				ki_parse_stmt_pushword(&S, W);
				break;
			
			case NODE_CALL:
				memset(&W, 0, sizeof W);
				W.type = WORD_CALL;
				W.value.s = T->val.s;
				W.value.Bc = 0;
				ki_parse_stmt_pushword(&S, W);
				break;
			
			case NODE_VARPUSH:
				memset(&W, 0, sizeof W);
				W.type = WORD_VARPUSH;
				W.value.s = T->val.s;
				ki_parse_stmt_pushword(&S, W);
				break;
			
			case NODE_LITERAL_STRING:
				memset(&W, 0, sizeof W);
				W.type = WORD_LITERAL_STRING;
				W.value.s = T->val.s;
				ki_parse_stmt_pushword(&S, W);
				break;
			
			case NODE_LITERAL_INTEGER:
				memset(&W, 0, sizeof W);
				W.type = WORD_LITERAL_INTEGER;
				W.value.i = T->val.i;
				ki_parse_stmt_pushword(&S, W);
				break;
			
			case NODE_LITERAL_FLOAT:
				memset(&W, 0, sizeof W);
				W.type = WORD_LITERAL_FLOAT;
				W.value.f = T->val.f;
				ki_parse_stmt_pushword(&S, W);
				break;
			
			case NODE_KEYWORD:
				memset(&W, 0, sizeof W);
				W.type = WORD_KEYWORD;
				W.value.k = T->val.i;
				ki_parse_stmt_pushword(&S, W);
				break;
				
			case NODE_STATEMENT_SEP:
				// push statement
				printf("STMT:\n");
				ki_parse_dump_stmt(S, 1);
				ki_parse_block_pushstmt(&B, S);
				printf("P:\n");
				ki_parse_dump(B, 1);
				// clear stmt
				S.child = NULL;
				break;
			
			default:
				printf("UNKNOWN LEX %s\n", ki_lex_nodenames[T->type]);
		}
		
		T = T->next;
	}
	
	printf("\tSTMT FIN:\n");
	ki_parse_dump_stmt(S, 2);
	ki_parse_block_pushstmt(&B, S);
	// clear stmt
	S.child = NULL;
	
	// parse step 2
	
	return B;
}

struct ParsedWord *ki_parse_word_next_guarded(struct ParsedWord *W, ParsedWordType t, Keyword k) {
	W = W->next;
	if (!W) ki_parse_error("invalid syntax: word doesnt exist");
	if (W->type != t) ki_parse_error("invalid syntax: wrong type");
	if (t == WORD_KEYWORD) {
		if (W->value.k != k) ki_parse_error("invalid syntax: wrong keyword");
	}
	return W;
}

struct ParsedWord ki_parse_clone_blocksword(struct ParsedWord *W) {
	struct ParsedBlock *Bks;
	
	Bks = malloc(W->value.Bc * sizeof(struct ParsedBlock));
	for (int i=0; i<W->value.Bc; i++) {
		printf("RECURSE BLOCK %d\n", i);
		Bks[i] = ki_parse_analyze_pass2(W->value.B[i]);
	}
	return (struct ParsedWord) {
		.type     = W->type,
		.value.Bc = W->value.Bc,
		.value.B  = Bks
	};
}

// is returning a whole new ParsedBlock the best option? No.
// does it work? Yes.
struct ParsedBlock ki_parse_analyze_pass2(struct ParsedBlock SrcB) {
	// pass 2: merge while/if/etc statements with their following blocks
	struct ParsedStmt *S = SrcB.child;
	struct ParsedBlock DestB = {0};
	
	while (S) {
		struct ParsedWord *W = S->child;
		struct ParsedStmt DestS = {0};
		struct ParsedBlock *Blks;
		
		struct ParsedBlock Ba;
			// if: condition
			// while: condition
			// for: init
		struct ParsedBlock Bb;
			// if: then
			// while: do
			// for: limit
		struct ParsedBlock Bc;
			// if: else
			// for: step
		struct ParsedBlock Bd;
			// for: body
		
		char *Str;
		
		while (W) {
			switch (W->type) {
				case WORD_KEYWORD:
					switch (W->value.k) {
						case KW_IF:
							printf("IF\n");
							// cond
							W = ki_parse_word_next_guarded(W, WORD_BLOCK, 0);
							Ba = ki_parse_analyze_pass2(W->value.B[0]);
							// then
							W = ki_parse_word_next_guarded(W, WORD_KEYWORD, KW_THEN);
							W = ki_parse_word_next_guarded(W, WORD_BLOCK, 0);
							Bb = ki_parse_analyze_pass2(W->value.B[0]);
							if (W->next) {
								// separate if statement, or the program will explode
								if (W->next->type == WORD_KEYWORD && W->next->value.k == KW_ELSE) {
									printf("WITH ELSE\n");
									W = ki_parse_word_next_guarded(W, WORD_KEYWORD, KW_ELSE);
									W = ki_parse_word_next_guarded(W, WORD_BLOCK, 0);
									Bc = ki_parse_analyze_pass2(W->value.B[0]);
								} else Bc.child = NULL;
							} else Bc.child = NULL;
							
							printf("A:\n");
							ki_parse_dump(Ba, 1);
							printf("B:\n");
							ki_parse_dump(Bb, 1);
							printf("C:\n");
							ki_parse_dump(Bc, 1);
							
							Blks = calloc(3, sizeof(struct ParsedBlock));
							Blks[0] = Ba;
							Blks[1] = Bb;
							Blks[2] = Bc;
							
							ki_parse_stmt_pushword(&DestS, (struct ParsedWord) {
								.type = WORD_IF,
								.value.Bc   = Bc.child ? 3 : 2,
								.value.B    = Blks
							});
							
							break;
						
						case KW_WHILE:
							Blks = calloc(2, sizeof(struct ParsedBlock));
							
							// cond
							W = ki_parse_word_next_guarded(W, WORD_BLOCK, 0);
							Blks[0] = ki_parse_analyze_pass2(W->value.B[0]);
							// body
							W = ki_parse_word_next_guarded(W, WORD_KEYWORD, KW_DO);
							W = ki_parse_word_next_guarded(W, WORD_BLOCK, 0);
							Blks[1] = ki_parse_analyze_pass2(W->value.B[0]);
							
							ki_parse_stmt_pushword(&DestS, (struct ParsedWord) {
								.type = WORD_WHILE,
								.value.Bc   = 2,
								.value.B    = Blks
							});
							
							break;
						
						default:
							printf("OTHERKW %s\n", ki_keyword_names[W->value.k]);
							ki_parse_stmt_pushword(&DestS, *W);
					}
					break;
				
				case WORD_BLOCK:
					ki_parse_stmt_pushword(&DestS, ki_parse_clone_blocksword(W));
					break;
				
				case WORD_CALL:
					if (W->next) {
						if (W->next->type == WORD_BLOCK) {
							
							Str = W->value.s;
							W = ki_parse_word_next_guarded(W, WORD_BLOCK, 0);
							
							Blks = ki_memdup(&W->value.B, sizeof(struct ParsedBlock));
							
							ki_parse_stmt_pushword(&DestS, (struct ParsedWord) {
								.type = WORD_CALL,
								.value.s  = Str,
								.value.Bc = 1,
								.value.B  = Blks
							});
							
						} else ki_parse_stmt_pushword(&DestS, *W);
					} else ki_parse_stmt_pushword(&DestS, *W);
					break;
				
				default:
					printf("OTHERTYPE %s\n", ki_parse_word_names[W->type]);
					ki_parse_stmt_pushword(&DestS, *W);
					
			}
			
			W = W->next;
		}
		
		ki_parse_block_pushstmt(&DestB, DestS);
		DestS.child = NULL;
		
		S = S->next;
	}
	
	return DestB;
}

struct ParsedBlock ki_parse_analyze(struct LexedBlock L) {
	struct ParsedBlock B = ki_parse_analyze_pass1(L, 0);
	return B;
	return ki_parse_analyze_pass2(B);
}

void ki_parse_dump(struct ParsedBlock B, int indent) {
	struct ParsedStmt *S = B.child;
	
	for (int i=0; S; i++) {
		INDENT(indent);
		printf("%d) STMT:\n", i + 1);
		ki_parse_dump_stmt(*S, indent + 1);
		
		S = S->next;
	}
}