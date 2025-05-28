#include "ki.h"

#define MAX_OBJECT_SIZE 4096
#define MERGE_CONSTANTS ON_COUNT(5)

size_t ki_compile_stmt(uint8_t *dest, struct CompiledTable *CT, struct ParsedStmt S) {
	struct ParsedWord *W = S.child;
	
	uint8_t *P = dest;
	
	while (W) {
		W = W->next;
	}
	
	return P - dest;
}

size_t ki_compile_block(uint8_t *dest, struct CompiledTable *CT, struct ParsedBlock B) {
	struct ParsedStmt *S = B.child;
	
	uint8_t *P = dest;
	
	while (S) {
		P += ki_compile_stmt(P, CT, *S);
		
		S = S->next;
	}
	
	return P - dest;
}

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
	
	//P += ki_compile_block(P, &CT, B);
	memcpy(BC_buffer, bin, sizeof bin);
	
	ki_compile_pushct(&CT, (struct CompiledNode) { .data = (struct KiValue){.type = KI_STR, .value.s = "read"} });
	ki_compile_pushct(&CT, (struct CompiledNode) { .data = (struct KiValue){.type = KI_STR, .value.s = "echo"} });
	
	ki_compile_pushct(&CT, (struct CompiledNode) {
		.name = "__main__",
		.data = (struct KiValue){.type = KI_BLOB, .value.Bc = sizeof bin, .value.B = BC_buffer}
	});
	
	K->table = CT;
}

void ki_prg_cnode_dump(struct CompiledNode C, int nest_level) {
	switch (C.data.type) {
		case KI_INT:
			printf("%d\n", C.data.value.i);
			break;
	}
}

void ki_prg_dump(struct Ki K, int nest_level) {
	
	struct CompiledNode *P = K.table.child;
	
	for (int i=0; P; i++) {
		INDENT(nest_level);
		printf("%d = ", i + 1);
		ki_prg_cnode_dump(*P, nest_level + 1);
		
		P = P->next;
	}
	
}