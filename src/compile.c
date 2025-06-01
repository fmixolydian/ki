#include "ki.h"
#include <stdarg.h>

#define MAX_OBJECT_SIZE 4096
#define MERGE_CONSTANTS ON_COUNT(5)

// DO NOT CALL THIS on an empty linked list
struct CompiledNode *ki_compile_ctable_tail(struct CompiledTable *CT) {
	struct CompiledNode *P = CT->child;
	while (P->next) {
		P = P->next;
	}
	
	return P;
}

void ki_compile_pushct(struct CompiledTable *CT, struct CompiledNode C) {
	struct CompiledNode *P = malloc(sizeof C);
	*P = C;
	P->next = NULL;
	
	if (CT->child) {
		struct CompiledNode *N = ki_compile_ctable_tail(CT);
		N->next = P;
	} else {
		CT->child = P;
	}
	
	CT->len++;
}

int ki_compile_cnode_float(struct CompiledTable *CT, float n) {
	struct CompiledNode *CN = CT->child;
	
	for (int i=0; CN; i++) {
		if (CN->data.type == KI_FLOAT) {
			if (CN->data.value.f == n) {
				return i;
			}
		}
		
		if (CN->next) {
			CN = CN->next;
		} else {
			struct CompiledNode N = (struct CompiledNode) {
				.data.type = KI_FLOAT,
				.data.value.f = n
			};
			
			CN->next = ki_memdup(&N, sizeof N);
			CN = CN->next;
		}
	}
	
	struct CompiledNode N = (struct CompiledNode) {
		.data.type = KI_FLOAT,
		.data.value.f = n
	};
	
	CT->child = ki_memdup(&N, sizeof N);
	return 1;
}


int ki_compile_cnode_int(struct CompiledTable *CT, int n) {
	struct CompiledNode *CN = CT->child;
	
	for (int i=0; CN; i++) {
		if (CN->data.type == KI_INT) {
			if (CN->data.value.i == n) {
				return i;
			}
		}
		
		if (CN->next) {
			CN = CN->next;
		} else {
			struct CompiledNode N = (struct CompiledNode) {
				.data.type = KI_INT,
				.data.value.i = n
			};
			
			CN->next = ki_memdup(&N, sizeof N);
			CN = CN->next;
		}
	}
	
	struct CompiledNode N = (struct CompiledNode) {
		.data.type = KI_INT,
		.data.value.i = n
	};
	
	CT->child = ki_memdup(&N, sizeof N);
	return 1;
}

int ki_compile_cnode_str(struct CompiledTable *CT, char *str) {
	struct CompiledNode *CN = CT->child;
	
	for (int i=1; CN; i++) {
		if (CN->data.type == KI_STR) {
			if (!strcmp(CN->data.value.s, str)) {
				return i;
			}
		}
		
		if (CN->next) {
			CN = CN->next;
		} else {
			struct CompiledNode N = (struct CompiledNode) {
				.data.type = KI_STR,
				.data.value.s = strdup(str)
			};
			
			CN->next = ki_memdup(&N, sizeof N);
			CN = CN->next;
		}
	}
	
	struct CompiledNode N = (struct CompiledNode) {
		.data.type = KI_STR,
		.data.value.s = strdup(str)
	};
	
	CT->child = ki_memdup(&N, sizeof N);
	return 1;
}

void ki_compile_error(char *s) {
	fprintf(stderr, "\033[31mCOMPILE ERROR: %s\033[0m\n", s);
	exit(1);
}

size_t ki_compile_block_countstmt(struct ParsedBlock B) {
	struct ParsedStmt *S = B.child;
	
	int i=0;
	while (S) {
		if (S->child) i++;
		S = S->next;
	}
	
	return i;
}

