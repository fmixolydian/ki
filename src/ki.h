#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

#define IS_BIG_ENDIAN    (htobe16(0x64AD) == 0x64AD)
#define IS_LITTLE_ENDIAN (htole16(0x64AD) == 0x64AD)

#define INDENT(n) for (int t=0; t<(n); t++) printf("\t")

#define ON_NEVER   -1
#define ON_ALWAYS   0
#define ON_COUNT(n) n

/* lexer structs */
typedef enum {
	NODE_NULL,
	
	NODE_KEYWORD,
	NODE_CALL,
	NODE_VARPUSH,
	NODE_VARPOP,
	
	NODE_LITERAL_INTEGER,
	NODE_LITERAL_FLOAT,
	NODE_LITERAL_STRING,
	
	NODE_BLOCK_START,
	NODE_BLOCK_END,
	
	NODE_STATEMENT_SEP,
} LexedNodeType;

typedef enum {
	// syntaxical: keywords that alter both parsed layout and assembled layout
	KW_IF,
	KW_THEN,
	KW_ELSE,
	
	KW_WHILE,
	KW_DO,
	
	KW_FOR,
	KW_TO,
	KW_STEP,
	
	KW_FOREACH,
	KW_IN,
	
	KW_SWITCH,
	KW_CASE,
	KW_DEFAULT,
	
	// programmatic: keywords that alter assembled layout
	KW_ADD,
	KW_SUB,
	KW_MUL,
	KW_DIV,
	KW_MOD,
	KW_NEG,
	KW_ABS,
	
	KW_MAX,
	KW_MIN,
	KW_INC,
	KW_DEC,
	
	KW_DUP,
	KW_DROP,
	KW_SWAP,
	KW_ROLL,
	
	KW_C_OR,
	KW_C_AND,
	KW_C_XOR,
	KW_C_NOT,
	KW_C_EQ,
	KW_C_NE,
	KW_C_LT,
	KW_C_GT,
	
	NO_KW
} Keyword;

struct LexedNode {
	LexedNodeType type;
	union {
		char    *s;
		uint32_t i;
		float    f;
	} val;
	
	struct LexedNode *next;
};

struct LexedBlock {
	struct LexedNode *tokens;
};

/* parser structs */
typedef enum {
	WORD_KEYWORD,
	WORD_VARPOP,
	WORD_VARPUSH,
	WORD_CALL,
	
	WORD_LITERAL_INTEGER,
	WORD_LITERAL_FLOAT,
	WORD_LITERAL_STRING,
	
	WORD_BLOCK,
	WORD_IF,
	WORD_WHILE,
	WORD_FOR
} ParsedWordType;

// WORD_BLOCK
// WORD_IF
// 	if (A) then (B)
// 	if (A) then (B) else (C)

struct ParsedStmt {
	struct ParsedStmt *next;
	void *child;
		// points to any struct derivative of ParsedWord
			// first elements:
				// ParsedWord *next;
				// ParsedWordType type;
};

struct ParsedBlock {
	struct ParsedStmt *child;
};

struct ParsedWord {
	struct ParsedWord *next;
	ParsedWordType type;
	
	struct {
		union {
			uint32_t i;
			float    f;
			char    *s;
			uint8_t  k;
		};
		
		size_t Bc;
		struct ParsedBlock *B;
	} value;
	
};


typedef enum {
	KI_INT,
	KI_STR,
	KI_FLOAT,
	KI_BLOB
} KiType;

struct KiValue {
	KiType type;
	
	union {
		uint32_t i;
		float    f;
		char    *s;
		struct {
			size_t Bc;
			uint8_t *B;
		}
	} value;
};

struct CompiledNode {
	char *name;
	struct KiValue data;
	
	struct CompiledNode *next;
};

