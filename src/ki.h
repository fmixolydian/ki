#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

#define IS_BIG_ENDIAN    (htobe16(0x64AD) == 0x64AD)
#define IS_LITTLE_ENDIAN (htole16(0x64AD) == 0x64AD)

/* lexer structs */
typedef enum {
	NODE_NULL,
	
	NODE_IDENTIFIER, // for function calls (operators, noargs or withargs), vars (pop)
	
	NODE_VARPUSH,
	
	NODE_LITERAL_INTEGER,
	NODE_LITERAL_FLOAT,
	NODE_LITERAL_STRING,
	
	NODE_BLOCK_START,
	NODE_BLOCK_END,
	
	NODE_STATEMENT_SEP,
} LexedNodeType;

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
	WORD_IDENTIFIER, // function calls, var pops (pass 1)
	WORD_VARPOP, // only pass 2
	WORD_VARPUSH,
	WORD_CALL, // only pass 2
	
	WORD_LITERAL_INTEGER,
	WORD_LITERAL_FLOAT,
	WORD_LITERAL_STRING,
	
	WORD_BLOCK, // empty block
	WORD_IF,
	WORD_WHILE,
	WORD_FOR,
} ParsedWordType;

// WORD_IDENTIFIER
// WORD_VARPOP
// WORD_VARPUSH
// WORD_CALL
// WORD_LITERAL_STRING
struct ParsedWordStr {
	void *next;
	ParsedWordType type;
	
	char *value;
};

// WORD_LITERAL_INTEGER
struct ParsedWordInt {
	void *next;
	ParsedWordType type;
	
	int value;
};

// WORD_LITERAL_FLOAT
struct ParsedWordFloat {
	void *next;
	ParsedWordType type;
	
	float value;
};

// WORD_BLOCK
// WORD_IF
// 	if (A) then (B)
// 	if (A) then (B)                                     else (C)
// 	if (A) then (B) elif (C) then (D)                   
// 	if (A) then (B) elif (C) then (D)                   else (E)
// 	if (A) then (B) elif (C) then (D) elif (E) then (F) 
// 	if (A) then (B) elif (C) then (D) elif (E) then (F) else (G)
// 	...

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
	void *next;
	ParsedWordType type;
};

struct ParsedWordBlocks {
	void *next;
	ParsedWordType type;
	
	size_t no_blocks;
	struct ParsedBlock B[];
};

struct ParsedWordBlockSingle {
	void *next;
	ParsedWordType type;
	
	struct ParsedBlock B;
};

struct ParsedBlock ki_parse_analyze(struct LexedBlock L);

void ki_lex_dump(struct LexedBlock prg);
struct LexedBlock ki_lex_analyze(char *program);
struct LexedBlock ki_lex_cutblock(struct LexedBlock src, int start, int end);
struct LexedBlock ki_lex_newblock();
struct LexedNode *ki_lex_tail(struct LexedBlock *prg);
void ki_lex_push(struct LexedBlock *prg, struct LexedNode node);

int  ki_strtok(char *dest, size_t len, char *src, char *delim);
void ki_strrev(char *dest, char *src, size_t n);