struct {size_t len; uint8_t data[16]; } ki_compile_kwasm[] = {
	[KW_ADD]   = {.len = 1, .data = {OP_ADD, 0}},
	[KW_SUB]   = {.len = 1, .data = {OP_SUB, 0}},
	[KW_MUL]   = {.len = 1, .data = {OP_MULT, 0}},
	[KW_DIV]   = {.len = 1, .data = {OP_DIV, 0}},
	[KW_MOD]   = {.len = 1, .data = {OP_MOD, 0}},
	[KW_NEG]   = {.len = 1, .data = {OP_NEG, 0}},
	[KW_ABS]   = {.len = 1, .data = {OP_ABS, 0}},
	[KW_MAX]   = {.len = 1, .data = {OP_MAX, 0}},
	[KW_MIN]   = {.len = 1, .data = {OP_MIN, 0}},
	[KW_INC]   = {.len = 1, .data = {OP_INC, 0}},
	[KW_DEC]   = {.len = 1, .data = {OP_DEC, 0}},
	[KW_DUP]   = {.len = 1, .data = {OP_DUP, 0}},
	[KW_DROP]  = {.len = 1, .data = {OP_DROP, 0}},
	[KW_SWAP]  = {.len = 1, .data = {OP_SWAP, 0}},
	[KW_ROR]   = {.len = 1, .data = {OP_ROR, 0}},
	[KW_ROL]   = {.len = 1, .data = {OP_ROL, 0}},
	[KW_C_OR]  = {.len = 1, .data = {OP_OR, 0}},
	[KW_C_AND] = {.len = 1, .data = {OP_AND, 0}},
	[KW_C_XOR] = {.len = 1, .data = {OP_XOR, 0}},
	[KW_C_NOT] = {.len = 1, .data = {OP_NOT, 0}},
	[KW_C_EQ]  = {.len = 1, .data = {OP_CMPEQ, 0}},
	[KW_C_NE]  = {.len = 1, .data = {OP_CMPNE, 0}},
	[KW_C_LT]  = {.len = 1, .data = {OP_CMPLT, 0}},
	[KW_C_GT]  = {.len = 1, .data = {OP_CMPGT, 0}}
};

size_t ki_compile_stmt(uint8_t *bin, struct CompiledTable *CT, struct ParsedStmt S) {
	size_t offset = 0;
	struct ParsedWord *W = S.child;
	
	uint8_t bin_A[4096];
	size_t offset_a = 0;
	
	uint8_t bin_B[4096];
	size_t offset_b = 0;
	
	while (W) {
		struct ParsedStmt *P;
		int CNslot, argc;
		
		switch (W->type) {
			case WORD_VARPUSH:
				CNslot = ki_compile_cnode_str(CT, W->value.s);
				offset += ki_compile_writefmt(bin+offset, "bw", OP_PUSHV, CNslot);
				break;
			
			case WORD_VARPOP:
				CNslot = ki_compile_cnode_str(CT, W->value.s);
				offset += ki_compile_writefmt(bin+offset, "bw", OP_POPV, CNslot);
				break;
			
			case WORD_CALL:
				if (W->value.B) {
					argc = ki_compile_block_countstmt(*W->value.B);
					
					printf("FN %d ARGS\n", argc);
					
					P = W->value.B->child;
					for (int i=0; P; i++) {
						if (P->child) {
							size_t stmt_off = ki_compile_stmt(bin+offset, CT, *P);
							offset += stmt_off;
							printf("ARGV[%d]: %lu\n", i, stmt_off);
						}
						
						P = P->next;
					}
					
				} else {
					printf("FN NO ARGS\n");
					
					argc = 0;
				}
				
				// offset += ki_compile_writefmt(bin+offset, "bb", OP_PUSHB, argc);
				CNslot = ki_compile_cnode_str(CT, W->value.s);
				offset += ki_compile_writefmt(bin+offset, "bw", OP_CALL, CNslot);
				
				break;
			
			
			case WORD_KEYWORD:
				if (ki_compile_kwasm[W->value.k].data) {
					printf("KW %d\n", W->value.k);
					offset += ki_compile_writefmt(bin+offset, "B", ki_compile_kwasm[W->value.k].len,
					                                               ki_compile_kwasm[W->value.k].data);
					
				} else {
					printf("KW ???\n");
				}
				break;
			
			case WORD_LITERAL_INTEGER:
				if (W->value.i < 256) {
					offset += ki_compile_writefmt(bin+offset, "bb", OP_PUSHB, W->value.i);
				} else if (W->value.i < 65536) {
					offset += ki_compile_writefmt(bin+offset, "bw", OP_PUSHW, W->value.i);
				} else {
					// int constants are never merged
					// since the size to speed tradeoff is often not worth it
					// we save a few bytes of memory, but lose manu cycles in performance
					// looking up and creating the ctable value
					offset += ki_compile_writefmt(bin+offset, "bd", OP_PUSHD, W->value.i);
				}
				break;
			
			case WORD_LITERAL_FLOAT:
				offset += ki_compile_writefmt(bin+offset, "bf", OP_PUSHF, W->value.f);
				break;
				
			case WORD_LITERAL_STRING:
				CNslot = ki_compile_cnode_str(CT, W->value.s);
				offset += ki_compile_writefmt(bin+offset, "bw", OP_PUSHC, CNslot);
				break;
				
			case WORD_BLOCK:
				ki_compile_error("standalone blocks are not allowed");
				break;
				
			case WORD_IF:
				offset += ki_compile_block(bin+offset, CT, W->value.B[0]);
				
				offset_a = ki_compile_block(bin_A, CT, W->value.B[1]);
				
				if (W->value.Bc > 2) {
					offset_b = ki_compile_block(bin_B, CT, W->value.B[2]);
				} else {
					// write block A
					// preamble (BRANZ over block 1 if condition not met)
					if (offset_a > 127) {
						offset += ki_compile_writefmt(bin+offset, "bw", OP_JMPNZ, offset + 3 + offset_a);
					} else {
						offset += ki_compile_writefmt(bin+offset, "bo", OP_BRAZ, offset_a);
					}
					
					offset += ki_compile_writefmt(bin+offset, "B", offset_a, bin_A);
				}
				
				
				
				break;
				
			case WORD_WHILE:
				break;
				
			case WORD_FOR:
				break;
		}
		W = W->next;
	}
	
	printf("STMT %lu\n", offset);
	return offset;
}