// constant table
typedef enum {
	// control flow (2x)
		OP_JUMP  = 0x20,
		OP_JMPZ  = 0x22,
		OP_JMPNZ = 0x23,
		
		OP_BRA   = 0x24,
		OP_BRAZ  = 0x26,
		OP_BRANZ = 0x27,
		
		OP_CALL  = 0x28,
		OP_CALLF = 0x29,
		OP_RETS  = 0x2A,
	
	// math (4x)
		OP_OR    = 0x40,
		OP_AND   = 0x41,
		OP_XOR   = 0x42,
		OP_NOT   = 0x43,
		OP_NEG   = 0x44,
		OP_ABS   = 0x45,
		OP_MIN   = 0x46,
		OP_MAX   = 0x47,
		
		OP_ADD   = 0x48,
		OP_SUB   = 0x49,
		OP_MULT  = 0x4A,
		OP_DIV   = 0x4B,
		OP_MOD   = 0x4C,
		OP_DEC   = 0x4E,
		OP_INC   = 0x4F,
		
	// stack (3x)
		OP_PUSH0   = 0x30, // 0
		OP_PUSH1   = 0x31, // 1
		OP_PUSHM1  = 0x32, // -1
		OP_PUSHV   = 0x33, // ctable var
		OP_PUSHB   = 0x34, // byte (8bit)
		OP_PUSHW   = 0x35, // word (16bit)
		OP_PUSHD   = 0x36, // dword (32bit)
		OP_PUSHF   = 0x37, // float (32bit)
		
		OP_DUP     = 0x38,
		OP_SWAP    = 0x39,
		OP_POPV    = 0x3A,
		OP_DROP    = 0x3B,
		OP_ROR     = 0x3C,
		OP_ROL     = 0x3D,
		
		OP_SSIZE   = 0x3E,
		OP_SCLEAR  = 0x3F,
	
	// comparison (7x)
		OP_CMPEQ   = 0x78,
		OP_CMPNE   = 0x79,
		OP_CMPLT   = 0x7A,
		OP_CMPGT   = 0x7B,
		OP_CMPEQV  = 0x7C,
		OP_CMPNEV  = 0x7D,
		OP_CMPLTV  = 0x7E,
		OP_CMPGTV  = 0x7F,
} KiOp;

struct CompiledTable {
	struct CompiledNode *child;
	size_t len;
};

struct KiStackNode {
	struct KiValue data;
	
	struct KiStackNode *prev;
	struct KiStackNode *next;
};

struct KiStack {
	struct KiStackNode *head;
	size_t len;
};

struct Ki {
	struct CompiledTable table;
	struct KiStack stack;
};

// general.c
int                  ki_strtok(char *dest, size_t len, char *src, char *delim);
void                 ki_strrev(char *dest, char *src,  size_t n);
void                 ki_memdup(            void *src,  size_t n);

// main.c
int                  main();

// parse.c
void                 ki_parse_error(char *s);
struct ParsedStmt    ki_parse_block_htail(      struct ParsedBlock *B);
void                 ki_parse_block_pushstmt(   struct ParsedBlock *B, struct ParsedStmt S);

void                 ki_parse_stmt_pushword(    struct ParsedStmt *S, struct ParsedWord W);
struct ParsedWord    ki_parse_stmt_htail(       struct ParsedStmt *S);
struct ParsedWord    ki_parse_word_next_guarded(struct ParsedWord *W, ParsedWordType t, Keyword k);
struct ParsedWord    ki_parse_clone_blocksword( struct ParsedWord *W);

struct ParsedBlock   ki_parse_analyze_pass1(    struct LexedBlock L, int nest_level);
struct ParsedBlock   ki_parse_analyze_pass2(    struct ParsedBlock SrcB);
struct ParsedBlock   ki_parse_analyze(          struct LexedBlock L);

void                 ki_parse_dump_stmt(        struct ParsedStmt S, int indent);
void                 ki_parse_dump(             struct ParsedBlock B, int indent);

// lex.c
struct LexedBlock    ki_lex_newblock();
struct LexedBlock    ki_lex_cutblock(struct LexedBlock src, int start, int end);
struct LexedNode     ki_lex_tail(struct LexedBlock *L);
void                 ki_lex_push(struct LexedBlock *L, struct LexedNode node);
void                 ki_lex_error(char *L, char *P, char *s);
struct LexedBlock    ki_lex_analyze(char *program);
void                 ki_lex_dump(struct LexedBlock L, int indent);

// compile.c
struct CompiledNode  ki_compile_ctable_tail(struct CompiledTable *CT);
void                 ki_compile_pushct(struct CompiledTable *CT, struct CompiledNode C);
void                 ki_compile(struct Ki *K, struct ParsedBlock B);
void                 ki_prg_cnode_dump(struct CompiledNode C, int nest_level);


extern const char *ki_keyword_names[];
extern const char *ki_lex_nodenames[];