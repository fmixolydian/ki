#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

#define IS_BIG_ENDIAN    (htobe16(0x64AD) == 0x64AD)
#define IS_LITTLE_ENDIAN (htole16(0x64AD) == 0x64AD)

#define INDENT(n) for (int t=0; t<(n); t++) printf("\t")

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
	int type; // = 0
	struct ParsedStmt *next;
	void *child;
		// points to any struct derivative of ParsedWord
			// first elements:
				// ParsedWord *next;
				// ParsedWordType type;
};

struct ParsedLexedStmt {
	int type; // = 1
	struct ParsedStmt *next;
	struct LexedBlock B;
};

struct ParsedBlock {
	struct ParsedStmt *child;
};

struct ParsedBlockStmt {
	int type; // = 1
	struct ParsedStmt *next;
	struct ParsedBlock B;
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
	} value;
};

struct KiPrgT {
	
};

// ki program table
struct Prg {
	struct PrgNode *child;
};

struct Ki {
	struct Prg kpt;
};

struct ParsedStmt *ki_parse_block_htail(struct ParsedBlock *B);
void ki_parse_block_pushstmt(struct ParsedBlock *B, struct ParsedStmt S);
void ki_parse_block_pushlexstmt(struct ParsedBlock *B, struct ParsedLexedStmt LS);
void ki_parse_block_pushblkstmt(struct ParsedBlock *B, struct ParsedBlockStmt BS);
void ki_parse_error(char *s);
struct ParsedWord *ki_parse_stmt_htail(struct ParsedStmt *S);
void ki_parse_stmt_pushword(struct ParsedStmt *S, struct ParsedWord W);
void ki_parse_dump_stmt(struct ParsedStmt S, int indent);
struct ParsedBlock ki_parse_analyze_pass1(struct LexedBlock L, int nest_level);
struct ParsedWord *ki_parse_word_next_guarded(struct ParsedWord *W, ParsedWordType t, Keyword k);
struct ParsedBlock ki_parse_analyze_pass2(struct ParsedBlock SrcB);
struct ParsedBlock ki_parse_analyze(struct LexedBlock L);
void ki_parse_dump(struct ParsedBlock B, int indent);
struct LexedBlock ki_lex_newblock();
struct LexedBlock ki_lex_cutblock(struct LexedBlock src, int start, int end);
struct LexedNode *ki_lex_tail(struct LexedBlock *prg);
void ki_lex_push(struct LexedBlock *prg, struct LexedNode node);
int ki_strtok(char *dest, size_t len, char *src, char *delim);
void ki_strrev(char *dest, char *src, size_t n);
void ki_lex_error(char *prg, char *P, char *s);
struct LexedBlock ki_lex_analyze(char *program);
void ki_lex_dump(struct LexedBlock prg, int indent);


extern const char *ki_keyword_names[];
extern const char *ki_lex_nodenames[];