size_t ki_compile_block(uint8_t *P, struct CompiledTable *CT, struct ParsedBlock B) {
	size_t offset = 0;
	struct ParsedStmt *S = B.child;
	
	while (S) {
		offset += ki_compile_stmt(P + offset, CT, *S);
		S = S->next;
	}
	
	printf("BLOCK %lu\n", offset);
	return offset;
}

void ki_compile(struct Ki *K, struct ParsedBlock B) {
	struct CompiledTable CT = {0};
	uint8_t *BC_buffer = malloc(4096);
	uint8_t *P = BC_buffer;
	
	const uint8_t bin[] = {
		OP_CALLF, 0x01, 0x00,
		OP_DUP,
		OP_CALLF, 0x02, 0x00,
		
		OP_DUP,
		OP_CALLF, 0x02, 0x00,
		OP_DUP,
		OP_BRANZ, -7,
		OP_RETS
	};
	
	K->main_fn    = BC_buffer;
	K->main_fn_sz = ki_compile_block(P, &CT, B);
	printf("PROGRAM %lu\n", K->main_fn_sz);
	
	/*
	
	memcpy(BC_buffer, bin, sizeof bin);
	
	ki_compile_pushct(&CT, (struct CompiledNode) { .data = (struct KiValue){.type = KI_STR, .value.s = "read"} });
	ki_compile_pushct(&CT, (struct CompiledNode) { .data = (struct KiValue){.type = KI_STR, .value.s = "echo"} });
	
	ki_compile_pushct(&CT, (struct CompiledNode) {
		.data = (struct KiValue){.type = KI_FUNCTION, .value.Bc = sizeof bin, .value.B = BC_buffer}
	});
	
	K->main_fn = 3;
	*/
	
	
	K->table = CT;
}

/*

C: ctable index, 16bit uint
P: 16-bit unsigned integer, program location
o: 8-bit signed integer (offset)

b: 8-bit unsigned integer
w: 16-bit unsigned integer
d: 32-bit unsigned integer
f: 32-bit float


*/
struct {char *name; char *args} ki_prg_opcodes[256] = {
	[OP_JUMP]  = {"JUMP",  "P"},
	[OP_JMPZ]  = {"JMPZ",  "P"},
	[OP_JMPNZ] = {"JMPNZ", "P"},
	[OP_BRA]   = {"BRA",   "o"},
	[OP_BRAZ]  = {"BRAZ",  "o"},
	[OP_BRANZ] = {"BRANZ", "o"},
	[OP_CALL]  = {"CALL",  "C"},
	[OP_CALLF] = {"CALLF", "C"},
	[OP_CMPEQ] = {"CMPEQ", ""},
	[OP_CMPNE] = {"CMPNE", ""},
	[OP_CMPLT] = {"CMPLT", ""},
	[OP_CMPGT] = {"CMPGT", ""},
	[OP_RETS]  = {"RETS",  ""},
	[OP_OR]    = {"OR",    ""},
	[OP_AND]   = {"AND",   ""},
	[OP_XOR]   = {"XOR",   ""},
	[OP_NOT]   = {"NOT",   ""},
	[OP_NEG]   = {"NEG",   ""},
	[OP_ABS]   = {"ABS",   ""},
	[OP_MIN]   = {"MIN",   ""},
	[OP_MAX]   = {"MAX",   ""},
	[OP_ADD]   = {"ADD",   ""},
	[OP_SUB]   = {"SUB",   ""},
	[OP_MULT]  = {"MULT",  ""},
	[OP_DIV]   = {"DIV",   ""},
	[OP_MOD]   = {"MOD",   ""},
	[OP_DEC]   = {"DEC",   ""},
	[OP_INC]   = {"INC",   ""},
	[OP_ENTERCLASS]  = {"ENTERCLASS",  ""},
	[OP_LEAVECLASS]  = {"LEAVECLASS",  ""},
	[OP_PUSHV]   = {"PUSHV",  "C"},
	[OP_PUSHC]   = {"PUSH",   "C"},
	[OP_PUSHB]   = {"PUSH",   "b"},
	[OP_PUSHW]   = {"PUSH",   "w"},
	[OP_PUSHD]   = {"PUSH",   "d"},
	[OP_PUSHF]   = {"PUSH",   "f"},
	[OP_DUP]     = {"DUP",     ""},
	[OP_SWAP]    = {"SWAP",    ""},
	[OP_POPV]    = {"POPV",    "C"},
	[OP_DROP]    = {"DROP",    ""},
	[OP_ROR]     = {"ROR",     ""},
	[OP_ROL]     = {"ROL",     ""},
	[OP_SSIZE]   = {"SSIZE",   ""},
	[OP_SCLEAR]  = {"SCLEAR",  ""},
};

struct CompiledNode *ki_get_cnode(struct CompiledTable CT, int N) {
	struct CompiledNode *P = CT.child;
	for (int i=1; i<N && P; i++) {
		P = P->next;
	}
	
	return P;
}

void ki_prg_function_dump(struct Ki K, uint8_t *B, size_t Bc, int nest_level) {
	uint16_t P = 0;
	uint16_t PreP;
	
	while (P < Bc) {
		char buffer[32] = {0};
		
		uint8_t opcode;
		int tmp;
		struct CompiledNode *CN;
		
		INDENT(nest_level);
		printf("%04x | ", P);
		
		PreP = P;
		
		opcode = B[P++];
		if (ki_prg_opcodes[opcode].name) {
			printf("%s ", ki_prg_opcodes[opcode].name);
			
			for (int i=0; ki_prg_opcodes[opcode].args[i]; i++) {
				if (i > 0) printf(", ");
				switch (ki_prg_opcodes[opcode].args[i]) {
					case 'o':
						tmp = (int8_t) B[P++];
						printf("%+d (0x%04x)", tmp, P + tmp);
						break;
					
					case 'C':
						tmp = ki_prg_read16(&B[P]);
						P += 2;
						
						printf("$%d: ", tmp);
						
						CN = ki_get_cnode(K.table, tmp);
						if (CN) {
							ki_prg_cnode_dump(K, *CN, nest_level + 1);
						} else {
							printf("INVALID");
						}
						break;
					
					case 'b':
						printf("0x%02x", B[P++]);
						break;
					case 'P':
					case 'w':
						printf("0x%04x", ki_prg_read16(&B[P]));
						P += 2;
						break;
					case 'd':
						printf("0x%08x", ki_prg_read32(&B[P]));
						P += 4;
						break;
					case 'f':
						printf("%f", ki_prg_readf32(&B[P]));
						P += 4;
						break;
					
					default:
						printf("?");
						break;
				}
			}
		} else {
			printf("????");
		}
		
		printf(" | ");
		
		for (int i=0; i<8; i++) {
			if (i < (P - PreP)) printf("%02x ", B[PreP + i]);
			else                printf("   ");
		}
		
		printf("\n");
	}
}

void ki_prg_cnode_dump(struct Ki K, struct CompiledNode C, int nest_level) {
	switch (C.data.type) {
		case KI_INT:
			printf("%d", C.data.value.i);
			break;
		
		case KI_STR:
			printf("'%s'", C.data.value.s);
			break;
		
		case KI_FLOAT:
			printf("%f", C.data.value.f);
			break;
		
		case KI_FUNCTION:
			printf("FUNCTION:\n");
			ki_prg_function_dump(K, C.data.value.B, C.data.value.Bc, nest_level + 1);
			break;
		
		default:
			printf("????\n");
			break;
	}
}

void ki_prg_dump(struct Ki K, int nest_level) {
	
	struct CompiledNode *P = K.table.child;
	
	for (int i=0; P; i++) {
		INDENT(nest_level);
		printf("%d = ", i + 1);
		ki_prg_cnode_dump(K, *P, nest_level + 1);
		printf("\n");
		
		P = P->next;
	}
	
	INDENT(nest_level);
	printf("MAIN (%lu):\n", K.main_fn_sz);
	ki_prg_function_dump(K, K.main_fn, K.main_fn_sz, nest_level + 1);
}